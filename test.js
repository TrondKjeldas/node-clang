var clang = require("./build/Release/clang.node");

var index = new clang.Clang("");

index.addSourceFile("foo.cpp");
index.addSourceFile("bar.cpp");

console.log( index.findFunction('foo') );

console.log( index.findFunction('bar') );
