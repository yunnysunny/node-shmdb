# node-shmdb

## why
Shareing the data between parent process and child process,you can servel ways.
First,use variable.

	var cluster = require('cluster'); 
	var a = 1;
	if (cluster.isMaster) {	
		cluster.fork();  
	} else if (cluster.isWorker) {
		console.log('a is ',a);
	}
Varivale `a` will get the value `1` in child process.But when you assign the value of `a` in one of process, the other process can not get the new value.In other words, it suit for readonly varivale.

Second,using event message to notify the other process.

	var cluster = require('cluster'); 

	if (cluster.isMaster) {	
		var worker = cluster.fork();  
		worker.send('a message form parent');
		worker.on('message', function(msg) {//message send by child
			console.log('get a message,its value:',msg);
		});
	} else if (cluster.isWorker) {
		cluster.worker.send(1);
		process.on('message',function(msg) {
			console.log(msg);
		});
	}  
In node,you can use the method of send to notify other process some value has changed.It suit for one process write and the other read.
But while both of two process read and read the same data,how can we deal with it? As we all known,there is no mechanism of lock in node,how to pervent from reading dirty data? `node-shmdb` is just aim at resolving the issue.

## theory
`node-shmdb` is an implementation of [shmdb](https://github.com/yunnysunny/shmdb "shmdb project"), which is based on os's shared memory.`shmdb` is a key-value type of database, unlike memcache or redis, it is not a database that connected via network,but it is embeded.For sharing data between processes, it use operation system's shared memeory.

## install

`npm install shmdb`

## example

	var shmdb = require('shmdb');
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

## API

### shmdb(param)

**description**  
Initializing shmdb in parent process or child process.It return an instance of Class `Shmdb`.

**parameter**

- {Number|Object} `param`  Using `Number` in parent process,it will create the shared memery in operation system,and return the two variable，`shmid` and `semid`,which will be used in child process.Using `Object` in child process,and its format should be `{shmid:Number,semid:Number}`,the `shmid` and `semid` can be 
obtained from parent process.

**return**

- {Shmdb} its prototies are as follows:

		{rv:Number,shmid:Number,semid:Number}
	
	When success,the `rv` will be zero,and `shmid` and `semid` will be assigned.On the other hand,the `rv` will be none-zero.

### Shmdb.prototype.put(key,value)

**description**

Put a pair of key/value into shmdb.

**parameter**

- key {String} the key
- value {String} the value

**return**

- {Object} {code:Number} when success the `code` will be zero.

### Shmdb.prototype.get(key)

**decription**

Get value by key.

**parameter**

- key {String} the key

**return**

- {Object} {code:Number,value:String}  when success the `code` will be zero.

### Shmdb.prototype.remove(key)

**description**

Remove an pair of key/value by key.

**parameter**

- key {String} the key

**return**

{Object} {code:Number,value:String}  when success the `code` will be zero.

### Shmdb.prototype.destroy()

**description**

Destory the shared memery allocated when you call the function `shmdb`.When process exit,this function will be called auto.

**parameter**

- void

**return**

- {Object} {code:Number} when success the `code` will be zero.

## contributors
[yunnysunny](https://github.com/yunnysunny) (maintainer)

## license
[Apache License, Version 2.0](http://www.apache.org/licenses/LICENSE-2.0.html)
