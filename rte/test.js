var oxygen = require('oxygen');
var hydrogen = require('..');

var window = oxygen.Window({
	width: 640,
	height: 480,
	bpp: 32,
	caption: 'HYDROGEN',
	style : oxygen.styles.APPLICATION
})

window.on('close', function(){ rt.exit(0); });
window.on('keydown', function(e)
{
	switch (e.vk_code)
	{
	case oxygen.keys.ESCAPE:
		process.exit(0);
		break;
	case oxygen.keys.F:
		window.toggle_fullscreen();
		break;
	}
});

var engine = new hydrogen.Engine(window, 
	{
		info: { show: process.platform == 'win32' },
		vsync_interval: 1,
	});

var entity = new hydrogen.Entity(engine);
engine.world.attach(entity);

var camera = new hydrogen.Camera(engine);
engine.world.attach(camera);

console.log('Engine started. Press <ESC> to exit, <F> toggles fullscreen');
process.stdin.resume();