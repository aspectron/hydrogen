SET_EXECUTION_OPTIONS(EVENT_QUEUE | NO_GC_NOTIFY);

var log = require("log");

var hydrogen = require("hydrogen");


log.info("Creating window...");

var window = hydrogen.window({
	width: 640,
	height: 480,
	bpp: 32,
	caption: "HYDROGEN",
	style : 0
})

log.info("Window created...");

dpc(5000, function(){
	log.info("Destroying window...");
	window.destroy();
})