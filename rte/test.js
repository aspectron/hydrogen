pragma("event-queue");

require("rte");

var log = require("log");

log.info("loading oxygen");
var oxygen = require("oxygen");

log.info("loading hydrogen");
var hydrogen = require("hydrogen");

log.info("Creating window...");

var window = oxygen.Window({
	width: 640,
	height: 480,
	bpp: 32,
	caption: "OXYGEN",
	style : oxygen.styles.APPLICATION
})

window.on('close', function(){ rt.exit(0); });
window.on('keydown', function(e)
{
	switch (e.vk_code)
	{
	case oxygen.keys.ESCAPE:
		window.destroy();
		rt.exit(0);
		break;
	case oxygen.keys.F:
		window.toggle_fullscreen();
		break;
	}
});

log.info("Window created. Press <ESC> to exit, <F> toggles fullscreen");

dpc(500000, function(){
	log.info("Destroying window...");
	window.destroy();
})

var engine = new hydrogen.Engine(window, 
	{
		info: { show: rt.platform=="windows" },
		vsync_interval: 1,
	});
