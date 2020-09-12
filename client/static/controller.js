var express = require('express');
var ejs = require("ejs");
var net = require('net');
var fs = require("fs");
var app = express();
app.set('view engine', 'ejs');
var mysql = require('mysql');

const socket = new net.Socket();
socket.setEncoding = 'UTF-8';
const port = 7775;
const hostname = '127.0.0.1';

socket.connect(port, hostname, function () {
   console.log("connect server success!");
});

var file_list = "";
var sfilename = "";

socket.on('data', function (msg) {
    var data = msg.toLocaleString();
    if(data.indexOf("..")!=-1){  //list files
        file_list = {
            files: data
        }
        console.log(file_list);
    }else{       //download file
        console.log(data);
        var fd = fs.openSync(sfilename,"w+");
        fs.writeFile(fd,data,function(err) {
            if (err) {
                return console.error(err);
            }
        });
    }
});

var connection = mysql.createConnection({
    host: '127.0.0.1',
    user: 'root',
    password: 'admin',
    database: 'mydata',
    port: '3306'
});

connection.connect();

app.get('/register', function (req, res) {
    var name = req.query.name;
    var pwd = req.query.pwd;
    var user = { uname: name, pwd: pwd };
    connection.query('insert into user set ?', user, function (err, rs) {
        if (err) throw err;
        res.sendfile(__dirname + "/" + "login.html");
    })
})

app.get('/login', function (req, res) {
    var name = req.query.name;
    var pwd = req.query.pwd;

    var selectSQL = "select * from user where uname = '" + name + "' and pwd = '" + pwd + "'";
    console.log(selectSQL);
    connection.query(selectSQL, function (err, rs) {
        if (err) {
            console.log('[SELECT ERROR] - ', err.message);
            return;
        }
        console.log(rs);
        if (rs == '') {
            console.log("username or password error!");
            res.sendfile(__dirname + "/" + "login.html");
        } else {
            res.redirect("/list");
        }
    })
})


app.get('/list', function (req, res) {

    socket.write("list");
    setTimeout(() => {
        res.render("index", file_list); 
    }, 1000);
    
})

app.get('/download', function (req, res) {

    socket.write("download");
               
    var filename = req.query.filename;
    sfilename = "../files/"+filename;
    socket.write(filename);

})


app.get('/upload', function (req, res) {

    socket.write("upload");
               
    var file = req.query.file;
    var s = file.split("\\");
    var filename = s[s.length-1];

    socket.write(filename);

    var buf = new Buffer.alloc(4096);

    var fd = fs.openSync(file, 'r+');
    
    var sendSize = 0;
    var packSize = 4096;

    let fileInfo = fs.statSync(file);
    let fileSize = fileInfo.size;
    
    setTimeout(function () {
        while (sendSize < fileSize) {
            let size = fs.readSync(fd, buf, 0, buf.length, sendSize);
            var data = buf.toString(undefined,0,size);
            console.log(data+"\n");
            console.log(data.length+"\n");

            socket.write(data);
            
            sendSize += packSize;
        }    
    },1000);
    
})

var server = app.listen(8080, function () {
    console.log("server start");
})


