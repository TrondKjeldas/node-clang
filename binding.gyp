{
  "targets": [
    {
      "target_name": "clang",
      "sources": [ "clang.cpp" ],
      "include_dirs": [ "/usr/local/include/clang-c/" ],
      "libraries": [ "-lclang", "-L/usr/lib/llvm-3.6/lib" ] # libclang-3.6.so
      }
    ]
}
