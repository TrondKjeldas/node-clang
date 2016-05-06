#ifndef CLANG_H
#define CLANG_H

#include <string>

#include <node.h>
#include <node_object_wrap.h>

// Clang includes
#include <Index.h>

typedef struct
{
  std::string spelling;
  std::string filename;
  int         line;
}
ResultInfo;

typedef struct
{
  const v8::FunctionCallbackInfo<v8::Value> *args;
  std::string searchName;
  std::vector<ResultInfo> results;
}
SearchInfo;

class Clang : public node::ObjectWrap {
public:
    static void Init(v8::Local<v8::Object> target);

private:
    CXIndex index;
    std::vector<CXTranslationUnit> tus;
    static v8::Persistent<v8::Function> constructor;
    static CXChildVisitResult visitor(CXCursor, CXCursor, void*);

    Clang(std::string filename);
    ~Clang();

    static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void addSourceFile(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void findFunction(const v8::FunctionCallbackInfo<v8::Value>& args);
};

#endif
