//#define BUILDING_NODE_EXTENSION
#include <node.h>
#include <nan.h>
extern "C" {
#include "mm.h"
#include "log.h"
}
#include "ShmdbObject.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace v8;

ShmdbObject::ShmdbObject() {
	hasInit = false;
	_isParent = false;
};
ShmdbObject::~ShmdbObject() {
	if (_isParent && hasInit) {
		printf("destroy the shmdb.\n");
		shmdb_destroy(&_handle);
	}
};

void ShmdbObject::Init(Handle<Object> module) {
	// Prepare constructor template
	Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(New);//使用ShmdbNanNew<Object>函数作为构造函数
	tpl->SetClassName(NanNew<String>("ShmdbObject"));//js中的类名为ShmdbObject
	tpl->InstanceTemplate()->SetInternalFieldCount(5);//指定js类的字段个数
	// Prototype
	Local< ObjectTemplate > prototypeTpl = tpl->PrototypeTemplate();

	prototypeTpl->Set(NanNew<String>("put"),//js类的成员函数名为put
		NanNew<FunctionTemplate>(put)->GetFunction());  //js类的成员处理函数
	prototypeTpl->Set(NanNew<String>("get"),
		NanNew<FunctionTemplate>(get)->GetFunction());		
	prototypeTpl->Set(NanNew<String>("remove"),
		NanNew<FunctionTemplate>(remove)->GetFunction());	
	prototypeTpl->Set(NanNew<String>("destroy"),
		NanNew<FunctionTemplate>(destroy)->GetFunction());	

	//Persistent<Function> constructor = Persistent<Function>::New/*New等价于js中的new*/(tpl->GetFunction());//new一个js实例
	Persistent<Function> constructor;
	NanAssignPersistent(constructor,tpl->GetFunction());
	module->Set(NanNew<String>("exports"), constructor);
}


NAN_METHOD(ShmdbObject::New) {
	NanScope();

	ShmdbObject* obj = new ShmdbObject();
	obj->hasInit = false;
	Local<Value> param = args[0];
	bool needCreate = true;
	int rv = -1;
	unsigned int _shmid = 0;
	unsigned int _semid = 0;
	
	if (param->IsUndefined()) {
		obj->_length = 128;
	} else if (param->IsNumber()) {
		obj->_length = param->NumberValue();/*将js中的参数转化为c++类型*/
	} else if (param->IsObject()) {
		needCreate = false;
		Local<Object> option = param->ToObject();
		_shmid = option->Get(NanNew<String>("shmid"))->NumberValue();
		_semid = option->Get(NanNew<String>("semid"))->NumberValue();
	}	
	
	if (needCreate) {
		STShmdbOption shmdbOption;
		memset(&shmdbOption,0,sizeof(shmdbOption));
		shmdbOption.logLevel = LEVEL_WARN;
		obj->_isParent = true;
		if (args.Length() == 2) {
			Local<Value> optionParam = args[1];
			if (optionParam->IsObject() && !optionParam->IsNull()) {
				Local<Value> logLevel = optionParam->ToObject()->Get(NanNew<String>("logLevel"));
				if (logLevel->IsNumber()) {
					shmdbOption.logLevel = logLevel->NumberValue();
				}				
			}
		}
		
		rv = shmdb_initParent(&obj->_handle,obj->_length,&shmdbOption);
		if (rv == 0) {	
			obj->hasInit = true;
			_shmid = (unsigned int)obj->_handle.shmid;
			_semid = (unsigned int)obj->_handle.semid;
			printf("[New]_length int:%d,shmid int:%d,semid int:%d\n",obj->_length,_shmid,_semid);
			//printf("shmid *int:%d,semid *int:%d\n",*(obj->shmid),*(obj->semid));
		} else {
			obj->hasInit = false;
		}
	} else {
		obj->_handle.shmid = (HANDLE)_shmid;
		obj->_handle.semid = (HANDLE)_semid;
		rv = shmdb_initChild(&obj->_handle);
		if (rv == 0) {
			obj->hasInit = true;
		}
	}
	

	obj->Wrap(args.This()/*将c++对象转化为js对象*/);
	args.This()->Set(NanNew<String>("rv"),Number::New(rv));
	if (rv == 0) {
		args.This()->Set(NanNew<String>("shmid"),Number::New(_shmid));
		args.This()->Set(NanNew<String>("semid"),Number::New(_semid));
	}
	

	NanReturnThis();//返回这个js对象
}


NAN_METHOD(ShmdbObject::put) {
	NanScope();
	Local<Object> result = NanNew<Object>();
	ShmdbObject* obj = ObjectWrap::Unwrap<ShmdbObject>(args.This());
	if (obj->hasInit) {
		if (args.Length() == 2) {

			v8::String::Utf8Value keyStr(args[0]->ToString());
			int keyLen = keyStr.length();
			char *keyBuffer = *keyStr;
			
			v8::String::Utf8Value  valueStr(args[1]->ToString());
			int valueLen = valueStr.length();
			char *valueBuffer = *valueStr;

			//printf("before put\n");
			int rv = shmdb_put(&obj->_handle,keyBuffer,(unsigned short)keyLen,
				valueBuffer,(unsigned short)valueLen);
			//printf("after put result:%x\n",rv);
				
			result->Set(NanNew<String>("code"), Number::New(rv));

		} else {
			result->Set(NanNew<String>("code"), Number::New(ERROR_PRARAM_ERROR));
		}		
	} else {
		result->Set(NanNew<String>("code"), Number::New(ERROR_NOT_INIT));
		printf("shmdb has not inited\n");
	}
	NanReturnValue(result);
}

NAN_METHOD(ShmdbObject::get) {
	NanScope();
	Local<Object> result = NanNew<Object>();
	ShmdbObject* obj = ObjectWrap::Unwrap<ShmdbObject>(args.This());
	if (!obj->hasInit) {
		result->Set(NanNew<String>("code"), Number::New(ERROR_NOT_INIT));
		printf("shmdb has not inited\n");
		NanReturnValue(result);
	}
	char *valueBuffer = NULL;
	unsigned short valueLen = 0;
	v8::String::Utf8Value keyStr(args[0]->ToString());
	int keyLen = keyStr.length();
	char *keyBuffer = *keyStr;
	//printf("before get\n");
	int rv = shmdb_get(&obj->_handle,keyBuffer,(unsigned short)keyLen,
				&valueBuffer,&valueLen);
	//printf("after get %x\n",rv);
	if (rv == 0) {
		Local<String> str = String::New(valueBuffer,valueLen);
		result->Set(NanNew<String>("data"),str);
		free(valueBuffer);
	}
	result->Set(NanNew<String>("code"), Number::New(rv));
	NanReturnValue(result);
}

NAN_METHOD(ShmdbObject::remove) {
	NanScope();
	Local<Object> result = NanNew<Object>();
	ShmdbObject* obj = ObjectWrap::Unwrap<ShmdbObject>(args.This());
	if (!obj->hasInit) {
		result->Set(NanNew<String>("code"), Number::New(ERROR_NOT_INIT));
		printf("shmdb has not inited\n");
		NanReturnValue(result);
	}
	
	char *valueBuffer = NULL;
	unsigned short valueLen = 0;
	v8::String::Utf8Value keyStr(args[0]->ToString());
	int keyLen = keyStr.length();
	char *keyBuffer = *keyStr;
	
	int rv = shmdb_delete(&obj->_handle,keyBuffer,(unsigned short)keyLen,
				&valueBuffer,&valueLen);
	
	if (rv == 0) {
		Local<String> str = String::New(valueBuffer,valueLen);
		result->Set(NanNew<String>("data"),str);
		free(valueBuffer);
	}
	result->Set(NanNew<String>("code"), Number::New(rv));
	NanReturnValue(result);	
}

NAN_METHOD(ShmdbObject::destroy) {
	NanScope();
	Local<Object> result = NanNew<Object>();
	ShmdbObject* obj = ObjectWrap::Unwrap<ShmdbObject>(args.This());
	if (!obj->hasInit) {
		result->Set(NanNew<String>("code"), Number::New(ERROR_NOT_INIT));
		printf("shmdb has not inited\n");
		NanReturnValue(result);
	}
	int rv = shmdb_destroy(&obj->_handle);
	result->Set(NanNew<String>("code"), Number::New(rv));
	NanReturnValue(result);	
}



