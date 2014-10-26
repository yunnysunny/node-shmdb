#ifndef MYOBJECT_H
#define MYOBJECT_H

#include <node.h>
#include "platform.h"
#if __IS_WIN__
#include <windows.h>
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
	static v8::Handle<v8::Value> New(const v8::Arguments& args);
	static v8::Handle<v8::Value> showInfo(const v8::Arguments& args);
	static v8::Handle<v8::Value> initChild(const v8::Arguments& args);
	static v8::Handle<v8::Value> put(const v8::Arguments& args);
	static v8::Handle<v8::Value> get(const v8::Arguments& args);
	static v8::Handle<v8::Value> remove(const v8::Arguments& args);
	static v8::Handle<v8::Value> dump(const v8::Arguments& args);
	static v8::Handle<v8::Value> destroy(const v8::Arguments& args);
	
	unsigned int _length;
	STHashShareHandle _handle;
	bool hasInit;
};

#endif