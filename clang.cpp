#include <node.h>
#include <v8.h>

#include <vector>
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
  NODE_SET_PROTOTYPE_METHOD(constructor, "findFunction", findFunction);
  NODE_SET_PROTOTYPE_METHOD(constructor, "addSourceFile", addSourceFile);

  // This has to be last, otherwise the properties won't show up on the
  // object in JavaScript.
  target->Set(name, constructor->GetFunction());
}

Clang::Clang(std::string arguments)
  : ObjectWrap()
{
  index = clang_createIndex(0, 1);
}

Clang::~Clang()
{
  for (std::vector<CXTranslationUnit>::iterator it = tus.begin() ; it != tus.end(); ++it)
    clang_disposeTranslationUnit(*it);
  clang_disposeIndex(index);
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

Handle<Value> Clang::addSourceFile(const Arguments& args)
{
  HandleScope scope;

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
  std::string filename(*str);

  // Retrieves the pointer to the wrapped object instance.
  Clang * obj = ObjectWrap::Unwrap<Clang>(args.This());

  obj->tus.push_back(clang_parseTranslationUnit(obj->index, filename.c_str(), NULL, 0, NULL, 0, 0));

  return args.This();
}

//std::vector<std::string> str_vec;

CXChildVisitResult Clang::visitor(CXCursor c1, CXCursor c2, void* ptr)
{
  SearchInfo *sinfo_ptr = (SearchInfo*)ptr;

  CXCursorKind kind = clang_getCursorKind(c1);
  if( kind == CXCursor_FunctionDecl || kind == CXCursor_CallExpr )
  {
    std::string spelling(clang_getCString(clang_getCursorSpelling(c1)));
    if(spelling == sinfo_ptr->searchName)
    {
      HandleScope scope;

      CXFile cxfile;
      unsigned line, column, offset;
      clang_getSpellingLocation(clang_getCursorLocation(c1),
                &cxfile, &line, &column, &offset);

      CXString fn = clang_getFileName(cxfile);
      std::string filename(clang_getCString(fn));

      ResultInfo res;
      res.spelling = spelling;
      res.filename = filename;
      res.line     = line;
      sinfo_ptr->results.push_back(res);

      Handle<Value> argv[4] = {
        String::New("visitChildren"), // event name
        String::New(spelling.c_str()),
        String::New(filename.c_str()),
        Integer::New(line)
      };

      node::MakeCallback(sinfo_ptr->args->This(), "emit", 4, argv);
    }
  }
  return CXChildVisit_Recurse; // CXChildVisit_Continue, CXChildVisit_Break
}


Handle<Value> Clang::findFunction(const Arguments& args)
{
  HandleScope scope;

  if (args.Length() < 1)
  {
    return ThrowException(Exception::TypeError(
                            String::New("Missing argument function name")));
  }

  if ( !args[0]->IsString())
  {
    return ThrowException(Exception::TypeError(
                            String::New("Argument must be a string")));
  }

  String::Utf8Value str(args[0]->ToString());
  std::string funcname(*str);

  // Retrieves the pointer to the wrapped object instance.
  Clang * obj = ObjectWrap::Unwrap<Clang>(args.This());

  int i = 0;
  Local<Array> Result = Array::New();

  for (std::vector<CXTranslationUnit>::iterator it = obj->tus.begin() ; it != obj->tus.end(); ++it)
  {
    HandleScope scope;

    SearchInfo sinfo;
    sinfo.searchName = funcname;
    sinfo.args = &args;

    CXCursor cursor = clang_getTranslationUnitCursor(*it);

    (void)clang_visitChildren(cursor, obj->visitor, &sinfo);
    //Local<Object> res_obj = Object::New();
    //res_obj->Set(String::New("result"), Integer::New(res));
    //Result->Set(i++, res_obj);
    for (std::vector<ResultInfo>::iterator it = sinfo.results.begin() ; it != sinfo.results.end(); ++it)
    {
      HandleScope scope;
      Local<Object> node_obj = Object::New();
      node_obj->Set(String::New("name"), String::New((*it).spelling.c_str()));
      node_obj->Set(String::New("filename"), String::New((*it).filename.c_str()));
      node_obj->Set(String::New("line"), Integer::New((*it).line));

      Result->Set(i++, node_obj);
    }
  }
  return scope.Close(Result);
}

void init(Handle<Object> target)
{
  Clang::Init(target);
}

NODE_MODULE(clang, init)
