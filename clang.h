#ifndef CLANG_H
#define CLANG_H

#include <string>

#include <node.h>

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
  std::string searchName;
  std::vector<ResultInfo> results;
}
SearchInfo;

class Clang : public node::ObjectWrap {
public:
    static v8::Persistent<v8::FunctionTemplate> constructor;
    static void Init(v8::Handle<v8::Object> target);

    static CXChildVisitResult visitor(CXCursor, CXCursor, void*);

protected:
    Clang(std::string filename);
    ~Clang();

    static v8::Handle<v8::Value> New(const v8::Arguments& args);
    static v8::Handle<v8::Value> addSourceFile(const v8::Arguments& args);
    static v8::Handle<v8::Value> findFunction(const v8::Arguments& args);

private:
    CXIndex index;
    std::vector<CXTranslationUnit> tus;
};

#endif
