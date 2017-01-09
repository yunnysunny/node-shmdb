#include "ShmdbObject.h"
extern "C" {
#include "mm.h"
#include "log.h"
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace v8;

Nan::Persistent<Function> ShmdbObject::constructor;

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
	Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);//使用ShmdbNan::New<Object>函数作为构造函数
	tpl->SetClassName(Nan::New<String>("ShmdbObject").ToLocalChecked());//js中的类名为ShmdbObject
	tpl->InstanceTemplate()->SetInternalFieldCount(5);//指定js类的字段个数
	// Prototype
	Local< ObjectTemplate > prototypeTpl = tpl->PrototypeTemplate();

	prototypeTpl->Set(Nan::New<String>("put").ToLocalChecked(),//js类的成员函数名为put
		Nan::New<FunctionTemplate>(put)->GetFunction());  //js类的成员处理函数
	prototypeTpl->Set(Nan::New<String>("get").ToLocalChecked(),
		Nan::New<FunctionTemplate>(get)->GetFunction());		
	prototypeTpl->Set(Nan::New<String>("remove").ToLocalChecked(),
		Nan::New<FunctionTemplate>(remove)->GetFunction());	
	prototypeTpl->Set(Nan::New<String>("destroy").ToLocalChecked(),
		Nan::New<FunctionTemplate>(destroy)->GetFunction());	

	//Persistent<Function> constructor = Persistent<Function>::New/*New等价于js中的new*/(tpl->GetFunction());//new一个js实例
	constructor.Reset(tpl->GetFunction());
	module->Set(Nan::New<String>("exports").ToLocalChecked(), tpl->GetFunction());
}


NAN_METHOD(ShmdbObject::New) {


	ShmdbObject* obj = new ShmdbObject();
	obj->hasInit = false;
	Local<Value> param = info[0];
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
		_shmid = option->Get(Nan::New<String>("shmid").ToLocalChecked())->NumberValue();
		_semid = option->Get(Nan::New<String>("semid").ToLocalChecked())->NumberValue();
	}	
	
	if (needCreate) {
		STShmdbOption shmdbOption;
		memset(&shmdbOption,0,sizeof(shmdbOption));
		shmdbOption.logLevel = LEVEL_WARN;
		obj->_isParent = true;
		if (info.Length() == 2) {
			Local<Value> optionParam = info[1];
			if (optionParam->IsObject() && !optionParam->IsNull()) {
				Local<Value> logLevel = optionParam->ToObject()->Get(Nan::New<String>("logLevel").ToLocalChecked());
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
	

	obj->Wrap(info.This()/*将c++对象转化为js对象*/);
	info.This()->Set(Nan::New<String>("rv").ToLocalChecked(),Nan::New<Number>(rv));
	if (rv == 0) {
		info.This()->Set(Nan::New<String>("shmid").ToLocalChecked(),Nan::New<Number>(_shmid));
		info.This()->Set(Nan::New<String>("semid").ToLocalChecked(),Nan::New<Number>(_semid));
	}
	

	info.GetReturnValue().Set(info.This());//返回这个js对象
}


NAN_METHOD(ShmdbObject::put) {

	Local<Object> result = Nan::New<Object>();
	ShmdbObject* obj = ObjectWrap::Unwrap<ShmdbObject>(info.Holder());
	if (obj->hasInit) {
		if (info.Length() == 2) {

			v8::String::Utf8Value keyStr(info[0]->ToString());
			int keyLen = keyStr.length();
			char *keyBuffer = *keyStr;
			
			v8::String::Utf8Value  valueStr(info[1]->ToString());
			int valueLen = valueStr.length();
			char *valueBuffer = *valueStr;

			//printf("before put\n");
			int rv = shmdb_put(&obj->_handle,keyBuffer,(unsigned short)keyLen,
				valueBuffer,(unsigned short)valueLen);
			//printf("after put result:%x\n",rv);
				
			result->Set(Nan::New<String>("code").ToLocalChecked(), Nan::New<Number>(rv));

		} else {
			result->Set(Nan::New<String>("code").ToLocalChecked(), Nan::New<Number>(ERROR_PRARAM_ERROR));
		}		
	} else {
		result->Set(Nan::New<String>("code").ToLocalChecked(), Nan::New<Number>(ERROR_NOT_INIT));
		printf("shmdb has not inited\n");
	}
	info.GetReturnValue().Set(result);//todo
}

NAN_METHOD(ShmdbObject::get) {

	Local<Object> result = Nan::New<Object>();
	ShmdbObject* obj = ObjectWrap::Unwrap<ShmdbObject>(info.This());
	if (!obj->hasInit) {
		result->Set(Nan::New<String>("code").ToLocalChecked(), Nan::New<Number>(ERROR_NOT_INIT));
		printf("shmdb has not inited\n");
		return info.GetReturnValue().Set(result);
	}
	char *valueBuffer = NULL;
	unsigned short valueLen = 0;
	v8::String::Utf8Value keyStr(info[0]->ToString());
	int keyLen = keyStr.length();
	char *keyBuffer = *keyStr;
	//printf("before get\n");
	int rv = shmdb_get(&obj->_handle,keyBuffer,(unsigned short)keyLen,
				&valueBuffer,&valueLen);
	//printf("after get %x\n",rv);
	if (rv == 0) {
		Local<String> str = Nan::NewOneByteString((uint8_t *)valueBuffer,valueLen).ToLocalChecked();
		result->Set(Nan::New<String>("data").ToLocalChecked(),str);
		free(valueBuffer);
	}
	result->Set(Nan::New<String>("code").ToLocalChecked(), Nan::New<Number>(rv));
	info.GetReturnValue().Set(result);
}

NAN_METHOD(ShmdbObject::remove) {

	Local<Object> result = Nan::New<Object>();
	ShmdbObject* obj = ObjectWrap::Unwrap<ShmdbObject>(info.This());
	if (!obj->hasInit) {
		result->Set(Nan::New<String>("code").ToLocalChecked(), Nan::New<Number>(ERROR_NOT_INIT));
		printf("shmdb has not inited\n");
		return info.GetReturnValue().Set(result);
	}
	
	char *valueBuffer = NULL;
	unsigned short valueLen = 0;
	v8::String::Utf8Value keyStr(info[0]->ToString());
	int keyLen = keyStr.length();
	char *keyBuffer = *keyStr;
	
	int rv = shmdb_delete(&obj->_handle,keyBuffer,(unsigned short)keyLen,
				&valueBuffer,&valueLen);
	
	if (rv == 0) {
		Local<String> str = Nan::NewOneByteString((uint8_t *)valueBuffer,valueLen).ToLocalChecked();
		result->Set(Nan::New<String>("data").ToLocalChecked(),str);
		free(valueBuffer);
	}
	result->Set(Nan::New<String>("code").ToLocalChecked(), Nan::New<Number>(rv));
	info.GetReturnValue().Set(result);	
}

NAN_METHOD(ShmdbObject::destroy) {

	Local<Object> result = Nan::New<Object>();
	ShmdbObject* obj = ObjectWrap::Unwrap<ShmdbObject>(info.This());
	if (!obj->hasInit) {
		result->Set(Nan::New<String>("code").ToLocalChecked(), Nan::New<Number>(ERROR_NOT_INIT));
		printf("shmdb has not inited\n");
		return info.GetReturnValue().Set(result);
	}
	int rv = shmdb_destroy(&obj->_handle);
	result->Set(Nan::New<String>("code").ToLocalChecked(), Nan::New<Number>(rv));
	info.GetReturnValue().Set(result);	
}



