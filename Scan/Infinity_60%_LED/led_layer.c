#include <Lib/ScanLib.h>
#include <print.h>
#include <kll_defs.h>

#define MAX_LAYERS	3
#define MAX_PIXELS	63
#define MAX_ANIMATIONS	10
#define PAGEBUFFER_SIZE	16*10

#define LEDLAYER_COMMAND_CLEARLAYER	0x00
#define LEDLAYER_COMMAND_SETPIXEL	0x01
#define LEDLAYER_COMMAND_NONE		0x09

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

typedef struct
{
	uint8_t blend_mode;
	uint8_t always_on;
	uint8_t activation;
	uint8_t activated;
} led_layer_t;

led_layer_t led_layers_info[MAX_LAYERS];
uint8_t led_layers[MAX_LAYERS][MAX_PIXELS];

typedef struct 
{
	uint8_t origin;
	uint8_t type;
	uint8_t tick;
	uint8_t duration;
	uint8_t data[4];
} led_animation_t;

led_animation_t led_animations[MAX_ANIMATIONS];

uint8_t led_animation_count;

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
		led_layers_info[lidx].activation ++;
}

inline void led_layer_set_mode(uint8_t lidx, uint8_t mode)
{
	if (is_valid_layer(lidx))
		led_layers_info[lidx].blend_mode = mode;
}

inline void led_layer_deactivate(uint8_t lidx)
{
	if (is_valid_layer(lidx))
		led_layers_info[lidx].activation --;
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

inline uint8_t led_layer_flatten_pixel(uint8_t pidx)
{
	uint8_t value = 0;

	for (uint8_t lidx = 0; lidx < MAX_LAYERS; ++lidx)
	{
		if (led_layers_info[lidx].activated)
		{
			uint8_t px = led_layers[lidx][pidx];
			uint8_t blend = led_layers_info[lidx].blend_mode;
			if (blend == BLEND_OVERLAY && value < px)
			{
				value = px;
			}
			else if (blend == BLEND_OVERWRITE)
			{
				return px;
			}
		}
	}

	return value;
}

led_animation_t* create_animation()
{
	if (led_animation_count >= MAX_ANIMATIONS)
		return 0x0;
	return &led_animations[led_animation_count++];
}

void animate_0(led_animation_t* ani)
{
	uint8_t diff = ani->tick/5;
	uint8_t start = ani->origin - diff;
	uint8_t end = ani->origin + diff;

	for (uint8_t i=start; i<=end; ++i)
	{
		led_layer_set_pixel(1, i%MAX_PIXELS, 255);
	}
}

uint8_t tick;
void animation_update()
{
	led_layer_clear(1, 0);

	tick ++;
	if (tick > 20)
		tick = 0;

	// animating
	for (int i=0; i<led_animation_count; ++i)
	{
		led_animation_t* ani = &led_animations[i];
		switch (ani->type)
		{
			case 0:
				animate_0(ani);
				break;
		}

		if (tick == 0) {
			ani->duration--;
			ani->tick++;
		}
	}

	// compacting
	for (int i=0; i<led_animation_count; ++i)
	{
	/*
		print("l:");
		printHex(i);
		print(" d=");
		printHex(led_animations[i].duration);
		*/
		// if finished animation
		if (led_animations[i].duration == 0)
		{
		//print(" zero ");
			while(i < led_animation_count)
			{
				led_animation_count--;
				// if running animation
				if (led_animations[led_animation_count].duration != 0)
				{
		//print("=>");
		//printHex(led_animation_count);
		//print(NL);
					led_animations[i] = led_animations[led_animation_count];
					break;
				}
			}
		}
	}

	if (led_animation_count != 0)
	{
		print(" anicnt = ");
		printHex(led_animation_count);
		print(NL);
	}
}

uint8_t* led_current_pagebuffer()
{
	return flatten_layer;
}

int led_pagebuffer_size()
{
	return sizeof(flatten_layer);
}

void LED_layer_capability( uint8_t state, uint8_t stateType, uint8_t *args )
{
//	uint8_t command = args[0];
//	uint8_t layer   = args[1];
//	uint8_t index   = args[2];
//	uint8_t value   = args[3];

/*
	switch(command)
	{
		case LEDLAYER_COMMAND_CLEARLAYER:
			//led_layer_clear(layer, value);
			break;
		case LEDLAYER_COMMAND_SETPIXEL:
			//led_layer_set_pixel(layer, index, value);
			break;
		case LEDLAYER_COMMAND_NONE:
			break;
	}
*/
}

void LED_press_capability( uint8_t state, uint8_t stateType, uint8_t *args )
{
	uint8_t index = args[0];

	if (stateType == 0 && state == 0x01)
	{
		//led_layer_activate(1);
		//led_layer_set_pixel(1, index, 255);
		led_animation_t* ani = create_animation();
		if (ani != 0x0)
		{
			ani->origin = index;
			ani->type = 0;
			ani->tick = 0;
			ani->duration = 50;
			ani->data[0] = 0;
			ani->data[1] = 0;
			ani->data[2] = 0;
			ani->data[3] = 0;
		}
	}
//	else if (stateType == 0 && state == 0x03)
//	{
//		led_layer_deactivate(1);
//		led_layer_set_pixel(1, index, 0);
//	}
}

void led_layer_setup()
{
	led_layers_info[0].blend_mode = BLEND_OVERLAY;
	led_layers_info[0].always_on  = 1;
	led_layers_info[0].activation = 0;
	led_layers_info[0].activated  = 0;

	led_layers_info[1].blend_mode = BLEND_OVERWRITE;
	led_layers_info[1].always_on  = 1;
	led_layers_info[1].activation = 1;
	led_layers_info[1].activated  = 0;

	led_layers_info[2].blend_mode = BLEND_OVERLAY;
	led_layers_info[2].always_on  = 1;
	led_layers_info[2].activation = 0;
	led_layers_info[2].activated  = 0;

	led_layer_clear(2, 255);
}


uint8_t led_layer_update()
{
	animation_update();

	for (uint8_t lidx = 0; lidx < MAX_LAYERS; lidx++)
	{
		led_layers_info[lidx].activated =
			led_layers_info[lidx].always_on || (led_layers_info[lidx].activation > 0);
	}

	uint8_t* offset = flatten_layer + 2;
	uint8_t updated = 0x0;

	for (uint8_t pidx = 0; pidx < MAX_PIXELS; ++pidx)
	{
		uint8_t value = led_layer_flatten_pixel(pidx);

		uint8_t hi = (pidx & 0xf8) >> 3;
		uint8_t lw = (pidx & 0x07);
		uint8_t addr = (hi<<4) | lw;

		if (offset[addr] != value)
		{
			offset[addr] = value;
			updated = 0x1;
		}

	}

	return updated;
}


#if 0
#endif
//uint8_t* led_current_pagebuffer() { return 0; }
//uint8_t led_pagebuffer_size() { return 0; }
//void LED_layer_capability( uint8_t state, uint8_t stateType, uint8_t *args ) {}
//void LED_press_capability( uint8_t state, uint8_t stateType, uint8_t *args ) {}
//void led_layer_setup() {}
//uint8_t led_layer_update() { return 0; }

