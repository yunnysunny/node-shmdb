#define BUILDING_NODE_EXTENSION
#include <node.h>
extern "C" {
#include "mm.h"
}
#include "ShmdbObject.h"


#include <stdio.h>

using namespace v8;

ShmdbObject::ShmdbObject() {};
ShmdbObject::~ShmdbObject() {};

void ShmdbObject::Init(Handle<Object> module) {
  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);//使用ShmdbObject::New函数作为构造函数
  tpl->SetClassName(String::NewSymbol("ShmdbObject"));//js中的类名为ShmdbObject
  tpl->InstanceTemplate()->SetInternalFieldCount(3);//指定js类的字段个数
  // Prototype
  tpl->PrototypeTemplate()->Set(String::NewSymbol("showInfo"),//js类的成员函数名为plusOne
      FunctionTemplate::New(showInfo)->GetFunction());//js类的成员处理函数

  Persistent<Function> constructor = Persistent<Function>::New/*New等价于js中的new*/(tpl->GetFunction());//new一个js实例
  module->Set(String::NewSymbol("exports"), constructor);
}

Handle<Value> ShmdbObject::New(const Arguments& args/*js中的参数*/) {
  HandleScope scope;

  ShmdbObject* obj = new ShmdbObject();
  obj->counter_ = args[0]->IsUndefined() ? 128 : args[0]->NumberValue()/*将js中的参数转化为c++类型*/;
  printf("num:%d\n",obj->counter_);
  int rv = shmdb_initParent(&obj->_handle,obj->counter_);
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
  printf("counter_:%d\n handle.shmid:%d\n",obj->counter_,obj->_handle.shmid);
  //obj->counter_ += 1;
  

  return scope.Close(Number::New(obj->counter_)/*转化为js中的数字*/);
}