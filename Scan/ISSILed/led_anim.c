#include <Lib/ScanLib.h>
#include <print.h>
#include <kll_defs.h>

#define MAX_LAYERS	3
#define MAX_PIXELS	63
#define MAX_ANIMATIONS	50
#define PAGEBUFFER_SIZE	16*10

enum LA_layer_mode_t
{
	LA_BLEND_TRANSPARENT 	= 0x01,
	LA_BLEND_OVERWRITE 	= 0x02
};

uint8_t LA_pagebuffer[] = { 0xE8, 0x24,
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


#define LA_EASE_LINEAR              0
#define LA_EASE_QUADRATIC_IN        1
#define LA_EASE_QUADRATIC_OUT       2
#define LA_EASE_QUADRATIC_INOUT     3
#define LA_EASE_CUBIC_IN            4
#define LA_EASE_CUBIC_OUT           5
#define LA_EASE_CUBIC_INOUT         6
#define LA_EASE_QUARTIC_IN          7
#define LA_EASE_QUARTIC_OUT         8
#define LA_EASE_QUARTIC_INOUT       9
#define LA_EASE_QUINTIC_IN          10
#define LA_EASE_QUINTIC_OUT         11
#define LA_EASE_QUINTIC_INOUT       12
#define LA_EASE_SINUSOIDAL_IN       13
#define LA_EASE_SINUSOIDAL_OUT      14
#define LA_EASE_SINUSOIDAL_INOUT    15
#define LA_EASE_EXPONENTIAL_IN      16
#define LA_EASE_EXPONENTIAL_OUT     17
#define LA_EASE_EXPONENTIAL_INOUT   18
#define LA_EASE_CIRCULAR_IN         19
#define LA_EASE_CIRCULAR_OUT        20
#define LA_EASE_CIRCULAR_INOUT      21
#define LA_EASE_ELASTIC_IN          22
#define LA_EASE_ELASTIC_OUT         23
#define LA_EASE_ELASTIC_INOUT       24
#define LA_EASE_BACK_IN             25
#define LA_EASE_BACK_OUT            26
#define LA_EASE_BACK_INOUT          27
#define LA_EASE_BOUNCE_IN           28
#define LA_EASE_BOUNCE_OUT          29
#define LA_EASE_BOUNCE_INOUT        30

#define LA_EASE_WIDTH		32
// data was generaged using TWEEN.js
uint8_t LA_ease_curve[][32] = {
{  0,  8, 16, 24, 32, 41, 49, 57, 65, 74, 82, 90, 98,106,115,123,131,139,148,156,164,172,180,189,197,205,213,222,230,238,246,255}, // 0 Linear.None
{  0,  0,  1,  2,  4,  6,  9, 13, 16, 21, 26, 32, 38, 44, 52, 59, 67, 76, 85, 95,106,117,128,140,152,165,179,193,208,223,238,255}, // 1 Quadratic.In
{  0, 16, 31, 46, 61, 75, 89,102,114,126,137,148,159,169,178,187,195,202,210,216,222,228,233,238,241,245,248,250,252,253,254,255}, // 2 Quadratic.Out
{  0,  0,  2,  4,  8, 13, 19, 26, 33, 42, 53, 64, 76, 89,104,119,135,150,165,178,190,201,212,221,228,235,241,246,250,252,254,255}, // 3 Quadratic.InOut
{  0,  0,  0,  0,  0,  1,  1,  2,  4,  6,  8, 11, 14, 18, 23, 28, 35, 42, 49, 58, 68, 79, 91,104,118,133,150,168,187,208,231,255}, // 4 Cubic.In
{  0, 23, 46, 67, 86,104,121,136,150,163,175,186,196,205,212,219,226,231,236,240,243,246,248,250,252,253,253,254,254,254,254,255}, // 5 Cubic.Out
{  0,  0,  0,  0,  2,  4,  7, 11, 17, 24, 34, 45, 59, 75, 93,115,139,161,179,195,209,220,230,237,243,247,250,252,254,254,254,255}, // 6 Cubic.InOut
{  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  2,  4,  5,  7, 10, 13, 18, 23, 28, 35, 44, 53, 64, 77, 91,107,126,146,169,195,223,255}, // 7 Quartic.In
{  0, 31, 59, 85,108,128,147,163,177,190,201,210,219,226,231,236,241,244,247,249,250,252,253,253,254,254,254,254,254,254,254,255}, // 8 Quartic.Out
{  0,  0,  0,  0,  0,  1,  2,  5,  9, 14, 22, 32, 45, 63, 84,111,143,170,191,209,222,232,240,245,249,252,253,254,254,254,254,255}, // 9 Quartic.InOut
{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  6,  9, 12, 16, 22, 28, 36, 45, 57, 70, 86,105,127,153,182,216,255}, // 10 Quintic.In
{  0, 38, 72,101,127,149,168,184,197,209,218,226,232,238,242,245,248,250,251,252,253,254,254,254,254,254,254,254,254,254,254,255}, // 11 Quintic.Out
{  0,  0,  0,  0,  0,  0,  1,  2,  4,  8, 14, 22, 35, 52, 76,108,146,178,202,219,232,240,246,250,252,253,254,254,254,254,254,255}, // 12 Quintic.InOut
{  0,  0,  1,  2,  5,  8, 11, 15, 20, 26, 32, 38, 45, 53, 61, 70, 79, 88, 98,109,120,131,142,154,166,178,191,203,216,229,242,254}, // 13 Sinusoidal.In
{  0, 12, 25, 38, 51, 63, 76, 88,100,112,123,134,145,156,166,175,184,193,201,209,216,222,228,234,239,243,246,249,252,253,254,255}, // 14 Sinusoidal.Out
{  0,  0,  2,  5, 10, 16, 22, 30, 39, 49, 60, 71, 83, 95,108,121,133,146,159,171,183,194,205,215,224,232,238,244,249,252,254,255}, // 15 Sinusoidal.InOut
{  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  2,  2,  3,  4,  5,  7,  8, 11, 13, 17, 21, 27, 34, 42, 53, 66, 83,104,130,163,203,255}, // 16 Exponential.In
{  0, 51, 91,124,150,171,188,201,212,220,227,233,237,241,243,246,247,249,250,251,252,252,253,253,253,254,254,254,254,254,254,255}, // 17 Exponential.Out
{  0,  0,  0,  0,  0,  1,  1,  2,  4,  6, 10, 17, 26, 41, 65,101,153,189,213,228,237,244,248,250,252,253,253,254,254,254,254,255}, // 18 Exponential.InOut
{  0,  0,  0,  1,  2,  3,  4,  6,  8, 10, 13, 16, 19, 23, 27, 31, 36, 41, 47, 53, 60, 67, 75, 84, 93,104,116,129,145,164,190,255}, // 19 Circular.In
{  0, 64, 90,109,125,138,150,161,170,179,187,194,201,207,213,218,223,227,231,235,238,241,244,246,248,250,251,252,253,254,254,255}, // 20 Circular.Out
{  0,  0,  1,  2,  4,  6,  9, 13, 18, 23, 30, 37, 46, 58, 72, 95,159,182,196,208,217,224,231,236,241,245,248,250,252,253,254,255}, // 21 Circular.InOut
{  0,254,254,254,  0,  0,  0,  1,  0,  0,254,252,251,250,251,253,  2,  7, 13, 17, 16,  9,249,228,206,188,186,209,  6, 86,178,255}, // 22 Elastic.In
{  0, 76,168,248, 45, 68, 66, 48, 26,  5,245,238,237,241,247,252,  1,  3,  4,  3,  2,  0,254,254,253,254,254,254,  0,  0,  0,255}, // 23 Elastic.Out
{  0,254,  0,  0,  0,254,253,253,  1,  6,  8,252,230,220,  3, 89,165,251, 34, 24,  2,246,248,253,  1,  1,  0,254,254,254,  0,255}, // 24 Elastic.InOut
{  0,254,253,251,249,246,243,240,237,235,232,231,229,229,229,231,234,238,243,250,  4, 15, 27, 42, 59, 79,101,126,153,184,218,254}, // 25 Back.In
{  0, 36, 70,101,128,153,175,195,212,227,239,250,  4, 11, 16, 20, 23, 25, 25, 25, 23, 22, 19, 17, 14, 11,  8,  5,  3,  1,  0,255}, // 26 Back.Out
{  0,253,250,245,240,235,232,229,229,233,240,252, 14, 37, 67,105,149,187,217,240,  2, 14, 21, 25, 25, 22, 19, 14,  9,  4,  1,255}, // 27 Back.InOut
{  0,  3,  3,  1, 10, 15, 15, 12,  4, 11, 30, 44, 54, 61, 63, 62, 56, 46, 33, 15, 12, 54, 92,126,156,182,204,222,236,246,252,255}, // 28 Bounce.In
{  0,  2,  8, 18, 32, 50, 72, 98,128,162,200,242,239,221,208,198,192,191,193,200,210,224,243,250,242,239,239,244,253,251,251,255}, // 29 Bounce.Out
{  0,  1,  5,  7,  2, 15, 27, 31, 28, 16,  6, 46, 78,102,118,126,128,136,152,176,208,248,238,226,223,227,239,252,247,249,253,255}, // 30 Bounce.InOut
};

typedef struct
{
	uint8_t blend_mode;
	uint8_t always_on;
	uint8_t refcount;
	uint8_t activated;
} LA_layer_t;

LA_layer_t LA_layer_info[MAX_LAYERS];
uint8_t LA_layer[MAX_LAYERS][MAX_PIXELS];

typedef struct 
{
	uint8_t  origin;
	uint8_t  type;
	uint16_t start;
	uint16_t duration;
	uint8_t  data[6];
} LA_animation_t;

LA_animation_t LA_animations[MAX_ANIMATIONS];

uint8_t LA_animation_count;
uint16_t LA_global_tick;

inline uint8_t LA_is_valid_layer(uint8_t lidx)
{
	return lidx < MAX_LAYERS;
}

inline uint8_t LA_is_valid_pixel(uint8_t lidx, uint8_t pidx)
{
	return lidx < MAX_LAYERS && pidx < MAX_PIXELS;
}

inline void LA_set_mode(uint8_t lidx, uint8_t mode)
{
	if (LA_is_valid_layer(lidx))
		LA_layer_info[lidx].blend_mode = mode;
}

inline void LA_ref(uint8_t lidx)
{
	if (LA_is_valid_layer(lidx))
		LA_layer_info[lidx].refcount ++;
}

inline void LA_unref(uint8_t lidx)
{
	if (LA_is_valid_layer(lidx))
		LA_layer_info[lidx].refcount --;
}

inline void LA_set_pixel(uint8_t lidx, uint8_t pidx, uint8_t value)
{
	if (!LA_is_valid_pixel(lidx, pidx))
		return;

	LA_layer[lidx][pidx] = value;
}

inline void LA_set_max_pixel(uint8_t lidx, uint8_t pidx, uint8_t value)
{
	if (!LA_is_valid_pixel(lidx, pidx))
		return;

	LA_layer[lidx][pidx] = LA_layer[lidx][pidx] > value
		? LA_layer[lidx][pidx]
		: value;
}

inline void LA_pixels_clear(uint8_t* pixels, uint8_t value)
{
	memset(pixels, value, MAX_PIXELS);
}

inline void LA_layer_clear(uint8_t lidx, uint8_t value)
{
	if (!LA_is_valid_layer(lidx))
		return;

	LA_pixels_clear(LA_layer[lidx], value);
}

inline uint8_t LA_flatten_pixel(uint8_t pidx)
{
	uint8_t value = 0;

	for (uint8_t lidx = 0; lidx < MAX_LAYERS; ++lidx)
	{
		if (LA_layer_info[lidx].activated)
		{
			uint8_t px = LA_layer[lidx][pidx];
			uint8_t blend = LA_layer_info[lidx].blend_mode;
			if (blend == LA_BLEND_TRANSPARENT && value < px)
			{
				value = px;
			}
			else if (blend == LA_BLEND_OVERWRITE)
			{
				return px;
			}
		}
	}

	return value;
}

LA_animation_t* LA_create_animation(uint8_t type, uint8_t origin, uint16_t duration)
{
	if (LA_animation_count >= MAX_ANIMATIONS)
		return 0x0;

	LA_animation_t* ani = &LA_animations[LA_animation_count++];

	ani->type = type;
	ani->origin = origin;
	ani->start = LA_global_tick;
	ani->duration = duration;

	ani->data[0] = 0;
	ani->data[1] = 0;
	ani->data[2] = 0;
	ani->data[3] = 0;
	ani->data[4] = 0;
	ani->data[5] = 0;

	return ani;
}

void LA_animate_0(LA_animation_t* ani, uint16_t tick)
{
#define ANIM0_WIDTH 5

	uint8_t alive = 0;
	for (uint8_t i=0; i<ANIM0_WIDTH; ++i)
	{
		int16_t p = tick*2 - i*100 + 1;
		if (p>255) p = 510 - p;
		uint8_t value = ((p<=0 || 255<p) ? 0 : p) / (i*i+1);
		
		LA_set_max_pixel(1, (ani->origin-i)%MAX_PIXELS, value);
		LA_set_max_pixel(1, (ani->origin+i)%MAX_PIXELS, value);

		if (value != 0)
			alive = 1;
	}
	if (alive == 0)
	{
		ani->duration = tick;
	}
}

uint8_t LA_ease(uint16_t tick, uint16_t dur, uint8_t ei)
{
	uint16_t idx = (tick * LA_EASE_WIDTH) / dur;
	return LA_ease_curve[ei][idx];
}

uint8_t LA_ease_reverse(uint16_t tick, uint16_t dur, uint8_t ei)
{
	uint16_t idx = ((dur - tick) * LA_EASE_WIDTH) / dur;
	return LA_ease_curve[ei][idx];
}

void LA_animate_1(LA_animation_t* ani, uint16_t tick)
{
	// 0 - ease function
	uint8_t v;
	if (ani->data[0] == 0)
	{
		v = (uint16_t) LA_ease(tick, ani->duration, ani->data[1]) * ani->data[2] / 255;
	}
	else
	{
		v = (uint16_t) LA_ease_reverse(tick, ani->duration, ani->data[1]) * ani->data[2] / 255;
	}
	LA_set_max_pixel(1, ani->origin, v);
}

void LA_animate_2(LA_animation_t* ani, uint16_t tick)
{
	if (ani->data[0] == 0)
		return;

	uint8_t last_spread = ani->data[4];

//print("SP ");

	uint8_t current_spread = LA_ease(tick, ani->duration, ani->data[1]);
	while (last_spread * ani->data[0] <= current_spread)
	{

//printHex(last_spread); print(","); printHex(last_spread*ani->data[0]); print(":"); printHex(current_spread);

		if (last_spread == 0)
		{
			LA_animation_t * ani0 = LA_create_animation(1, ani->origin, ani->data[2]);
			if (ani0 != 0x0)
			{
				ani0->data[0] = 1;		// decreasing
				ani0->data[1] = ani->data[3];
				ani0->data[2] = 255;
			}
		}
		else
		{
			LA_animation_t * ani0 = LA_create_animation(1, (ani->origin-last_spread) % MAX_PIXELS, ani->data[2] - last_spread * 10);
			if (ani0 != 0x0)
			{
				ani0->data[0] = 1;		// decreasing
				ani0->data[1] = ani->data[3];
				ani0->data[2] = 255 - last_spread * 20;
			}
			LA_animation_t * ani1 = LA_create_animation(1, (ani->origin+last_spread) % MAX_PIXELS, ani->data[2] - last_spread * 10);
			if (ani1 != 0x0)
			{
				ani1->data[0] = 1;		// decreasing
				ani1->data[1] = ani->data[3];
				ani1->data[2] = 255 - last_spread * 20;
			}
		}
		last_spread ++;
	}
	ani->data[4] = last_spread;

//print(NL);

}

typedef void (*LA_animation_fun_t)(LA_animation_t* ani, uint16_t tick);

LA_animation_fun_t LA_ani_funcs[] = {
	LA_animate_0,
	LA_animate_1,
	LA_animate_2
};

uint8_t LA_update_animation()
{
	if (LA_animation_count == 0)
	{
		static uint16_t lt = 0;
		if (lt == LA_global_tick)
		{
			LA_layer_info[1].refcount = 0;
			return 1;
		}
		else if (lt < LA_global_tick || LA_global_tick + 500 < lt)
		{
			lt = LA_global_tick + 500;
		}
		return 0;
	}

	LA_layer_info[1].refcount = 1;
	LA_layer_clear(1, 0);

	// animating
	for (int i=0; i<LA_animation_count; ++i)
	{
		LA_animation_t* ani = &LA_animations[i];
		if (ani->type < sizeof(LA_ani_funcs))
		{
			uint16_t tick = LA_global_tick - ani->start;
			(*LA_ani_funcs[ani->type])(ani, tick);

			if (tick == ani->duration)
			{
				ani->type = 0xff;
			}
		}
	}

	// compacting
	for (int i=0; i<LA_animation_count; ++i)
	{
		// if finished animation
		if (LA_animations[i].type == 0xff)
		{
			while(i < LA_animation_count)
			{
				LA_animation_count--;
				if (LA_animations[LA_animation_count].type != 0xff)
				{
					LA_animations[i] = LA_animations[LA_animation_count];
					break;
				}
			}
		}
	}

	return 1;
}

uint8_t* LA_get_pagebuffer()
{
	return LA_pagebuffer;
}

int LA_get_pagebuffer_size()
{
	return sizeof(LA_pagebuffer);
}

void LA_press_capability( uint8_t state, uint8_t stateType, uint8_t *args )
{
	uint8_t index = args[0];

	if (stateType == 0 && state == 0x01)
	{
#if 0
		LA_animation_t* ani = LA_create_animation(1, index, 100);
		if (ani != 0x0)
		{
			ani->data[0] = 100;			// pixel interval
			ani->data[1] = LA_EASE_SINUSOIDAL_OUT;	// pixel curve
		}
#else
		LA_animation_t* ani = LA_create_animation(2, index, 50);
		if (ani != 0x0)
		{
			ani->data[0] = 70;			// spread interval
			ani->data[1] = LA_EASE_QUINTIC_OUT;	// spreading curve
			ani->data[2] = 50;			// pixel interval
			ani->data[3] = LA_EASE_SINUSOIDAL_OUT;	// pixel curve
		}
#endif
	}
}

void LA_setup()
{
	LA_layer_info[0].blend_mode = LA_BLEND_TRANSPARENT;
	LA_layer_info[0].always_on  = 1;
	LA_layer_info[0].refcount   = 0;
	LA_layer_info[0].activated  = 0;

	LA_layer_info[1].blend_mode = LA_BLEND_OVERWRITE;
	LA_layer_info[1].always_on  = 0;
	LA_layer_info[1].refcount   = 0;
	LA_layer_info[1].activated  = 0;

	LA_layer_info[2].blend_mode = LA_BLEND_OVERWRITE;
	LA_layer_info[2].always_on  = 1;
	LA_layer_info[2].refcount   = 0;
	LA_layer_info[2].activated  = 0;

	LA_layer_clear(2, 255);
}


uint8_t LA_is_update_time_passed()
{
	static uint32_t last_update_10ms = 0;

	uint32_t now10 = millis() / 10;
	if (last_update_10ms != now10)
	{
		last_update_10ms = now10;
		return 1;
	}
	return 0;
}

uint8_t LA_update_pixels()
{
	uint8_t updated = 0;

	for (uint8_t lidx = 1; lidx < MAX_LAYERS; ++lidx)
	{
		uint8_t activated =
			LA_layer_info[lidx].always_on ||
			(LA_layer_info[lidx].refcount > 0);

		if (activated)
		{
			uint8_t blend = LA_layer_info[lidx].blend_mode;
			uint8_t* psrc = &LA_layer[lidx][0];
			uint8_t* pdst = LA_pagebuffer + 2;

			if (blend == LA_BLEND_TRANSPARENT)
			{
				for (uint8_t i=0, j = 0; i<MAX_PIXELS; i++)
				{
					if (*pdst < *psrc)
					{
						*pdst = *psrc;
					}

					pdst ++;
					psrc ++;

					if (++j == 8)
					{
						pdst += 8;
						j = 0;
					}
				}
			}
			else if (blend == LA_BLEND_OVERWRITE)
			{
				for (uint8_t i=0, j = 0; i<MAX_PIXELS; i++)
				{
					*pdst++ = *psrc++;

					if (++j == 8)
					{
						pdst += 8;
						j = 0;
					}
				}
				return updated;
			}
		}
	}

	return updated;
}

uint8_t LA_update()
{
	if (!LA_is_update_time_passed())
		return 0;

	uint8_t u1 = LA_update_animation();
	uint8_t u2 = LA_update_pixels();

	LA_global_tick ++;

	return u1 || u2;
}


#if 0
#endif
//uint8_t* LA_get_pagebuffer() { return 0; }
//uint8_t LA_get_pagebuffer_size() { return 0; }
//void LA_layer_capability( uint8_t state, uint8_t stateType, uint8_t *args ) {}
//void LA_press_capability( uint8_t state, uint8_t stateType, uint8_t *args ) {}
//void LA_setup() {}
//uint8_t LA_update() { return 0; }

