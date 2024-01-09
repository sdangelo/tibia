var path = require("path");
var sep = path.sep;

module.exports = function (data, api) {
	data.tibia.lv2 = {
		prefixes: [
			{ id: "doap", uri: "http://usefulinc.com/ns/doap#" },
			{ id: "foaf", uri: "http://xmlns.com/foaf/0.1/" },
			{ id: "lv2", uri: "http://lv2plug.in/ns/lv2core#" },
			{ id: "rdf", uri: "http://www.w3.org/1999/02/22-rdf-syntax-ns#" },
			{ id: "rdfs", uri: "http://www.w3.org/2000/01/rdf-schema#" },
			{ id: "units", uri: "http://lv2plug.in/ns/extensions/units#" }
		],
		ports: [],

		ttlURI: function (uri) {
			return uri.charAt(0) == "@" ? uri.substring(1) : "<" + uri + ">";
		},

		expandURI: function (uri) {
			if (uri.charAt(0) != "@")
				return uri;
			var i = uri.indexOf(":");
			var p = uri.substring(1, uri.indexOf(":"));
			return data.tibia.lv2.prefixes.find(x => x.id == p).uri + uri.substring(i + 1);
		}
	};

	for (var id in data.lv2.prefixes)
		data.tibia.lv2.prefixes.push({ id: id, uri: data.lv2.prefixes[id] });

	for (var i = 0; i < data.product.parameters.length; i++) {
		var p = data.product.parameters[i];
		var e = Object.create(p);
		e.type = "control";
		e.symbol = data.lv2.parameterSymbols[i];
		e.paramIndex = i;
		data.tibia.lv2.ports.push(e);
	}

	var audioBuses = data.product.buses.filter(x => x.type == "audio");
	for (var i = 0; i < audioBuses.length; i++) {
		var b = audioBuses[i];
		for (var j = 0; j < b.channels; j++) {
			var e = { type: "audio", direction: b.direction, name: b.name, sidechain: b.sidechain, cv: b.cv };
			e.symbol = data.lv2.busSymbols[i] + "_" + j;
			data.tibia.lv2.ports.push(e);
		}
	}

	data.tibia.lv2.ports.sort((a, b) => a.type != b.type ? (a.type == "audio" ? -1 : 1) : (a.direction != b.direction ? (a.direction == "input" ? -1 : 1) : 0));

	data.tibia.lv2.units = {
		"bar":			"@units:bar",
		"beat":			"@units:beat",
		"bpm":			"@units:bpm",
		"cent":			"@units:cent",
		"cm":			"@units:cm",
		"coef":			"@units:coef",
		"db":			"@units:db",
		"degree":		"@units:degree",
		"frame":		"@units:frame",
		"hz":			"@units:hz",
		"inch":			"@units:inch",
		"khz":			"@units:khz",
		"km":			"@units:km",
		"m":			"@units:m",
		"mhz":			"@units:mhz",
		"midiNote":		"@units:midiNote",
		"mile":			"@units:mile",
		"min":			"@units:min",
		"mm":			"@units:mm",
		"ms":			"@units:ms",
		"oct":			"@units:oct",
		"pc":			"@units:pc",
		"s":			"@units:s",
		"semitone12TET":	"@units:semitone12TET"
	};

	api.generateFileFromTemplateFile(`data${sep}manifest.ttl`, `data${sep}manifest.ttl`, data);
	api.copyFile(`src${sep}lv2.c`, `src${sep}lv2.c`);
	api.generateFileFromTemplateFile(`src${sep}data.h`, `src${sep}data.h`, data);
};
