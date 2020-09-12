const net = require('net');
const socket = new net.Socket();
const port = 7775;
const hostname = '127.0.0.1';
socket.setEncoding = 'UTF-8';

socket.connect(port, hostname, function () {
    socket.write('list');
});

socket.on('data', function (msg) {
    console.log(msg.toString());
});

socket.on('error', function (error) {
    console.log('error' + error);
});
