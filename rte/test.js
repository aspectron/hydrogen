SET_EXECUTION_OPTIONS(EVENT_QUEUE | NO_GC_NOTIFY);

require("rte");

var log = require("log");

//var oxygen = require("oxygen");

log.info("loading hydrogen");
var hydrogen = require("hydrogen");
log.info("hydrogen loaded");

/*
log.info("loading oxygen");
var oxygen = require("oxygen");
log.info("loading hydrogen");
var hydrogen = require("hydrogen");

log.info("Creating window...");

var window = oxygen.window({
	width: 640,
	height: 480,
	bpp: 32,
	caption: "OXYGEN",
	style : 0
})

log.info("Window created...");


dpc(5000, function(){
	log.info("Destroying window...");
	window.destroy();
})

var engine = new hydrogen.engine(window);

*/