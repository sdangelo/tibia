#define DATA_LV2_URI	"{{=it.tibia.CGetUTF8StringLiteral(it.tibia.lv2.expandURI(it.lv2.uri))}}"

#define DATA_PRODUCT_AUDIO_INPUT_CHANNELS_N	{{=it.product.buses.filter(x => x.type == "audio" && x.direction == "input").reduce((s, x) => s += x.channels, 0)}}
#define DATA_PRODUCT_AUDIO_OUTPUT_CHANNELS_N	{{=it.product.buses.filter(x => x.type == "audio" && x.direction == "output").reduce((s, x) => s += x.channels, 0)}}
#define DATA_PRODUCT_CONTROL_INPUTS_N		{{=it.product.parameters.filter(x => x.direction == "input").length}}
#define DATA_PRODUCT_CONTROL_OUTPUTS_N		{{=it.product.parameters.filter(x => x.direction == "output").length}}

#if DATA_PRODUCT_CONTROL_INPUTS_N > 0
static float param_defaults[DATA_PRODUCT_CONTROL_INPUTS_N] = {
	{{~it.tibia.lv2.ports.filter(x => x.type == "control" && x.direction == "input") :p}}{{=p.defaultValue.toExponential()}}f, {{~}}
};
#endif
