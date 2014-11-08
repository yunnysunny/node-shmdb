var shmdb = require('./index');
var cluster = require('cluster');

var obj = shmdb(10);

/* obj.showInfo();
console.log('=======================');
var result = obj.put('key','xxx中文abc');
console.log('result',result);
console.log(obj.get('key')); 
console.log('============================');
result = obj.remove('key');
console.log('remove',result);
console.log('============================');
console.log('value:',obj.get('key'));
console.log('============================'); */

obj.put('some','value1');
if (cluster.isMaster) {
	console.log('I am master');
	cluster.fork();  
	setTimeout(function() {
		console.log(obj.get('some'));
		obj.put('xx','parent');
	},2000);
} else if (cluster.isWorker) {
	obj.initChild();
	console.log('get some from child:',obj.get('some'));
	var r = obj.put('some','other');
	console.log(r);
	console.log('I am worker #' + cluster.worker.id);
	setTimeout(function() {
		console.log('get xx form parent',obj.get('xx'));
	},4000);
}

process.on('uncaughtException', function(err) {
    console.log(err);    
});