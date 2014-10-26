var addon = require('./build/Release/shmdb');

var obj = new addon(10);
console.dir(obj);

obj.showInfo();
//console.log(obj.put);
console.log('=======================');
var result = obj.put('key','value');
console.log('result',result);
console.log('value:',obj.get('key')); 
console.log('============================');
result = obj.remove('key');
console.log('remove',result);
console.log('============================');
console.log('value:',obj.get('key'));
console.log('============================');

process.on('uncaughtException', function(err) {
    console.log(err);
    
});