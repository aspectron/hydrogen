pragma('event-queue');

require('rte');

var oxygen = require('oxygen');
var hydrogen = require('hydrogen');

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
		rt.exit(0);
		break;
	case oxygen.keys.F:
		window.toggle_fullscreen();
		break;
	}
});

var engine = new hydrogen.Engine(window, 
	{
		info: { show: rt.platform == 'windows' },
		vsync_interval: 1,
	});

var entity = new hydrogen.Entity(engine);
engine.world.attach(entity);

var camera = new hydrogen.Camera(engine);
engine.world.attach(camera);

console.log('Engine started. Press <ESC> to exit, <F> toggles fullscreen');
