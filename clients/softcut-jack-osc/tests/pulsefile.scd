n = NetAddr("localhost", 9999);

~setup = {

	// load file
	f = "/home/emb/code/softcut-lib/pulse.wav";

	n.sendMsg("/softcut/buffer/clear");
	n.sendMsg("/softcut/buffer/read_mono", f, 0, 0, 0.5, 0, 0);

//	n.sendMsg("/softcut/buffer/write_mono", "/home/emb/code/softcut-lib/pulse_cutloop.wav", 0, 1.0, 0);

	n.sendMsg("/set/enabled/cut", 0, 1);
	n.sendMsg("/set/level/cut", 0, 0.5);
	n.sendMsg("/set/pan/cut", 0, 0);

	n.sendMsg("/set/param/cut/buffer", 0, 0);

	n.sendMsg("/set/param/cut/loop_start", 0, 0);
	n.sendMsg("/set/param/cut/loop_end", 0, 1);
	n.sendMsg("/set/param/cut/loop_flag", 0, 0);

	n.sendMsg("/set/param/cut/fade_time", 0, 0);

	n.sendMsg("/set/param/cut/post_filter_lp", 0, 0);
	n.sendMsg("/set/param/cut/post_filter_hp", 0, 0);
	n.sendMsg("/set/param/cut/post_filter_bp", 0, 0);
	n.sendMsg("/set/param/cut/post_filter_br", 0, 0);
	n.sendMsg("/set/param/cut/post_filter_dry", 0, 1);
	n.sendMsg("/set/param/cut/play_flag", 0, 1);

};

~bang = {
	n.sendMsg("/set/param/cut/position", 0, 0);
};

r = Routine {

	var delta = 0.5;
	~setup.value;
	0.2.wait;

	20.do({
		~bang.value;
		delta = delta * 0.9;
		delta.wait;
	});
}.play;