# node-shmdb

## 背景

在进程之间共享数据，有下面几种方式。首先是使用变量。
	
	
	var cluster = require('cluster'); 
	var a = 1;
	if (cluster.isMaster) {	
		cluster.fork();  
	} else if (cluster.isWorker) {
		console.log('a is ',a);
	}
在子进程中变量`a`的值是1，但是在子进程或者主进程中给`a`重新赋值时，另一个进程将得不到新赋的值。也就是说这种情况适合变量只读的情况。

其次使用事件通知。

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
在node中可以使用函数send来通知其他进程有值要更改。这种情况适合一个进程读，一个进程写的场景。众所周知，node没有锁的设计，那么怎么防止读到脏数据呢？这就是`node-shmdb`要解决的问题。

## 原理
`node-shmdb`实现自 一个基于共享内存的项目[shmdb](https://github.com/yunnysunny/shmdb "shmdb project")。`shmdb`是一个key-value类型的数据库，与memcache或者redis不同，它不通过网络建立连接，而是嵌入式的。它使用操作系统的共享内存来在不同进程间共享数据。

## 安装
`npm install shmdb`

## 示例

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

**描述**  
在父进程或者子进程中初始化`shmdb`.返回类 `Shmdb`的实例.

**参数**

- {Number|Object} `param`  在父进程中使用Number类型，他将在操作系统中创建共享内存，并且返回两个变量，`shmid`和`semid`，这两个变量将会在子进程中使用。在子进程中使用`Object`类型，他的格式应该为`{shmid:Number,semid:Number}`,`shmid`和`semid`可以从父进程中获取。

**返回**

- {Shmdb} 它的属性如下:

		{rv:Number,shmid:Number,semid:Number}
	
	如果成功，`rv`值为0，并且`shmid`和`semid`将会被分配值。否则，`rv`值将非零。

### Shmdb.prototype.put(key,value)

**描述**

将键值对写入`shmdb`.

**参数**

- key {String} 键
- value {String} 值

**返回**

- {Object} {code:Number} 成功时`code`为0.

### Shmdb.prototype.get(key)

**描述**

根据键获取值.

**参数**

- key {String} 键

**return**

- {Object} {code:Number,value:String}  成功时`code`为0.

### Shmdb.prototype.remove(key)

**描述**

通过键来移除一对键值对.

**参数**

- key {String} 键

**返回**

{Object} {code:Number,value:String}  成功时`code`为0.

### Shmdb.prototype.destroy()

**描述**

销毁通过调用函数`shmdb`分配的共享内存。当进程退出时，这个函数就会自动调用。

**参数**

- 空

**返回**

- {Object} {code:Number} 成功时`code`为0.

## 贡献者
[yunnysunny](https://github.com/yunnysunny) (维护者)

## license
[Apache License, Version 2.0](http://www.apache.org/licenses/LICENSE-2.0.html)
