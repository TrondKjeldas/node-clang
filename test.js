var clang = require("./build/Release/clang.node");

var idx = clang.createIndex();

console.log( clang.hello() + clang.world() );


var obj = new clang.Clang("foo.cpp");
console.warn(obj);

obj.next();
