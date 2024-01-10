var path = require("path");
var sep = path.sep;

module.exports = function (data, api) {
	data.tibia.vst3 = {
		units: {
			"bar":			"bars",
			"beat":			"beats",
			"bpm":			"BPM",
			"cent":			"ct",
			"cm":			"cm",
			"coef":			"",
			"db":			"dB",
			"degree":		"deg",
			"frame":		"frames",
			"hz":			"Hz",
			"inch":			"\"",
			"khz":			"kHz",
			"km":			"km",
			"m":			"m",
			"mhz":			"MHz",
			"midiNote":		"MIDI note",
			"mile":			"mi",
			"min":			"mins",
			"mm":			"mm",
			"ms":			"ms",
			"oct":			"octaves",
			"pc":			"%",
			"s":			"s",
			"semitone12TET":	"semi"
		}
	};

	for (var i = 0; i < data.product.parameters.length; i++)
		data.product.parameters[i].paramIndex = i;

	api.generateFileFromTemplateFile(`data${sep}Info.plist`, `data${sep}Info.plist`, data);
	api.copyFile(`src${sep}vst3.c`, `src${sep}vst3.c`);
	api.generateFileFromTemplateFile(`src${sep}data.h`, `src${sep}data.h`, data);
};
