const data = {
	company: {{=JSON.stringify(it.company, null, 2)}},
	product: {{=JSON.stringify(it.product, null, 2)}}
};

export class Module {
	static get data() { return data; }

	async init(context, processorJsPath, wasmPath) {
		var wasmBytes = await fetch(wasmPath)
					.then(response => response.arrayBuffer())
					.then(bytes => bytes);
		if (!wasmBytes.byteLength)
			throw "Could not fetch WebAssembly module";

		await context.audioWorklet.addModule(processorJsPath);

		this.context = context;
		this.wasmBytes = wasmBytes;
	}

	context = null;
	wasmBytes = null;
}

export class Node extends AudioWorkletNode {
	constructor(module) {
		super(module.context, data.product.bundleName,
			{
				numberOfInputs: data.product.buses.filter(b => b.type == "audio" && b.direction == "input").length,
				numberOfOutputs: data.product.buses.filter(b => b.type == "audio" && b.direction == "output").length,
				outputChannelCount: data.product.buses.filter(b => b.type == "audio" && b.direction == "output").map(b => b.channels == "mono" ? 1 : 2),
				processorOptions: { wasmBytes: module.wasmBytes }
			});
	}
}
