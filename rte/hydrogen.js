//
// High Performance OpenGL Engine for JSX
// Copyright (C) 2010-2012 ASPECTRON Inc.
// All Rights Reserved.
//

var hydrogen = (function()
{
    var bindings = require("bindings");
    var hydrogen = new bindings.library("hydrogen");

    return hydrogen;

})();

exports.$ = hydrogen;
