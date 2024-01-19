/* 
 * Copyright (C) 2022, 2024 Orastron Srl unipersonale
 */

var buses = {{=JSON.stringify(it.product.buses, null, 2)}};
var parameters = {{=JSON.stringify(it.product.parameters, null, 2)}};

class Processor extends AudioWorkletProcessor {
	constructor(options) {
		super();

		var module = new WebAssembly.Module(options.processorOptions.wasmBytes);
		var instance = new WebAssembly.Instance(module, { env: {} });
		this.module = instance.exports;

		this.instance = this.module.processor_new(sampleRate);
		if (!this.instance)
			throw "Could not instantiate processor module";

		function getBuffers(p, output) {
			var ret = [];
			for (var i = 0; i < buses.length; i++) {
				if (buses[i].type != "audio" || (output && buses[i].direction == "input") || (!output && buses[i].direction == "output"))
					continue;
				if (buses[i].channels == "mono") {
					ret.push([ new Float32Array(this.module.memory.buffer, p, 128) ]);
					p += 128 * 4;
				} else {
					ret.push([
						new Float32Array(this.module.memory.buffer, p, 128),
						new Float32Array(this.module.memory.buffer, p + 128 * 4, 128)
					]);
					p += 2 * 128 * 4;
				}
			}
			return ret;
		}
		this.x = getBuffers.call(this, this.module.processor_get_x_buf(this.instance), false);
		this.y = getBuffers.call(this, this.module.processor_get_y_buf(this.instance), true);

		this.parametersIn = [];
		for (var i = 0; i < parameters.length; i++)
			if (parameters[i].direction == "input")
				this.parametersIn.push({ index: i, value: NaN });

		this.parametersOut = [];
		for (var i = 0; i < parameters.length; i++)
			if (parameters[i].direction == "output")
				this.parametersOut.push({ index: i, value: NaN });

		if (this.parametersOut.length > 0)
			this.parametersOutValues = new Float32Array(this.module.memory.buffer, this.module.processor_get_out_params(this.instance), this.parametersOut.length);
		
		this.paramOutChangeMsg = { type: "paramOutChange", index: NaN, value: NaN };

		var self = this;
		this.port.onmessage = function (e) {
			switch (e.data.type) {
			case "noteOn":
				self.module.processor_note_on(self.instance, e.data.index, e.data.note, e.data.velocity);
				break;
			case "noteOff":
				self.module.processor_note_off(self.instance, e.data.index, e.data.note, e.data.velocity);
				break;
			case "allSoundsOff":
				self.module.processor_all_sounds_off(self.instance, e.data.index);
				break;
			case "allNotesOff":
				self.module.processor_all_notes_off(self.instance, e.data.index);
				break;
			case "channelPressure":
				self.module.processor_channel_pressure(self.instance, e.data.index, e.data.value);
				break;
			case "pitchBendChange":
				self.module.processor_pitch_bend_change(self.instance, e.data.index, e.data.value);
				break;
			}
		};
	}

	process(inputs, outputs, params) {
		for (var i = 0; i < this.parametersIn.length; i++) {
			var index = this.parametersIn[i].index;
			var parameter = parameters[index];
			var name = parameter.name;
			var value = params[name][0];
			if (value != this.parametersIn[i].value) {
				if (parameter.isBypass || parameter.toggled)
					value = value > 0 ? 1 : 0;
				else if (parameter.integer)
					value = Math.round(value);
				value = Math.min(Math.max(value, parameter.minimum), parameter.maximum);
				if (value != this.parametersIn[i].value) {
					this.module.processor_set_parameter(this.instance, index, value);
					this.parametersIn[i].value = value;
				}
			}
		}

		var n = outputs[0][0].length;
		var i = 0;
		while (i < n) {
			var s = Math.min(n - i, 128);

			for (var j = 0; j < this.x.length; j++) {
				var input = inputs[j];
				if (!input.length) {
					for (var k = 0; k < this.x[j].length; k++)
						this.x[j][k].fill(0);
				} else {
					for (var k = 0; k < this.x[j].length; k++)
						if (k <= input.length)
							this.x[j][k].set(input[k].subarray(i, s));
						else
							this.x[j][k].fill(0);
				}
			}

			this.module.processor_process(this.instance, s);

			for (var j = 0; j < this.y.length; j++) {
				var output = outputs[j];
				for (var k = 0; k < this.y[j].length; k++)
					output[k].set(this.y[j][k].subarray(i, s));
			}

			i += s;
		}
		
		for (var i = 0; i < this.parametersOut.length; i++) {
			var index = this.parametersOut[i].index;
			var value = this.parametersOutValues[i];
			if (value != this.parametersOut[i].value) {
				this.paramOutChangeMsg.index = index;
				this.paramOutChangeMsg.value = value;
				this.port.postMessage(this.paramOutChangeMsg);
				this.parametersOut[i].value = value;
			}
		}

		return true; // because Chrome sucks: https://bugs.chromium.org/p/chromium/issues/detail?id=921354
	}

	static get parameterDescriptors() {
		var ret = [];
		for (var i = 0; i < parameters.length; i++) {
			var p = parameters[i];
			if (p.direction == "output")
				continue;
			ret.push({ name: p.name, minValue: p.minimum, maxValue: p.maximum, defaultValue: p.defaultValue, automationRate: "k-rate" });
		}
		return ret;
	}
}

registerProcessor("{{=it.product.bundleName}}", Processor);
