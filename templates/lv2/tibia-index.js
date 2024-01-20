var path = require("path");
var sep = path.sep;

module.exports = function (data, api) {
	data.tibia.lv2 = {
		prefixes: [
			{ id: "atom",	uri: "http://lv2plug.in/ns/ext/atom#" },
			{ id: "doap",	uri: "http://usefulinc.com/ns/doap#" },
			{ id: "foaf",	uri: "http://xmlns.com/foaf/0.1/" },
			{ id: "log",	uri: "http://lv2plug.in/ns/ext/log#" },
			{ id: "lv2",	uri: "http://lv2plug.in/ns/lv2core#" },
			{ id: "midi",	uri: "http://lv2plug.in/ns/ext/midi#" },
			{ id: "pprops",	uri: "http://lv2plug.in/ns/ext/port-props#" },
			{ id: "rdf",	uri: "http://www.w3.org/1999/02/22-rdf-syntax-ns#" },
			{ id: "rdfs",	uri: "http://www.w3.org/2000/01/rdf-schema#" },
			{ id: "units",	uri: "http://lv2plug.in/ns/extensions/units#" },
			{ id: "urid",	uri: "http://lv2plug.in/ns/ext/urid#" }
		],
		units: {
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
		},
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

	var buses = data.product.buses;
	var audioPorts = [];
	var midiPorts = [];
	for (var bi = 0; bi < buses.length; bi++) {
		var b = buses[bi];
		if (b.type == "audio") {
			if (b.channels == "mono") {
				var e = { type: "audio", direction: b.direction, name: b.name, sidechain: b.sidechain, cv: b.cv, optional: b.optional, busIndex: bi };
				e.symbol = data.lv2.busSymbols[bi];
				audioPorts.push(e);
			} else {
				var e = { type: "audio", direction: b.direction, name: b.name + " Left", shortName: b.shortName + " L", sidechain: b.sidechain, cv: b.cv, busIndex: bi };
				e.symbol = data.lv2.busSymbols[bi] + "_L";
				data.tibia.lv2.ports.push(e);
				var e = { type: "audio", direction: b.direction, name: b.name + " Right", shortName: b.shortName + " R", sidechain: b.sidechain, cv: b.cv, busIndex: bi };
				e.symbol = data.lv2.busSymbols[bi] + "_R";
				audioPorts.push(e);
			}
		} else {
			var e =	{ type: "midi", direction: b.direction, name: b.name, sidechain: b.sidechain, control: b.control, optional: b.optional, busIndex: bi };
			e.symbol = data.lv2.busSymbols[bi];
			midiPorts.push(e);
		}
	}
	audioPorts.sort((a, b) => a.direction != b.direction ? (a.direction == "input" ? -1 : 1) : 0);
	midiPorts.sort((a, b) => a.direction != b.direction ? (a.direction == "input" ? -1 : 1) : 0);
	data.tibia.lv2.ports.push.apply(data.tibia.lv2.ports, audioPorts);
	data.tibia.lv2.ports.push.apply(data.tibia.lv2.ports, midiPorts);

	/*
	var audioBuses = data.product.buses.filter(x => x.type == "audio");
	var ports = [];
	var bi = 0;
	for (; bi < audioBuses.length; bi++) {
		var b = audioBuses[bi];
		if (b.channels == "mono") {
			var e = { type: "audio", direction: b.direction, name: b.name, sidechain: b.sidechain, cv: b.cv, optional: b.optional, busIndex: bi };
			e.symbol = data.lv2.busSymbols[bi];
			ports.push(e);
		} else {
			var e = { type: "audio", direction: b.direction, name: b.name + " Left", shortName: b.shortName + " L", sidechain: b.sidechain, cv: b.cv, busIndex: bi };
			e.symbol = data.lv2.busSymbols[bi] + "_L";
			data.tibia.lv2.ports.push(e);
			var e = { type: "audio", direction: b.direction, name: b.name + " Right", shortName: b.shortName + " R", sidechain: b.sidechain, cv: b.cv, busIndex: bi };
			e.symbol = data.lv2.busSymbols[bi] + "_R";
			ports.push(e);
		}
	}
	ports.sort((a, b) => a.direction != b.direction ? (a.direction == "input" ? -1 : 1) : 0);
	data.tibia.lv2.ports.push.apply(data.tibia.lv2.ports, ports);

	var midiBuses = data.product.buses.filter(x => x.type == "midi");
	var ports = [];
	for (var i = 0; i < midiBuses.length; i++, bi++) {
		var b = midiBuses[i];
		var e = { type: "midi", direction: b.direction, name: b.name, sidechain: b.sidechain, control: b.control, optional: b.optional, busIndex: bi };
		e.symbol = data.lv2.busSymbols[bi];
		ports.push(e);
	}
	ports.sort((a, b) => a.direction != b.direction ? (a.direction == "input" ? -1 : 1) : 0);
	data.tibia.lv2.ports.push.apply(data.tibia.lv2.ports, ports);
	*/

	var ports = [];
	for (var i = 0; i < data.product.parameters.length; i++) {
		var p = data.product.parameters[i];
		var e = Object.create(p);
		e.type = "control";
		e.symbol = data.lv2.parameterSymbols[i];
		e.paramIndex = i;
		ports.push(e);
	}
	ports.sort((a, b) => a.direction != b.direction ? (a.direction == "input" ? -1 : 1) : 0);
	data.tibia.lv2.ports.push.apply(data.tibia.lv2.ports, ports);

	api.generateFileFromTemplateFile(`data${sep}manifest.ttl`, `data${sep}manifest.ttl`, data);
	api.copyFile(`src${sep}lv2.c`, `src${sep}lv2.c`);
	api.generateFileFromTemplateFile(`src${sep}data.h`, `src${sep}data.h`, data);
};
