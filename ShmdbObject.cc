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
	Local<FunctionTemplate> tpl = FunctionTemplate::New(New);//ʹ��ShmdbObject::New������Ϊ���캯��
	tpl->SetClassName(String::NewSymbol("ShmdbObject"));//js�е�����ΪShmdbObject
	tpl->InstanceTemplate()->SetInternalFieldCount(3);//ָ��js����ֶθ���
	// Prototype
	Local< ObjectTemplate > prototypeTpl = tpl->PrototypeTemplate();
	prototypeTpl->Set(String::NewSymbol("showInfo"),//js��ĳ�Ա������ΪplusOne
		FunctionTemplate::New(showInfo)->GetFunction());//js��ĳ�Ա������
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

	Persistent<Function> constructor = Persistent<Function>::New/*New�ȼ���js�е�new*/(tpl->GetFunction());//newһ��jsʵ��
	module->Set(String::NewSymbol("exports"), constructor);
}


Handle<Value> ShmdbObject::New(const Arguments& args/*js�еĲ���*/) {
  HandleScope scope;

  ShmdbObject* obj = new ShmdbObject();
  obj->_length = args[0]->IsUndefined() ? 128 : args[0]->NumberValue()/*��js�еĲ���ת��Ϊc++����*/;
  printf("num:%d\n",obj->_length);
  int rv = shmdb_initParent(&obj->_handle,obj->_length);
  if (rv == 0) {	
	obj->hasInit = true;
  } else {
	obj->hasInit = false;
  }
  
  obj->Wrap(args.This()/*��c++����ת��Ϊjs����*/);

  return args.This();//�������js����
}

Handle<Value> ShmdbObject::showInfo(const Arguments& args) {
  HandleScope scope;

  ShmdbObject* obj = ObjectWrap::Unwrap<ShmdbObject>(args.This());//��js����ת��Ϊc++����
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
  
  return scope.Close(Number::New(obj->_length)/*ת��Ϊjs�е�����*/);
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



