var path = require("path");
var sep = path.sep;

module.exports = function (data, api) {
	api.generateFileFromTemplateFile(`src${sep}data.h`, `src${sep}data.h`, data);
	api.copyFile(`src${sep}main.c`, `src${sep}main.c`);
};
