var path = require("path");
var sep = path.sep;

module.exports = function (data, api) {
	api.generateFileFromTemplateFile(`src${sep}data.h`, `src${sep}data.h`, data);
	api.generateFileFromTemplateFile(`src${sep}index.html`, `src${sep}index.html`, data);
	api.copyFile(`src${sep}native.mm`, `src${sep}native.mm`);
	api.copyFile(`src${sep}app-Bridging-Header.h`, `src${sep}app-Bridging-Header.h`);
	api.copyFile(`src${sep}app.swift`, `src${sep}app.swift`, data);
};
