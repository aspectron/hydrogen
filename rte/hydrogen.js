
var hydrogen = (function()
{
    var bindings = require("bindings");
    var hydrogen = new bindings.library("hydrogen");

    return hydrogen;

})();

exports.$ = hydrogen;
