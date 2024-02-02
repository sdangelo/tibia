module.exports = function (data, api) {
	api.copyFile(`Makefile`, `Makefile`);
	api.generateFileFromTemplateFile(`vars.mk`, `vars.mk`, data);
	api.generateFileFromTemplateFile(`project.yml`, `project.yml`, data);
};
