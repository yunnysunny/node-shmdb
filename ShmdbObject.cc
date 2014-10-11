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
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);//ʹ��ShmdbObject::New������Ϊ���캯��
  tpl->SetClassName(String::NewSymbol("ShmdbObject"));//js�е�����ΪShmdbObject
  tpl->InstanceTemplate()->SetInternalFieldCount(3);//ָ��js����ֶθ���
  // Prototype
  tpl->PrototypeTemplate()->Set(String::NewSymbol("showInfo"),//js��ĳ�Ա������ΪplusOne
      FunctionTemplate::New(showInfo)->GetFunction());//js��ĳ�Ա������

  Persistent<Function> constructor = Persistent<Function>::New/*New�ȼ���js�е�new*/(tpl->GetFunction());//newһ��jsʵ��
  module->Set(String::NewSymbol("exports"), constructor);
}

Handle<Value> ShmdbObject::New(const Arguments& args/*js�еĲ���*/) {
  HandleScope scope;

  ShmdbObject* obj = new ShmdbObject();
  obj->counter_ = args[0]->IsUndefined() ? 128 : args[0]->NumberValue()/*��js�еĲ���ת��Ϊc++����*/;
  printf("num:%d\n",obj->counter_);
  int rv = shmdb_initParent(&obj->_handle,obj->counter_);
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
  printf("counter_:%d\n handle.shmid:%d\n",obj->counter_,obj->_handle.shmid);
  //obj->counter_ += 1;
  

  return scope.Close(Number::New(obj->counter_)/*ת��Ϊjs�е�����*/);
}