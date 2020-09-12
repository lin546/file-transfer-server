# file-transfer

基于Socket的文件传输系统（Using Linux C + NodeJS + Electron + Express）。

## 运行

Server端：
```
make
./server
```

Client端：

```
cd static
node controller.js
cd ../
npm start
```

## Note

关闭服务时，请先关闭server端，再关闭Client端。否则可能会提示IP地址被占用。

## About

该项目为学习Socket的练手项目，因为之前没有接触过网络编程和NodeJS，故必会有设计或实现不合理的地方。仅供自己学习记录。