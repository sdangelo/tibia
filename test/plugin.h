/*
 * Tibia
 *
 * Copyright (C) 2023, 2024 Orastron Srl unipersonale
 *
 * Tibia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * Tibia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Tibia.  If not, see <http://www.gnu.org/licenses/>.
 *
 * File author: Stefano D'Angelo
 */

typedef struct plugin {
	float	sample_rate;
	size_t	delay_line_length;

	float	gain;
	float	delay;
	float	cutoff;
	char	bypass;

	float *	delay_line;
	size_t	delay_line_cur;
	float	z1;
	float	cutoff_k;
	float	yz1;
} plugin;

static void plugin_init(plugin *instance) {
	(void)instance;
}

static void plugin_fini(plugin *instance) {
	(void)instance;
}

static void plugin_set_sample_rate(plugin *instance, float sample_rate) {
	instance->sample_rate = sample_rate;
	//safe approx instance->delay_line_length = ceilf(sample_rate) + 1;
	instance->delay_line_length = (size_t)(sample_rate + 1.f) + 1;
}

static size_t plugin_mem_req(plugin *instance) {
	return instance->delay_line_length * sizeof(float);
}

static void plugin_mem_set(plugin *instance, void *mem) {
	instance->delay_line = (float *)mem;
}

static void plugin_reset(plugin *instance) {
	for (size_t i = 0; i < instance->delay_line_length; i++)
		instance->delay_line[i] = 0.f;
	instance->delay_line_cur = 0;
	instance->z1 = 0.f;
	instance->cutoff_k = 1.f;
	instance->yz1 = 0.f;
}

static void plugin_set_parameter(plugin *instance, size_t index, float value) {
	switch (index) {
	case 0:
		//approx instance->gain = powf(10.f, 0.05f * value);
		instance->gain = ((2.6039890429412597e-4f * value + 0.032131027163547855f) * value + 1.f) / ((0.0012705124328080768f * value - 0.0666763481312185f) * value + 1.f);
		break;
	case 1:
		instance->delay = 0.001f * value;
		break;
	case 2:
		instance->cutoff = value;
		break;
	case 3:
		instance->bypass = value >= 0.5f;
		break;
	}
}

static float plugin_get_parameter(plugin *instance, size_t index) {
	(void)index;
	return instance->yz1;
}

static size_t calc_index(size_t cur, size_t delay, size_t len) {
	return (cur < delay ? cur + len : cur) - delay;
}

static void plugin_process(plugin *instance, const float **inputs, float **outputs, size_t n_samples) {
	//approx size_t delay = roundf(instance->sample_rate * instance->delay);
	size_t delay = (size_t)(instance->sample_rate * instance->delay + 0.5f);
	const float mA1 = instance->sample_rate / (instance->sample_rate + 6.283185307179586f * instance->cutoff * instance->cutoff_k);
	for (size_t i = 0; i < n_samples; i++) {
		instance->delay_line[instance->delay_line_cur] = inputs[0][i];
		const float x = instance->delay_line[calc_index(instance->delay_line_cur, delay, instance->delay_line_length)];
		instance->delay_line_cur++;
		if (instance->delay_line_cur == instance->delay_line_length)
			instance->delay_line_cur = 0;
		const float y = x + mA1 * (instance->z1 - x);
		instance->z1 = y;
		outputs[0][i] = instance->bypass ? inputs[0][i] : instance->gain * y;
		instance->yz1 = outputs[0][i];
	}
}

static void plugin_midi_msg_in(plugin *instance, size_t index, const uint8_t * data) {
	(void)index;
	if (((data[0] & 0xf0) == 0x90) && (data[2] != 0))
		//approx instance->cutoff_k = powf(2.f, (1.f / 12.f) * (note - 60));
		instance->cutoff_k = data[1] < 64 ? (-0.19558034980097166f * data[1] - 2.361735109225749f) / (data[1] - 75.57552349522389f) : (393.95397927344214f - 7.660826245588588f * data[1]) / (data[1] - 139.0755234952239f);
}

#ifdef TEMPLATE_HAS_UI
# define PLUGIN_UI

# include <pugl/pugl.h>
# include <pugl/cairo.h>
# include <cairo.h>

typedef struct {
	void *			widget;
	PuglWorld *		world;
	PuglView *		view;

	double			fw;
	double			fh;
	double			x;
	double			y;
	double			w;
	double			h;

	float			gain;
	float			delay;
	float			cutoff;
	char			bypass;
	float			y_z1;

	plugin_ui_callbacks	cbs;
} plugin_ui;

#define WIDTH		600.0
#define HEIGHT		400.0
#define RATIO		(WIDTH / HEIGHT)
#define INV_RATIO	(HEIGHT / WIDTH)

static void plugin_ui_get_default_size(uint32_t *width, uint32_t *height) {
	*width = WIDTH;
	*height = HEIGHT;
}

static void plugin_ui_update_geometry(plugin_ui *instance) {
	PuglRect frame = puglGetFrame(instance->view);
	instance->fw = frame.width;
	instance->fh = frame.height;
	if (frame.width == 0 || frame.height == 0)
		return;

	if (instance->fw / instance->fh > RATIO) {
		instance->w = RATIO * instance->fh;
		instance->h = instance->fh;
		instance->x = 0.5 * (instance->fw - instance->w);
		instance->y = 0.0;
	} else {
		instance->w = instance->fw;
		instance->h = INV_RATIO * instance->fw;
		instance->x = 0.0;
		instance->y = 0.5 * (instance->fh - instance->h);
	}
}

static void plugin_ui_draw(plugin_ui *instance) {
	cairo_t *cr = (cairo_t *)puglGetContext(instance->view);
	double x = instance->x;
	double y = instance->y;
	double w = instance->w;
	double h = instance->h;

	cairo_set_line_width(cr, 0.005 * h);

	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_paint(cr);

	cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
	cairo_rectangle(cr, x, y, w, h);
	cairo_fill(cr);

	cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
	cairo_rectangle(cr, x + 0.1 * w, y + 0.15 * h, 0.8 * w, 0.1 * h);
	cairo_fill(cr);
	cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
	cairo_rectangle(cr, x + 0.1 * w, y + 0.15 * h, 0.8 * w * instance->gain, 0.1 * h);
	cairo_fill(cr);
	cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
	cairo_rectangle(cr, x + 0.1 * w, y + 0.15 * h, 0.8 * w, 0.1 * h);
	cairo_stroke(cr);

	cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
	cairo_rectangle(cr, x + 0.1 * w, y + 0.3 * h, 0.8 * w, 0.1 * h);
	cairo_fill(cr);
	cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
	cairo_rectangle(cr, x + 0.1 * w, y + 0.3 * h, 0.8 * w * instance->delay, 0.1 * h);
	cairo_fill(cr);
	cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
	cairo_rectangle(cr, x + 0.1 * w, y + 0.3 * h, 0.8 * w, 0.1 * h);
	cairo_stroke(cr);

	cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
	cairo_rectangle(cr, x + 0.1 * w, y + 0.45 * h, 0.8 * w, 0.1 * h);
	cairo_fill(cr);
	cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
	cairo_rectangle(cr, x + 0.1 * w, y + 0.45 * h, 0.8 * w * instance->cutoff, 0.1 * h);
	cairo_fill(cr);
	cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
	cairo_rectangle(cr, x + 0.1 * w, y + 0.45 * h, 0.8 * w, 0.1 * h);
	cairo_stroke(cr);

	if (instance->bypass)
		cairo_set_source_rgb(cr, 1.0, 0, 0);
	else
		cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
	cairo_rectangle(cr, x + 0.4 * w, y + 0.6 * h, 0.2 * w, 0.1 * h);
	cairo_fill(cr);

	cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
	cairo_rectangle(cr, x + 0.1 * w, y + 0.75 * h, 0.8 * w, 0.1 * h);
	cairo_fill(cr);

	cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
	cairo_rectangle(cr, x + 0.1 * w, y + 0.75 * h, 0.8 * w * instance->y_z1, 0.1 * h);
	cairo_fill(cr);
}

static PuglStatus plugin_ui_on_event(PuglView *view, const PuglEvent *event) {
	switch (event->type) {
		case PUGL_CONFIGURE:
		{
			plugin_ui *instance = (plugin_ui *)puglGetHandle(view);
			plugin_ui_update_geometry(instance);
		}
			break;
		case PUGL_BUTTON_RELEASE:
		{
			plugin_ui *instance = (plugin_ui *)puglGetHandle(view);
			PuglRect frame = puglGetFrame(view);
			const PuglButtonEvent *ev = (const PuglButtonEvent *)event;
			double x = instance->x;
			double y = instance->y;
			double w = instance->w;
			double h = instance->h;

			if (ev->x >= x + 0.1 * w && ev->x <= x + 0.9 * w
			    && ev->y >= y + 0.15 * h && ev->y <= y + 0.25 * h) {
				instance->gain = (float)((ev->x - (x + 0.1 * w)) / (0.8 * w));
				instance->cbs.set_parameter(instance->cbs.handle, 0, -60.f + 80.f * instance->gain);
				puglPostRedisplay(instance->view);
			} else if (ev->x >= x + 0.1 * w && ev->x <= x + 0.9 * w
			    && ev->y >= y + 0.3 * h && ev->y <= y + 0.4 * h) {
				instance->delay = (float)((ev->x - (x + 0.1 * w)) / (0.8 * w));
				instance->cbs.set_parameter(instance->cbs.handle, 1, 1000.f * instance->delay);
				puglPostRedisplay(instance->view);
			} else if (ev->x >= x + 0.1 * w && ev->x <= x + 0.9 * w
			    && ev->y >= y + 0.45 * h && ev->y <= y + 0.55 * h) {
				instance->cutoff = (float)((ev->x - (x + 0.1 * w)) / (0.8 * w));
				instance->cbs.set_parameter(instance->cbs.handle, 2, (632.4555320336746f * instance->cutoff + 20.653108640674372f) / (1.0326554320337158f - instance->cutoff));
				puglPostRedisplay(instance->view);
			} 
		}
			break;
		case PUGL_EXPOSE:
		{
			plugin_ui *instance = (plugin_ui *)puglGetHandle(view);
			plugin_ui_draw(instance);
		}
			break;
		default:
			break;
	}
	return PUGL_SUCCESS;
}

static plugin_ui *plugin_ui_create(char has_parent, void *parent, plugin_ui_callbacks *cbs) {
	plugin_ui *instance = malloc(sizeof(plugin_ui));
	if (instance == NULL)
		return NULL;
	instance->world = puglNewWorld(PUGL_MODULE, 0);
	instance->view = puglNewView(instance->world);
	puglSetSizeHint(instance->view, PUGL_DEFAULT_SIZE, WIDTH, HEIGHT);
	puglSetViewHint(instance->view, PUGL_RESIZABLE, PUGL_TRUE);
	puglSetBackend(instance->view, puglCairoBackend());
	PuglRect frame = { 0, 0, WIDTH, HEIGHT };
	puglSetFrame(instance->view, frame);
	puglSetEventFunc(instance->view, plugin_ui_on_event);
	if (has_parent)
		puglSetParentWindow(instance->view, (PuglNativeView)parent);
	if (puglRealize(instance->view)) {
		puglFreeView(instance->view);
		puglFreeWorld(instance->world);
		return NULL;
	}
	puglShow(instance->view, PUGL_SHOW_RAISE);
	puglSetHandle(instance->view, instance);
	instance->widget = (void *)puglGetNativeView(instance->view);
	instance->cbs = *cbs;
	return instance;
}

static void plugin_ui_free(plugin_ui *instance) {
	puglFreeView(instance->view);
	puglFreeWorld(instance->world);
	free(instance);
}

static void plugin_ui_idle(plugin_ui *instance) {
	puglUpdate(instance->world, 0);
}

static void plugin_ui_set_parameter(plugin_ui *instance, size_t index, float value) {
	switch (index) {
	case 0:
		instance->gain = 0.0125f * value + 0.75f;
		break;
	case 1:
		instance->delay = 0.001f * value;
		break;
	case 2:
		// (bad) approx log unmap
		instance->cutoff = (1.0326554320337176f * value - 20.65310864067435f) / (value + 632.4555320336754f);
		break;
	case 3:
		instance->bypass = value >= 0.5f;
		break;
	case 4:
		instance->y_z1 = 0.5f * value + 0.5f;
		break;
	}
	puglPostRedisplay(instance->view);
}

#endif
