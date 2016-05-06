#include <vector>
#include <iostream>

#include "clang.h"

using namespace v8;

Persistent<Function> Clang::constructor;

void Clang::Init(Local<Object> target)
{
  Isolate* isolate = target->GetIsolate();

  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  Local<String> name = String::NewFromUtf8(isolate, "Clang");
  tpl->SetClassName(name);
  // ObjectWrap uses the first internal field to store the wrapped pointer.
  tpl->InstanceTemplate()->SetInternalFieldCount(1);


  // Add all prototype methods, getters and setters here.
  NODE_SET_PROTOTYPE_METHOD(tpl, "findFunction", findFunction);
  NODE_SET_PROTOTYPE_METHOD(tpl, "addSourceFile", addSourceFile);

  constructor.Reset(isolate, tpl->GetFunction());

  // This has to be last, otherwise the properties won't show up on the
  // object in JavaScript.
  target->Set(name, tpl->GetFunction());
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

void Clang::New(const FunctionCallbackInfo<Value>& args)
{
  Isolate* isolate = args.GetIsolate();

  if (!args.IsConstructCall())
  {
    isolate->ThrowException(Exception::TypeError(
                            String::NewFromUtf8(isolate, "Use the new operator to create instances of this object."))
                          );
  }

  if (args.Length() < 1)
  {
    isolate->ThrowException(Exception::TypeError(
                            String::NewFromUtf8(isolate, "Missing arguments")));
  }

  if ( !args[0]->IsString())
  {
    isolate->ThrowException(Exception::TypeError(
                            String::NewFromUtf8(isolate, "Argument must be a string")));

  }


  String::Utf8Value str(args[0]->ToString());
  std::string s(*str);

  // Creates a new instance object of this type and wraps it.
  Clang * obj = new Clang(s);
  obj->Wrap(args.This());

  args.GetReturnValue().Set(args.This());
}

void Clang::addSourceFile(const FunctionCallbackInfo<Value>& args)
{
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 1)
  {
    isolate->ThrowException(Exception::TypeError(
                            String::NewFromUtf8(isolate, "Missing arguments")));
  }

  if ( !args[0]->IsString())
  {
    isolate->ThrowException(Exception::TypeError(
                            String::NewFromUtf8(isolate, "Argument must be a string")));

  }


  String::Utf8Value str(args[0]->ToString());
  std::string filename(*str);

  // Retrieves the pointer to the wrapped object instance.
  Clang * obj = ObjectWrap::Unwrap<Clang>(args.This());

  obj->tus.push_back(clang_parseTranslationUnit(obj->index, filename.c_str(), NULL, 0, NULL, 0, 0));
}

CXChildVisitResult Clang::visitor(CXCursor c1, CXCursor c2, void* ptr)
{
  SearchInfo *sinfo_ptr = (SearchInfo*)ptr;

  Isolate* isolate = sinfo_ptr->args->GetIsolate();

  CXCursorKind kind = clang_getCursorKind(c1);
  if( kind == CXCursor_FunctionDecl || kind == CXCursor_CallExpr )
  {
    std::string spelling(clang_getCString(clang_getCursorSpelling(c1)));
    if(spelling == sinfo_ptr->searchName)
    {
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
        String::NewFromUtf8(isolate, "visitChildren"), // event name
        String::NewFromUtf8(isolate, spelling.c_str()),
        String::NewFromUtf8(isolate, filename.c_str()),
        Integer::New(isolate, line)
      };

      node::MakeCallback(isolate, sinfo_ptr->args->This(), "emit", 4, argv);
    }
  }
  return CXChildVisit_Recurse; // CXChildVisit_Continue, CXChildVisit_Break
}

void Clang::findFunction(const FunctionCallbackInfo<Value>& args)
{
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 1)
  {
    isolate->ThrowException(Exception::TypeError(
                            String::NewFromUtf8(isolate, "Missing argument function name")));
  }

  if ( !args[0]->IsString())
  {
    isolate->ThrowException(Exception::TypeError(
                            String::NewFromUtf8(isolate, "Argument must be a string")));
  }

  String::Utf8Value str(args[0]->ToString());
  std::string funcname(*str);

  // Retrieves the pointer to the wrapped object instance.
  Clang * obj = ObjectWrap::Unwrap<Clang>(args.Holder());

  int i = 0;
  Local<Array> Result = Array::New(isolate);

  for (std::vector<CXTranslationUnit>::iterator it = obj->tus.begin() ; it != obj->tus.end(); ++it)
  {
    SearchInfo sinfo;
    sinfo.searchName = funcname;
    sinfo.args = &args;

    CXCursor cursor = clang_getTranslationUnitCursor(*it);

    (void)clang_visitChildren(cursor, obj->visitor, &sinfo);

    for (std::vector<ResultInfo>::iterator it = sinfo.results.begin() ; it != sinfo.results.end(); ++it)
    {
      Local<Object> node_obj = Object::New(isolate);
      node_obj->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, (*it).spelling.c_str()));
      node_obj->Set(String::NewFromUtf8(isolate, "filename"), String::NewFromUtf8(isolate, (*it).filename.c_str()));
      node_obj->Set(String::NewFromUtf8(isolate, "line"), Integer::New(isolate, (*it).line));

      Result->Set(i++, node_obj);
    }
  }

  args.GetReturnValue().Set(Result);
}

void init(Handle<Object> target)
{
  Clang::Init(target);
}

NODE_MODULE(clang, init)
