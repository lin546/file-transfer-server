#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
# define LENGTH 10 
# define PORT 7775

int clifd = 0;
char cmd[20] = {};

void* start_run(void *arg);
void upload(void);
void download(void);
void filelist(void);

int main()
{
	int sockfd;
	pthread_t pid;
	unsigned int addrlen;
	struct sockaddr_in servaddr;
	struct sockaddr_in cliaddr;
	
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd == -1){
		perror("套接字创建失败!\n");
		exit(1);
	}

	// 设置SO_REUSEADDR选项，允许端口重用
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }
	
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	bzero(&(servaddr.sin_zero),8);

	if(bind(sockfd,(struct sockaddr*)&servaddr,sizeof(struct sockaddr)) == -1){
		perror("套接字绑定失败!\n");
		exit(1);
	}

	if(listen(sockfd,LENGTH) == -1){
		perror("设置监听模式错误！\n");
		exit(1);
	}

	printf("等待客户端连接...\n");
	while(1)
	{
		addrlen = sizeof(struct sockaddr_in);
		clifd = accept(sockfd,(struct sockaddr*)&cliaddr,&addrlen);
		if(clifd == -1){
			perror("接受连接请求错误！\n");
			continue;
		}
		printf("客户端IP：%s\n",inet_ntoa(cliaddr.sin_addr));
		pthread_create(&pid,NULL,start_run,&clifd);
	}
}

// 开始运行
void* start_run(void *arg){
	while(1){	
		int c_size = read(clifd,cmd,sizeof(cmd));
		if(strcmp("upload",cmd)==0)
		{
			printf("收到客户端的上传指令!\n");
			upload();
			memset(&cmd,0,20);
		}
		else if(strcmp("download",cmd)==0){
			printf("收到客户端的下载指令!\n");
			download();
			memset(&cmd,0,20);
		}
		else if(strcmp("list",cmd)==0){
			printf("收到客户端的目录指令!\n");
			filelist();
			printf("文件列表传输完毕\n");
			memset(&cmd,0,20);
		}
	}
}

void upload(void)
{
	char filename[20] = {};
	int size = read(clifd,filename,sizeof(filename));

	char path[50]={};
	strcat(path,"./files/");
	strcat(path,filename);
	
	printf("客户端请求上传文件:%s\n",filename);
	
	int fd = open(path,O_CREAT|O_RDWR,00777);
	int r_size = 0;
	char buf[4096] = {};
	do{
		r_size = recv(clifd,buf,sizeof(buf),0);
		printf("接受文件大小为:%d字节\n",r_size);
		write(fd,buf,r_size);
	}while(r_size==4096);

	close(fd);			
	return;	
}

void download(void){
	DIR *dir = opendir("./files");
	struct dirent *dirent;
	struct stat sta;
	char list[1024] = {};
	char filename[50] = {};
	char path[50]={};

	while((dirent = readdir(dir)) != NULL){
		strcat(list,dirent->d_name);
		strcat(list,",");
	}
	
	read(clifd,filename,sizeof(filename));
	printf("客户端请求下载:%s\n",filename);
	if(strstr(list,filename) == NULL){
		char result[50] = "该文件不存在！";
		printf("%s\n",result);
	}
	else{
		strcat(path,"./files/");
		strcat(path,filename);
		stat(path,&sta);   //获得文件的属性
		long filesize = sta.st_size;
		char sfilesize[10] = {};
		sprintf(sfilesize,"%ld",filesize);
		printf("文件存在,文件大小为:%ld byte!\n",filesize);
		
		int fd = open(path, O_RDONLY);
		int r_size = 0;
		int w_size = 0;
		do{
			char buf[2048] = {};
			r_size = read(fd,buf,sizeof(buf));	
			w_size = write(clifd,buf,r_size);
			usleep(500);
		}while(r_size == 2048);
		printf("发送完毕\n");
		close(fd);
	}
	return;
}

void filelist(void)
{
	DIR *dir = opendir("./files");
	char list[1024] = {};
	struct dirent *dirent;
  	int first = 1;
	while((dirent = readdir(dir)) != NULL){
		if(strcmp(dirent->d_name, ".") == 0 || strcmp(dirent->d_name, "..") == 0)
			continue;
		if (!first) {
			strcat(list,"|");
		}
		strcat(list,dirent->d_name);
		first = 0;
	}
	
	
	write(clifd,list,strlen(list));
	closedir(dir);
	return;
}
