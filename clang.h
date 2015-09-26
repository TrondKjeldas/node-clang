#ifndef CLANG_H
#define CLANG_H

#include <string>

#include <node.h>

// Clang includes
#include <Index.h>


class Clang : public node::ObjectWrap {
public:
    static v8::Persistent<v8::FunctionTemplate> constructor;
    static void Init(v8::Handle<v8::Object> target);

    static CXChildVisitResult visitor(CXCursor, CXCursor, void*);

protected:
    Clang(std::string filename);

    static v8::Handle<v8::Value> New(const v8::Arguments& args);
    static v8::Handle<v8::Value> Next(const v8::Arguments& args);



    static v8::Handle<v8::Value> Value(const v8::Arguments& args);



    // Your own object variables here
    int value_;


    CXIndex index;
    CXTranslationUnit tu;
};

#endif
