#include <Lib/ScanLib.h>
#include <print.h>
#include <kll_defs.h>

#define MAX_LAYERS	3
#define MAX_PIXELS	63
#define PAGEBUFFER_SIZE	16*10

enum led_layer_mode_t
{
	BLEND_OVERLAY 	= 0x01,
	BLEND_OVERWRITE = 0x02,
	BLEND_MASK	= 0x0f,
	ACTIVE_ALWAYS 	= 0x80,
};

uint8_t flatten_layer[] = { 0xE8, 0x24,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* C1-1 -> C1-16 */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* C2-1 -> C2-16 */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* C3-1 -> C3-16 */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* C4-1 -> C4-16 */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* C5-1 -> C5-16 */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* C6-1 -> C6-16 */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* C7-1 -> C7-16 */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* C8-1 -> C8-16 */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* C9-1 -> C9-16 */
};

uint8_t led_layers_mode[MAX_LAYERS] = {
	LedLayerModes_define
};
uint8_t led_layers_activation[MAX_LAYERS];
uint8_t led_layers[MAX_LAYERS][MAX_PIXELS];


inline uint8_t is_valid_layer(uint8_t lidx)
{
	return lidx < MAX_LAYERS;
}

inline uint8_t is_valid_pixel(uint8_t lidx, uint8_t pidx)
{
	return lidx < MAX_LAYERS && pidx < MAX_PIXELS;
}

inline void led_layer_activate(uint8_t lidx)
{
	if (is_valid_layer(lidx))
		led_layers_activation[lidx]++;
}

inline void led_layer_set_mode(uint8_t lidx, uint8_t mode)
{
	if (is_valid_layer(lidx))
		led_layers_mode[lidx] = mode;
}

inline void led_layer_deactivate(uint8_t lidx)
{
	if (is_valid_layer(lidx))
		led_layers_activation[lidx]--;
}

inline void led_layer_set_pixel(uint8_t lidx, uint8_t pidx, uint8_t value)
{
	if (!is_valid_pixel(lidx, pidx))
		return;

	led_layers[lidx][pidx] = value;
}

inline void led_pixels_clear(uint8_t* pixels, uint8_t value)
{
	memset(pixels, value, MAX_PIXELS);
}

inline void led_layer_clear(uint8_t lidx, uint8_t value)
{
	if (!is_valid_layer(lidx))
		return;

	led_pixels_clear(led_layers[lidx], value);
}

uint8_t* led_currrent_pagebuffer()
{
	return flatten_layer;
}

int led_pagebuffer_size()
{
	return sizeof(flatten_layer);
}

uint8_t led_layer_flatten_pixel(uint8_t pidx)
{
	uint8_t value = 0;

	for (uint8_t lidx = 0; lidx < MAX_LAYERS; ++lidx)
	{
		uint8_t px = led_layers[lidx][pidx];
		uint8_t mode = led_layers_mode[lidx];
		uint8_t is_active = (mode & ACTIVE_ALWAYS) || (led_layers_activation[lidx] != 0);

		if (is_active)
		{
			switch (mode & BLEND_MASK)
			{
			case BLEND_OVERLAY:
				value = value > px ? value : px;
				break;

			case BLEND_OVERWRITE:
				value = px;
				return value;
			}
		}
	}

	return value;
}

uint8_t led_layer_update()
{
	uint8_t* offset = flatten_layer + 2;
	uint8_t updated = 0x0;

	for (uint8_t pidx = 0; pidx < MAX_PIXELS; ++pidx)
	{
		uint8_t value = led_layer_flatten_pixel(pidx);

		uint8_t hi = (pidx & 0xf8) >> 3;
		uint8_t lw = (pidx & 0x07);
		uint8_t addr = (hi<<4) | lw;

		if (addr < 0 || addr >= PAGEBUFFER_SIZE)
		{
			print("ERROR ");
			printHex(pidx);
			print("=>");
			printHex(addr);
			print(NL);
			continue;
		}

		if (offset[addr] != value)
		{
			offset[addr] = value;
			updated = 0x1;
		}

	}

	return updated;
}

#define LEDLAYER_COMMAND_CLEARLAYER	0x00
#define LEDLAYER_COMMAND_SETPIXEL	0x01
#define LEDLAYER_COMMAND_NONE		0x09

void LED_layer_capability( uint8_t state, uint8_t stateType, uint8_t *args )
{
	uint8_t command = args[0];
	uint8_t layer   = args[1];
	uint8_t index   = args[2];
	uint8_t value   = args[3];

	switch(command)
	{
		case LEDLAYER_COMMAND_CLEARLAYER:
			//led_layer_clear(layer, value);
			break;
		case LEDLAYER_COMMAND_SETPIXEL:
			led_layer_set_pixel(layer, index, value);
			break;
		case LEDLAYER_COMMAND_NONE:
			break;
	}
}

void LED_press_capability( uint8_t state, uint8_t stateType, uint8_t *args )
{
	uint8_t index = args[0];

	if (stateType == 0 && state == 0x01)
	{
		led_layer_activate(1);
		led_layer_set_pixel(1, index, 255);
	}
	else if (stateType == 0 && state == 0x03)
	{
		led_layer_deactivate(1);
		led_layer_set_pixel(1, index, 0);
	}
}

void led_layer_setup()
{
	led_layer_clear(2, 255);
}
