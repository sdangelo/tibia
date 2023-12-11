#define DATA_COMPANY_NAME			"{{=it.company.name}}"
#define DATA_COMPANY_URL			"{{=it.company.url}}"
#define DATA_COMPANY_EMAIL			"{{=it.company.email}}"

#define DATA_PLUGIN_NAME			"{{=it.plugin.name}}"
#define DATA_PLUGIN_VERSION			"{{=it.plugin.version}}"

#define DATA_VST3_PLUGIN_CID_1			0x{{=it.vst3.plugin.cid.substring(0, 8)}}
#define DATA_VST3_PLUGIN_CID_2			0x{{=it.vst3.plugin.cid.substring(8, 16)}}
#define DATA_VST3_PLUGIN_CID_3			0x{{=it.vst3.plugin.cid.substring(16, 24)}}
#define DATA_VST3_PLUGIN_CID_4			0x{{=it.vst3.plugin.cid.substring(24, 32)}}

#define DATA_VST3_CONTROLLER_CID_1		0x{{=it.vst3.controller.cid.substring(0, 8)}}
#define DATA_VST3_CONTROLLER_CID_2		0x{{=it.vst3.controller.cid.substring(8, 16)}}
#define DATA_VST3_CONTROLLER_CID_3		0x{{=it.vst3.controller.cid.substring(16, 24)}}
#define DATA_VST3_CONTROLLER_CID_4		0x{{=it.vst3.controller.cid.substring(24, 32)}}

#define DATA_VST3_SUBCATEGORY			"{{=it.vst3.subCategory}}"

#define DATA_PLUGIN_BUSES_AUDIO_INPUT_N		{{=it.plugin.buses.filter(x => x.type == "audio" && x.direction == "input").length}}
#define DATA_PLUGIN_BUSES_AUDIO_OUTPUT_N	{{=it.plugin.buses.filter(x => x.type == "audio" && x.direction == "output").length}}
#define DATA_PLUGIN_BUSES_EVENT_INPUT_N		{{=it.plugin.buses.filter(x => x.type == "event" && x.direction == "input").length}}
#define DATA_PLUGIN_BUSES_EVENT_OUTPUT_N	{{=it.plugin.buses.filter(x => x.type == "event" && x.direction == "output").length}}

#if DATA_PLUGIN_BUSES_AUDIO_INPUT_N > 0
static struct Steinberg_Vst_BusInfo busInfoAudioInput[DATA_PLUGIN_BUSES_AUDIO_INPUT_N] = {
{{~it.plugin.buses.filter(x => x.type == "audio" && x.direction == "input") :b}}
	{
		/* .mediaType		= */ Steinberg_Vst_MediaTypes_kAudio,
		/* .direction		= */ Steinberg_Vst_BusDirections_kInput,
		/* .channelCount	= */ {{=b.channels}},
		/* .name		= */ { {{~Array.from(b.name) :c}}0x{{=c.charCodeAt(0).toString(16)}}, {{~}}0 },
		/* .busType		= */ {{?b.aux}}Steinberg_Vst_BusTypes_kAux{{??}}Steinberg_Vst_BusTypes_kMain{{?}},
		/* .flags		= */ {{?b.cv}}Steinberg_Vst_BusInfo_BusFlags_kIsControlVoltage{{??}}Steinberg_Vst_BusInfo_BusFlags_kDefaultActive{{?}}
	},
{{~}}
};
#endif

#if DATA_PLUGIN_BUSES_AUDIO_OUTPUT_N > 0
static struct Steinberg_Vst_BusInfo busInfoAudioOutput[DATA_PLUGIN_BUSES_AUDIO_OUTPUT_N] = {
{{~it.plugin.buses.filter(x => x.type == "audio" && x.direction == "output") :b}}
	{
		/* .mediaType		= */ Steinberg_Vst_MediaTypes_kAudio,
		/* .direction		= */ Steinberg_Vst_BusDirections_kOutput,
		/* .channelCount	= */ {{=b.channels}},
		/* .name		= */ { {{~Array.from(b.name) :c}}0x{{=c.charCodeAt(0).toString(16)}}, {{~}}0 },
		/* .busType		= */ {{?b.aux}}Steinberg_Vst_BusTypes_kAux{{??}}Steinberg_Vst_BusTypes_kMain{{?}},
		/* .flags		= */ {{?b.cv}}Steinberg_Vst_BusInfo_BusFlags_kIsControlVoltage{{??}}Steinberg_Vst_BusInfo_BusFlags_kDefaultActive{{?}}
	},
{{~}}
};
#endif

#if DATA_PLUGIN_BUSES_EVENT_INPUT_N > 0
static struct Steinberg_Vst_BusInfo busInfoEventInput[DATA_PLUGIN_BUSES_EVENT_INPUT_N] = {
{{~it.plugin.buses.filter(x => x.type == "event" && x.direction == "input") :b}}
	{
		/* .mediaType		= */ Steinberg_Vst_MediaTypes_kEvent,
		/* .direction		= */ Steinberg_Vst_BusDirections_kInput,
		/* .channelCount	= */ {{=b.channels}},
		/* .name		= */ { {{~Array.from(b.name) :c}}0x{{=c.charCodeAt(0).toString(16)}}, {{~}}0 },
		/* .busType		= */ Steinberg_Vst_BusTypes_kMain,
		/* .flags		= */ Steinberg_Vst_BusInfo_BusFlags_kDefaultActive
	},
{{~}}
};
#endif

#if DATA_PLUGIN_BUSES_EVENT_OUTPUT_N > 0
static struct Steinberg_Vst_BusInfo busInfoAudioInput[DATA_PLUGIN_BUSES_EVENT_OUTPUT_N] = {
{{~it.plugin.buses.filter(x => x.type == "event" && x.direction == "output") :b}}
	{
		/* .mediaType		= */ Steinberg_Vst_MediaTypes_kEvent,
		/* .direction		= */ Steinberg_Vst_BusDirections_kOutput,
		/* .channelCount	= */ {{=b.channels}},
		/* .name		= */ { {{~Array.from(b.name) :c}}0x{{=c.charCodeAt(0).toString(16)}}, {{~}}0 },
		/* .busType		= */ Steinberg_Vst_BusTypes_kMain,
		/* .flags		= */ Steinberg_Vst_BusInfo_BusFlags_kDefaultActive
	},
{{~}}
};
#endif

#define DATA_PLUGIN_PARAMETERS_N		{{=it.plugin.parameters.length}}

#if DATA_PLUGIN_PARAMETERS_N > 0
static struct Steinberg_Vst_ParameterInfo parameterInfo[DATA_PLUGIN_PARAMETERS_N] = {
{{~it.plugin.parameters :p:i}}
	{
		/* .id				= */ {{=i}},
		/* .title			= */ { {{~Array.from(p.name) :c}}0x{{=c.charCodeAt(0).toString(16)}}, {{~}}0 },
		/* .shortTitle			= */ { {{~Array.from(p.shortName) :c}}0x{{=c.charCodeAt(0).toString(16)}}, {{~}}0 },
		/* .units			= */ { {{~Array.from(p.units) :c}}0x{{=c.charCodeAt(0).toString(16)}}, {{~}}0 },
		/* .stepCount			= */ {{=p.steps}},
		/* .defaultNormalizedValue	= */ {{=p.defaultValue}},
		/* .unitId			= */ 0,
		/* .flags			= */ {{?p.isBypass}}Steinberg_Vst_ParameterInfo_ParameterFlags_kIsBypass | {{?}}{{?p.direction == "input"}}Steinberg_Vst_ParameterInfo_ParameterFlags_kCanAutomate{{??}}Steinberg_Vst_ParameterInfo_ParameterFlags_kIsReadOnly{{?}}
	},
{{~}}
};
#endif
