var addon = require('./build/Release/shmdb');

var obj = new addon(10);
console.dir(obj);
obj.showInfo();