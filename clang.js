var Clang = require("./build/Release/clang.node").Clang;
var events = require('events');


inherits(Clang, events.EventEmitter);
exports.Clang = Clang;

// extend prototype
function inherits(target, source) {
  for (var k in source.prototype) {
    target.prototype[k] = source.prototype[k];
  }
}
