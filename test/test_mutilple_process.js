var shmdb = require('../index');
var cluster = require('cluster');
var obj = null;

if (cluster.isMaster) {
	console.info('=====================I am master=====================');
	obj = shmdb(10);
	if (obj.rv == 0) {
		console.log(obj);
		obj.put('some','value1');
		
		cluster.fork({shmid:obj.shmid,semid:obj.semid});  
		setTimeout(function() {
			console.log('get new value changed in child process.',obj.get('some'));
			console.log('put value to xx in parent process.',obj.put('xx','parent'));
		},2000);
	} else {
		console.error('get shared memery error:0x[%x]',obj.rv);
	}
	
} else if (cluster.isWorker) {
	console.info('=====================I am child=====================');
	var shmid = parseInt(process.env.shmid);
	var semid = parseInt(process.env.semid);
	if (shmid && semid) {
		obj = shmdb({shmid:shmid,semid:semid});
		console.log('get some in child:',obj.get('some'));
		console.log('put result in child',obj.put('some','other'));
		setTimeout(function() {
			console.log('get xx form parent',obj.get('xx'));
		},4000); 
	}
}

process.on('uncaughtException', function(err) {
    console.log(err);    
});