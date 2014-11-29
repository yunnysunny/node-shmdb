var shmdb = require('../index');
var cluster = require('cluster');
var obj = shmdb(10);
if (obj.rv == 0) {
	console.log('=======================');
	console.log('put result',obj.put('key','xxx中文abc'));
	console.log('get',obj.get('key')); 
	console.log('remove',obj.remove('key'));
	console.log('get:',obj.get('key'));
	obj = null;
	var i = 1;
	setInterval(function(){
		console.log(i++);
	},1000);
} else {
	console.error('create shmdb error:['+obj.rv+']');
}
