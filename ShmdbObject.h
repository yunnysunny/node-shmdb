#ifndef MYOBJECT_H
#define MYOBJECT_H

#include <nan.h>
#include "platform.h"
#if __IS_WIN__
//#include <windows.h>
#else
#endif
extern "C" {
#include "mm.h"
}

#define ERROR_NOT_INIT		0xe0000001
#define	ERROR_PRARAM_ERROR	0xe0000002

class ShmdbObject : public node::ObjectWrap {
 public:
	static void Init(v8::Handle<v8::Object> module);

 private:
	ShmdbObject();
	~ShmdbObject();
	
	// static char *getBuffer(v8::Local<v8::String>);
	static NAN_METHOD(New);
	static NAN_METHOD(put);
	static NAN_METHOD(get);
	static NAN_METHOD(remove);
	static NAN_METHOD(dump);
	static NAN_METHOD(destroy);
    static Nan::Persistent<v8::Function> constructor;
	
	unsigned int _length;
	STHashShareHandle _handle;
	bool _isParent;
	bool hasInit;
};

#endif