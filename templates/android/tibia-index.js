var path = require("path");
var sep = path.sep;

module.exports = function (data, api) {
	api.generateFileFromTemplateFile(`data${sep}AndroidManifest.xml`, `data${sep}AndroidManifest.xml`, data);
	api.generateFileFromTemplateFile(`src${sep}MainActivity.java`, `src${sep}MainActivity.java`, data);
};
