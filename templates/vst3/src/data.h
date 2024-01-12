#define DATA_COMPANY_NAME			"{{=it.tibia.CGetUTF8StringLiteral(it.company.name, 63)}}"
#define DATA_COMPANY_URL			"{{=it.tibia.CGetUTF8StringLiteral(it.company.url, 255)}}"
#define DATA_COMPANY_EMAIL			"{{=it.tibia.CGetUTF8StringLiteral(it.company.email, 127)}}"

#define DATA_PRODUCT_NAME			"{{=it.tibia.CGetUTF8StringLiteral(it.product.name, 63)}}"
#define DATA_PRODUCT_VERSION			"{{=(it.product.version + '.' + it.product.buildVersion).substring(0, 63)}}"

static Steinberg_char16 dataProductNameW[64] = { {{~Array.from(it.product.name).slice(0, 63) :c}}0x{{=c.charCodeAt(0).toString(16)}}, {{~}}0 };
static Steinberg_char16 dataProductVersionW[64] = { {{~Array.from(it.product.version + "." + it.product.buildVersion).slice(0, 63) :c}}0x{{=c.charCodeAt(0).toString(16)}}, {{~}}0 };

#define DATA_VST3_SDK_VERSION			"VST 3.7.9"
static Steinberg_char16 dataVST3SDKVersionW[64] = { {{~Array.from("VST 3.7.9") :c}}0x{{=c.charCodeAt(0).toString(16)}}, {{~}}0 };

static Steinberg_char16 dataVST3ControllerNameW[64] = { {{~Array.from(it.product.name + " Controller").slice(0, 63) :c}}0x{{=c.charCodeAt(0).toString(16)}}, {{~}}0 };

#define DATA_VST3_PLUGIN_CID_1			0x{{=it.vst3.plugin.cid.substring(0, 8)}}
#define DATA_VST3_PLUGIN_CID_2			0x{{=it.vst3.plugin.cid.substring(8, 16)}}
#define DATA_VST3_PLUGIN_CID_3			0x{{=it.vst3.plugin.cid.substring(16, 24)}}
#define DATA_VST3_PLUGIN_CID_4			0x{{=it.vst3.plugin.cid.substring(24, 32)}}

#define DATA_VST3_CONTROLLER_CID_1		0x{{=it.vst3.controller.cid.substring(0, 8)}}
#define DATA_VST3_CONTROLLER_CID_2		0x{{=it.vst3.controller.cid.substring(8, 16)}}
#define DATA_VST3_CONTROLLER_CID_3		0x{{=it.vst3.controller.cid.substring(16, 24)}}
#define DATA_VST3_CONTROLLER_CID_4		0x{{=it.vst3.controller.cid.substring(24, 32)}}

static const Steinberg_TUID dataPluginCID = SMTG_INLINE_UID(DATA_VST3_PLUGIN_CID_1, DATA_VST3_PLUGIN_CID_2, DATA_VST3_PLUGIN_CID_3, DATA_VST3_PLUGIN_CID_4);
static const Steinberg_TUID dataControllerCID = SMTG_INLINE_UID(DATA_VST3_CONTROLLER_CID_1, DATA_VST3_CONTROLLER_CID_2, DATA_VST3_CONTROLLER_CID_3, DATA_VST3_CONTROLLER_CID_4);

#define DATA_VST3_SUBCATEGORY			"{{=it.vst3.subCategory}}"

#define DATA_PRODUCT_BUSES_AUDIO_INPUT_N	{{=it.product.buses.filter(x => x.type == "audio" && x.direction == "input").length}}
#define DATA_PRODUCT_BUSES_AUDIO_OUTPUT_N	{{=it.product.buses.filter(x => x.type == "audio" && x.direction == "output").length}}
#define DATA_PRODUCT_BUSES_EVENT_INPUT_N	{{=it.product.buses.filter(x => x.type == "event" && x.direction == "input").length}}
#define DATA_PRODUCT_BUSES_EVENT_OUTPUT_N	{{=it.product.buses.filter(x => x.type == "event" && x.direction == "output").length}}

#define DATA_PRODUCT_CHANNELS_AUDIO_INPUT_N	{{=it.product.buses.filter(x => x.type == "audio" && x.direction == "input").reduce((a, x) => a + (x.channels == "mono" ? 1 : 2), 0)}}
#define DATA_PRODUCT_CHANNELS_AUDIO_OUTPUT_N	{{=it.product.buses.filter(x => x.type == "audio" && x.direction == "output").reduce((a, x) => a + (x.channels == "mono" ? 1 : 2), 0)}}

#if DATA_PRODUCT_BUSES_AUDIO_INPUT_N > 0
static struct Steinberg_Vst_BusInfo busInfoAudioInput[DATA_PRODUCT_BUSES_AUDIO_INPUT_N] = {
{{~it.product.buses.filter(x => x.type == "audio" && x.direction == "input") :b}}
	{
		/* .mediaType		= */ Steinberg_Vst_MediaTypes_kAudio,
		/* .direction		= */ Steinberg_Vst_BusDirections_kInput,
		/* .channelCount	= */ {{=b.channels == "mono" ? 1 : 2}},
		/* .name		= */ { {{~Array.from(b.name) :c}}0x{{=c.charCodeAt(0).toString(16)}}, {{~}}0 },
		/* .busType		= */ {{?b.sidechain}}Steinberg_Vst_BusTypes_kAux{{??}}Steinberg_Vst_BusTypes_kMain{{?}},
		/* .flags		= */ 0{{?b.cv}} | Steinberg_Vst_BusInfo_BusFlags_kIsControlVoltage{{?}}{{?!b.optional}} | Steinberg_Vst_BusInfo_BusFlags_kDefaultActive{{?}}
	},
{{~}}
};
#endif

#if DATA_PRODUCT_BUSES_AUDIO_OUTPUT_N > 0
static struct Steinberg_Vst_BusInfo busInfoAudioOutput[DATA_PRODUCT_BUSES_AUDIO_OUTPUT_N] = {
{{~it.product.buses.filter(x => x.type == "audio" && x.direction == "output") :b}}
	{
		/* .mediaType		= */ Steinberg_Vst_MediaTypes_kAudio,
		/* .direction		= */ Steinberg_Vst_BusDirections_kOutput,
		/* .channelCount	= */ {{=b.channels == "mono" ? 1 : 2}},
		/* .name		= */ { {{~Array.from(b.name) :c}}0x{{=c.charCodeAt(0).toString(16)}}, {{~}}0 },
		/* .busType		= */ {{?b.sidechain}}Steinberg_Vst_BusTypes_kAux{{??}}Steinberg_Vst_BusTypes_kMain{{?}},
		/* .flags		= */ 0{{?b.cv}} | Steinberg_Vst_BusInfo_BusFlags_kIsControlVoltage{{?}}{{?!b.optional}} | Steinberg_Vst_BusInfo_BusFlags_kDefaultActive{{?}}
	},
{{~}}
};
#endif

#if DATA_PRODUCT_BUSES_EVENT_INPUT_N > 0
static struct Steinberg_Vst_BusInfo busInfoEventInput[DATA_PRODUCT_BUSES_EVENT_INPUT_N] = {
{{~it.product.buses.filter(x => x.type == "event" && x.direction == "input") :b}}
	{
		/* .mediaType		= */ Steinberg_Vst_MediaTypes_kEvent,
		/* .direction		= */ Steinberg_Vst_BusDirections_kInput,
		/* .channelCount	= */ 1,
		/* .name		= */ { {{~Array.from(b.name) :c}}0x{{=c.charCodeAt(0).toString(16)}}, {{~}}0 },
		/* .busType		= */ Steinberg_Vst_BusTypes_kMain,
		/* .flags		= */ 0{{?!b.optional}} | Steinberg_Vst_BusInfo_BusFlags_kDefaultActive{{?}}
	},
{{~}}
};
#endif

#if DATA_PRODUCT_BUSES_EVENT_OUTPUT_N > 0
static struct Steinberg_Vst_BusInfo busInfoAudioInput[DATA_PRODUCT_BUSES_EVENT_OUTPUT_N] = {
{{~it.product.buses.filter(x => x.type == "event" && x.direction == "output") :b}}
	{
		/* .mediaType		= */ Steinberg_Vst_MediaTypes_kEvent,
		/* .direction		= */ Steinberg_Vst_BusDirections_kOutput,
		/* .channelCount	= */ 1,
		/* .name		= */ { {{~Array.from(b.name) :c}}0x{{=c.charCodeAt(0).toString(16)}}, {{~}}0 },
		/* .busType		= */ Steinberg_Vst_BusTypes_kMain,
		/* .flags		= */ 0{{?!b.optional}} | Steinberg_Vst_BusInfo_BusFlags_kDefaultActive{{?}}
	},
{{~}}
};
#endif

#define DATA_PRODUCT_PARAMETERS_N		{{=it.product.parameters.filter(x => !x.isLatency).length}}

#if DATA_PRODUCT_PARAMETERS_N > 0
static struct Steinberg_Vst_ParameterInfo parameterInfo[DATA_PRODUCT_PARAMETERS_N] = {
{{~it.product.parameters.filter(x => !x.isLatency) :p:i}}
	{
		/* .id				= */ {{=i}},
{{?p.isBypass}}
		/* .title			= */ { {{~Array.from("Bypass") :c}}0x{{=c.charCodeAt(0).toString(16)}}, {{~}}0 },
		/* .shortTitle			= */ { {{~Array.from("Bypass") :c}}0x{{=c.charCodeAt(0).toString(16)}}, {{~}}0 },
		/* .units			= */ { 0 },
		/* .stepCount			= */ 1,
		/* .defaultNormalizedValue	= */ 0.0,
		/* .unitId			= */ 0,
		/* .flags			= */ Steinberg_Vst_ParameterInfo_ParameterFlags_kIsBypass | Steinberg_Vst_ParameterInfo_ParameterFlags_kCanAutomate
{{??}}
		/* .title			= */ { {{~Array.from(p.name) :c}}0x{{=c.charCodeAt(0).toString(16)}}, {{~}}0 },
		/* .shortTitle			= */ { {{~Array.from(p.shortName) :c}}0x{{=c.charCodeAt(0).toString(16)}}, {{~}}0 },
		/* .units			= */ { {{~Array.from(p.unit in it.tibia.vst3.units ? it.tibia.vst3.units[p.unit] : "") :c}}0x{{=c.charCodeAt(0).toString(16)}}, {{~}}0 },
		/* .stepCount			= */ {{?p.toggled}}1{{??p.list && p.scalePoints.length > 1}}Number(1 / (p.scalePoints.length - 1)).toExponential(){{??p.integer}}Number(p.maximum - p.minimum).toExponential(){{??}}0{{?}},
		/* .defaultNormalizedValue	= */ {{=Number((p.defaultValue - p.minimum) / (p.maximum - p.minimum)).toExponential()}},
		/* .unitId			= */ 0,
		/* .flags			= */ {{?p.direction == "input"}}Steinberg_Vst_ParameterInfo_ParameterFlags_kCanAutomate{{??}}Steinberg_Vst_ParameterInfo_ParameterFlags_kIsReadOnly{{?}}
{{?}}
	},
{{~}}
};

# define DATA_PARAM_BYPASS	1
# define DATA_PARAM_TOGGLED	(1<<1)
# define DATA_PARAM_INTEGER	(1<<2)

static struct {
	size_t		index;
	double		min;
	double		max;
	double		def;
	uint32_t	flags;
	// scalePoints?
} parameterData[DATA_PRODUCT_PARAMETERS_N] = {
{{~it.product.parameters.filter(x => !x.isLatency) :p:i}}
	{
		/* .index	= */ {{=p.paramIndex}},
		/* .min		= */ {{=p.minimum.toExponential()}},
		/* .max		= */ {{=p.maximum.toExponential()}},
		/* .def		= */ {{=p.defaultValue.toExponential()}},
		/* .flags	= */ {{?p.isBypass}}DATA_PARAM_BYPASS{{??}}0{{?p.toggled}} | DATA_PARAM_TOGGLED{{?}}{{?p.integer}} | DATA_PARAM_INTEGER{{?}}{{?}}
	},
{{~}}
};
#endif

{{?it.product.parameters.find(x => x.isLatency)}}
# define DATA_PARAM_LATENCY_INDEX	{{=it.product.parameters.find(x => x.isLatency).paramIndex}}
{{?}}
