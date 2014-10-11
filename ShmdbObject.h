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

class ShmdbObject : public node::ObjectWrap {
 public:
	static void Init(v8::Handle<v8::Object> module);

 private:
	ShmdbObject();
	~ShmdbObject();

	static v8::Handle<v8::Value> New(const v8::Arguments& args);
	static v8::Handle<v8::Value> showInfo(const v8::Arguments& args);
	
	unsigned int counter_;
	STHashShareHandle _handle;
	bool hasInit;
};

#endif