var clang = require("./build/Release/clang.node");

var index = new clang.Clang("foo.cpp");

console.log( index.findFunction('foo') );

console.log( index.findFunction('main') );
