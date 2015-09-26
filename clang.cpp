#include <node.h>
#include <v8.h>

#include <iostream>

#include "clang.h"

using namespace v8;



Persistent<FunctionTemplate> Clang::constructor;

void Clang::Init(Handle<Object> target)
{
  HandleScope scope;

  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  Local<String> name = String::NewSymbol("Clang");

  constructor = Persistent<FunctionTemplate>::New(tpl);
  // ObjectWrap uses the first internal field to store the wrapped pointer.
  constructor->InstanceTemplate()->SetInternalFieldCount(1);
  constructor->SetClassName(name);

  // Add all prototype methods, getters and setters here.
  NODE_SET_PROTOTYPE_METHOD(constructor, "value", Value);
  NODE_SET_PROTOTYPE_METHOD(constructor, "next", Next);

  // This has to be last, otherwise the properties won't show up on the
  // object in JavaScript.
  target->Set(name, constructor->GetFunction());
}

Clang::Clang(std::string filename)
  : ObjectWrap()
{
  index = clang_createIndex(0, 1);

  tu = clang_parseTranslationUnit(index, filename.c_str(), NULL, 0, NULL, 0, 0);
}


Handle<Value> Clang::New(const Arguments& args)
{
  HandleScope scope;

  if (!args.IsConstructCall())
  {
    return ThrowException(Exception::TypeError(
                            String::New("Use the new operator to create instances of this object."))
                          );
  }

  if (args.Length() < 1)
  {
    return ThrowException(Exception::TypeError(
                            String::New("Missing arguments")));
  }

  if ( !args[0]->IsString())
  {
    return ThrowException(Exception::TypeError(
                            String::New("Argument must be a string")));

  }


  String::Utf8Value str(args[0]->ToString());
  std::string s(*str);

  // Creates a new instance object of this type and wraps it.
  Clang * obj = new Clang(s);
  obj->Wrap(args.This());

  return args.This();
}

CXChildVisitResult Clang::visitor(CXCursor c1, CXCursor c2, void* ptr)
{
  CXString s = clang_getCursorSpelling(c1);

  std::cout << "visiting: " <<  clang_getCString(s) << "\n";

  return CXChildVisit_Recurse; // CXChildVisit_Continue, CXChildVisit_Break
}


Handle<Value> Clang::Next(const Arguments& args)
{
  HandleScope scope;

  if (args.Length() > 0)
  {
    return ThrowException(Exception::TypeError(
                            String::New("Argument not expected")));
  }

  // Retrieves the pointer to the wrapped object instance.
  Clang * obj = ObjectWrap::Unwrap<Clang>(args.This());

  CXCursor cursor = clang_getTranslationUnitCursor(obj->tu);

  unsigned res = clang_visitChildren(cursor, obj->visitor, NULL);

  return scope.Close(Integer::New(0));
}



Handle<Value> Clang::Value(const Arguments& args)
{
  HandleScope scope;

  // Retrieves the pointer to the wrapped object instance.
  Clang * obj = ObjectWrap::Unwrap<Clang>(args.This());

  return scope.Close(Integer::New(obj->value_));
}




Handle<Value> Method(const Arguments& args)
{
  HandleScope scope;

  return scope.Close(String::New("hello"));
}

Handle<Value> Method2(const Arguments& args)
{
  HandleScope scope;

  return scope.Close(String::New("world"));
}

Handle<Value> createIndex(const Arguments& args)
{
  HandleScope scope;

  CXIndex idx = clang_createIndex(0, 1);

  return scope.Close(String::New("world"));
}


void init(Handle<Object> target)
{
  target->Set(String::NewSymbol("hello"),
              FunctionTemplate::New(Method)->GetFunction());
  target->Set(String::NewSymbol("world"),
              FunctionTemplate::New(Method2)->GetFunction());
  target->Set(String::NewSymbol("createIndex"),
              FunctionTemplate::New(createIndex)->GetFunction());

  Clang::Init(target);
}

NODE_MODULE(clang, init)
