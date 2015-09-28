var clang = require("./clang.js");

var index = new clang.Clang("");

index.addSourceFile("foo.cpp");
index.addSourceFile("bar.cpp");

index.on("visitChildren", function(symbol, filename, line) {
  console.log("Visited: " + symbol + " -> " + filename + "::" + line);
})

console.log( index.findFunction('foo') );

console.log( index.findFunction('bar') );
