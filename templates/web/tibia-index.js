var path = require("path");
var sep = path.sep;

module.exports = function (data, api) {
	api.copyFile(`src${sep}memset.h`, `src${sep}memset.h`);
	api.copyFile(`src${sep}walloc.h`, `src${sep}walloc.h`);
	api.copyFile(`src${sep}processor.c`, `src${sep}processor.c`);
	api.generateFileFromTemplateFile(`src${sep}data.h`, `src${sep}data.h`, data);
	api.generateFileFromTemplateFile(`src${sep}processor.js`, `src${sep}processor.js`, data);
	api.generateFileFromTemplateFile(`src${sep}module.js`, `src${sep}module.js`, data);
};
