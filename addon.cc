#define BUILDING_NODE_EXTENSION
#include <node.h>
#include "ShmdbObject.h"

using namespace v8;

void InitAll(Handle<Object> exports, Handle<Object> module) {
  ShmdbObject::Init(module);
}

NODE_MODULE(shmdb, InitAll)