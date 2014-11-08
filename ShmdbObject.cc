#define BUILDING_NODE_EXTENSION
#include <node.h>
extern "C" {
#include "mm.h"
}
#include "ShmdbObject.h"
#include <stdio.h>
#include <stdlib.h>


using namespace v8;

ShmdbObject::ShmdbObject() {
	hasInit = false;
};
ShmdbObject::~ShmdbObject() {};

void ShmdbObject::Init(Handle<Object> module) {
	// Prepare constructor template
	Local<FunctionTemplate> tpl = FunctionTemplate::New(New);//使用ShmdbObject::New函数作为构造函数
	tpl->SetClassName(String::NewSymbol("ShmdbObject"));//js中的类名为ShmdbObject
	tpl->InstanceTemplate()->SetInternalFieldCount(3);//指定js类的字段个数
	// Prototype
	Local< ObjectTemplate > prototypeTpl = tpl->PrototypeTemplate();
	prototypeTpl->Set(String::NewSymbol("showInfo"),//js类的成员函数名为plusOne
		FunctionTemplate::New(showInfo)->GetFunction());//js类的成员处理函数
	prototypeTpl->Set(String::NewSymbol("initChild"),
		FunctionTemplate::New(initChild)->GetFunction());
	prototypeTpl->Set(String::NewSymbol("put"),
		FunctionTemplate::New(put)->GetFunction());  
	prototypeTpl->Set(String::NewSymbol("get"),
		FunctionTemplate::New(get)->GetFunction());		
	prototypeTpl->Set(String::NewSymbol("remove"),
		FunctionTemplate::New(remove)->GetFunction());	
	prototypeTpl->Set(String::NewSymbol("destroy"),
		FunctionTemplate::New(destroy)->GetFunction());	

	Persistent<Function> constructor = Persistent<Function>::New/*New等价于js中的new*/(tpl->GetFunction());//new一个js实例
	module->Set(String::NewSymbol("exports"), constructor);
}


Handle<Value> ShmdbObject::New(const Arguments& args/*js中的参数*/) {
  HandleScope scope;

  ShmdbObject* obj = new ShmdbObject();
  obj->_length = args[0]->IsUndefined() ? 128 : args[0]->NumberValue()/*将js中的参数转化为c++类型*/;
  printf("num:%d\n",obj->_length);
  int rv = shmdb_initParent(&obj->_handle,obj->_length);
  if (rv == 0) {	
	obj->hasInit = true;
  } else {
	obj->hasInit = false;
  }
  
  obj->Wrap(args.This()/*将c++对象转化为js对象*/);

  return args.This();//返回这个js对象
}

Handle<Value> ShmdbObject::showInfo(const Arguments& args) {
  HandleScope scope;

  ShmdbObject* obj = ObjectWrap::Unwrap<ShmdbObject>(args.This());//将js对象转化为c++对象
  if (obj->hasInit) {
	STHashShareMemHead head;
	//printf("_length:%d\n handle.shmid:%d\n",obj->_length,obj->_handle.shmid);
	int rv = shmdb_getInfo(&obj->_handle,&head);
	if (rv == 0) {
		printf("totalLen:%d,baseLen:%d\n",head.totalLen,head.baseLen);
	} else {
		printf("shmdb_getInfo %x\n",rv);
	}
  } else {
	printf("shmdb has not inited\n");
  }
  
  return scope.Close(Number::New(obj->_length)/*转化为js中的数字*/);
}

Handle<Value> ShmdbObject::initChild(const v8::Arguments& args) {
	HandleScope scope;
	Local<Object> result = Object::New();
	ShmdbObject* obj = ObjectWrap::Unwrap<ShmdbObject>(args.This());
	if (obj->hasInit) {

		int rv = shmdb_initChild(&obj->_handle);
		result->Set(String::NewSymbol("code"), Number::New(rv));
	} else {
		result->Set(String::NewSymbol("code"), Number::New(ERROR_NOT_INIT));
		printf("shmdb has not inited\n");
	}
	return scope.Close(result);
}

Handle<Value> ShmdbObject::put(const v8::Arguments& args) {
	HandleScope scope;
	Local<Object> result = Object::New();
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
				
			result->Set(String::NewSymbol("code"), Number::New(rv));

		} else {
			result->Set(String::NewSymbol("code"), Number::New(ERROR_PRARAM_ERROR));
		}		
	} else {
		result->Set(String::NewSymbol("code"), Number::New(ERROR_NOT_INIT));
		printf("shmdb has not inited\n");
	}
	return scope.Close(result);
}

Handle<Value> ShmdbObject::get(const v8::Arguments& args) {
	HandleScope scope;
	Local<Object> result = Object::New();
	ShmdbObject* obj = ObjectWrap::Unwrap<ShmdbObject>(args.This());
	if (!obj->hasInit) {
		result->Set(String::NewSymbol("code"), Number::New(ERROR_NOT_INIT));
		printf("shmdb has not inited\n");
		return scope.Close(result);
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
		result->Set(String::NewSymbol("data"),str);
		free(valueBuffer);
	}
	result->Set(String::NewSymbol("code"), Number::New(rv));
	return scope.Close(result);
}

Handle<Value> ShmdbObject::remove(const v8::Arguments& args) {
	HandleScope scope;
	Local<Object> result = Object::New();
	ShmdbObject* obj = ObjectWrap::Unwrap<ShmdbObject>(args.This());
	if (!obj->hasInit) {
		result->Set(String::NewSymbol("code"), Number::New(ERROR_NOT_INIT));
		printf("shmdb has not inited\n");
		return scope.Close(result);
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
		result->Set(String::NewSymbol("data"),str);
		free(valueBuffer);
	}
	result->Set(String::NewSymbol("code"), Number::New(rv));
	return scope.Close(result);	
}

Handle<Value> ShmdbObject::destroy(const v8::Arguments& args) {
	HandleScope scope;
	Local<Object> result = Object::New();
	ShmdbObject* obj = ObjectWrap::Unwrap<ShmdbObject>(args.This());
	if (!obj->hasInit) {
		result->Set(String::NewSymbol("code"), Number::New(ERROR_NOT_INIT));
		printf("shmdb has not inited\n");
		return scope.Close(result);
	}
	int rv = shmdb_destroy(&obj->_handle);
	result->Set(String::NewSymbol("code"), Number::New(rv));
	return scope.Close(result);	
}



