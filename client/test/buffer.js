var fs = require("fs");

fs.open("../files/hello.txt","w+",function (err,fd) {
   if (err) {
       return;
   } 
   console.log("file open success");
})
