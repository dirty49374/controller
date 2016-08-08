#include <Lib/ScanLib.h>
#include <print.h>
#include <kll_defs.h>

#define LA_MAX_LAYERS		3
#define LA_MAX_ANIMATIONS	50
#define	LA_SCREEN_WIDTH		LAScreenWidth_define
#define	LA_SCREEN_HEIGHT	LAScreenHeight_define
#define LA_SCREEN_PIXELS	(LA_SCREEN_WIDTH * LA_SCREEN_HEIGHT)

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
#define LA_EASE_MAX		    31

#define LA_EASE_ELEMCOUNT	    32


enum LA_layer_kind_t
{
	LA_LAYER_GUIDE,
	LA_LAYER_ANIMATION,
	LA_LAYER_BACKGROUND
};

enum LA_layer_mode_t
{
	LA_BLEND_TRANSPARENT 	= 0x01,
	LA_BLEND_OVERWRITE 	= 0x02
};

typedef struct
{
	uint8_t blend_mode;
	uint8_t always_on;
	uint8_t refcount;
	uint8_t activated;
} LA_layer_info_t;

typedef union
{
	struct
	{
		uint8_t x;
		uint8_t y;
	};
	uint16_t coord;
} LA_coord_t;

typedef struct 
{
	LA_coord_t 	origin;
	uint16_t 	start;
	uint16_t 	duration;
	uint8_t  	type;
	uint8_t  	layer;
	uint8_t  	data[6];
} LA_animation_t;

const uint8_t LA_screen_to_pagebuffer_map[LAScreenHeight_define][LAScreenWidth_define] = {
LAScreenToPageBufferMap_define
};

const uint8_t LA_ease_curve[][32] = {
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

const int8_t LA_8dir[] = {
	0,-1,
	1,-1,
	1,0,
	1,1,
	0,1,
	-1,1,
	-1,0,
	-1,-1
};

uint8_t LA_pagebuffer[] = {
	0xE8, 0x24,
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

// data was generaged using TWEEN.js
struct
{
	uint8_t animation_count;
	uint8_t press_animation_type;
	uint16_t tick;
	LA_coord_t last_coord;
} LA_global;

uint8_t 		LA_layer[LA_MAX_LAYERS][LA_SCREEN_HEIGHT][LA_SCREEN_WIDTH];

LA_layer_info_t 	LA_layer_info[LA_MAX_LAYERS];

uint8_t		LA_layer_merged[LA_SCREEN_PIXELS];

LA_animation_t 	LA_animations[LA_MAX_ANIMATIONS];

inline LA_coord_t LA_coord(uint8_t x, uint8_t y)
{
	LA_coord_t c;
	c.x = x;
	c.y = y;
	return c;
}

inline uint8_t LA_is_valid_layer(uint8_t lidx)
{
	return lidx < LA_MAX_LAYERS;
}

inline uint8_t LA_is_valid_pixel(uint8_t lidx, LA_coord_t c)
{
	return lidx < LA_MAX_LAYERS && c.x < LA_SCREEN_WIDTH && c.y < LA_SCREEN_HEIGHT;
}

inline void LA_set_mode(uint8_t lidx, uint8_t mode)
{
	if (LA_is_valid_layer(lidx))
		LA_layer_info[lidx].blend_mode = mode;
}

inline void LA_layer_ref(uint8_t lidx)
{
	if (LA_is_valid_layer(lidx))
		LA_layer_info[lidx].refcount ++;
}

inline void LA_layer_unref(uint8_t lidx)
{
	if (LA_is_valid_layer(lidx))
		LA_layer_info[lidx].refcount --;
}

inline void LA_layer_set_pixel(uint8_t lidx, LA_coord_t c, uint8_t value)
{
	if (!LA_is_valid_pixel(lidx, c))
		return;

	LA_layer[lidx][c.y][c.x] = value;
}

inline void LA_set_max_pixel(uint8_t lidx, LA_coord_t c, uint8_t value)
{
	if (!LA_is_valid_pixel(lidx, c))
		return;

	uint8_t o = LA_layer[lidx][c.y][c.x];

	LA_layer[lidx][c.y][c.x] = o > value ? o : value;
}

inline void LA_pixels_clear(uint8_t* pixels, uint8_t value)
{
	memset(pixels, value, LA_SCREEN_PIXELS);
}

inline void LA_layer_clear(uint8_t lidx, uint8_t value)
{
	if (!LA_is_valid_layer(lidx))
		return;

	LA_pixels_clear(&LA_layer[lidx][0][0], value);
}

LA_animation_t* LA_create_animation(uint8_t type, uint8_t layer, LA_coord_t c, uint16_t duration)
{
	if (LA_global.animation_count >= LA_MAX_ANIMATIONS)
		return 0x0;

	LA_animation_t* ani = &LA_animations[LA_global.animation_count++];

	ani->origin   = c;
	ani->start    = LA_global.tick;
	ani->duration = duration;
	ani->type     = type;
	ani->layer    = layer;

	ani->data[0]  = 0;
	ani->data[1]  = 0;
	ani->data[2]  = 0;
	ani->data[3]  = 0;
	ani->data[4]  = 0;
	ani->data[5]  = 0;

	LA_layer_ref(layer);

	return ani;
}

uint8_t LA_ease(uint16_t tick, uint16_t dur, uint8_t ei)
{
	if (dur == 0)
		return 0;

	if (ei >= LA_EASE_MAX)
		return 0;

	uint16_t idx = (tick * LA_EASE_ELEMCOUNT) / dur;

	if (idx >= LA_EASE_ELEMCOUNT)
		return 0;

	return LA_ease_curve[ei][idx];
}

uint8_t LA_ease_reverse(uint16_t tick, uint16_t dur, uint8_t ei)
{
	if (dur == 0)
		return 0;

	if (ei >= LA_EASE_MAX)
		return 0;

	uint16_t idx = ((dur - tick) * LA_EASE_ELEMCOUNT) / dur;

	if (idx >= LA_EASE_ELEMCOUNT)
		return 0;

	return LA_ease_curve[ei][idx];
}

inline int8_t LA_dir(uint8_t a, uint8_t b)
{
	if (a < b)
		return 1;
	if (a > b)
		return -1;
	return 0;
}

inline LA_coord_t LA_coord_move(LA_coord_t c, int8_t dx, int8_t dy)
{
	c.x += dx;
	c.y += dy;

	return c;
}


inline int8_t LA_is_valid_coord(LA_coord_t c)
{
	return c.x < LA_SCREEN_WIDTH && c.y < LA_SCREEN_HEIGHT && LA_screen_to_pagebuffer_map[c.y][c.x] != 0xff;
}

inline int8_t LA_is_valid_coord_to_move(LA_coord_t c, int8_t dx, int8_t dy)
{
	c.x += dx;
	c.y += dy;

	return LA_is_valid_coord(c);
}

void LA_animate_0(LA_animation_t* ani, uint16_t tick)
{

#define ANI0_DESCENDING(x) 	x->data[0]
#define ANI0_EASE_TYPE(x) 	x->data[1]
#define ANI0_MAX(x) 		x->data[2]
	// 0 - ease function
	uint8_t v;
	if (ANI0_DESCENDING(ani) == 0)
		v = (uint16_t) LA_ease(tick, ani->duration, ANI0_EASE_TYPE(ani)) * ANI0_MAX(ani) / 255;
	else
		v = (uint16_t) LA_ease_reverse(tick, ani->duration, ANI0_EASE_TYPE(ani)) * ANI0_MAX(ani) / 255;

	LA_set_max_pixel(ani->layer, ani->origin, v);
}

void LA_animate_1(LA_animation_t* ani, uint16_t tick)
{
#define ANI1_DX(x) 		x->data[0]
#define ANI1_DY(x) 		x->data[1]
#define ANI1_MOVEINTERVAL(x) 	x->data[2]
#define ANI1_MOVECNT(x) 	x->data[3]

	int8_t dx = ANI1_DX(ani);
	int8_t dy = ANI1_DY(ani);

	if (tick >= ANI1_MOVEINTERVAL(ani) * ANI1_MOVECNT(ani))
	{
		if (LA_is_valid_coord_to_move(ani->origin, dx, dy))
		{
			ani->origin = LA_coord_move(ani->origin, dx, dy);
		}
		else if (LA_is_valid_coord_to_move(ani->origin, -dx, dy))
		{
			ANI1_DX(ani) = -dx;
			ani->origin = LA_coord_move(ani->origin, -dx, dy);
		}
		else if (LA_is_valid_coord_to_move(ani->origin, dx, -dy))
		{
			ANI1_DY(ani) = -dy;
			ani->origin = LA_coord_move(ani->origin, dx, -dy);
		}
		else if (LA_is_valid_coord_to_move(ani->origin, -dx, -dy))
		{
			ANI1_DX(ani) = -dx;
			ANI1_DY(ani) = -dy;
			ani->origin = LA_coord_move(ani->origin, -dx, -dy);
		}

		ANI1_MOVECNT(ani)++;
	}

	uint8_t v = (uint16_t) LA_ease_reverse(tick, ani->duration, LA_EASE_SINUSOIDAL_INOUT);

	LA_set_max_pixel(ani->layer, ani->origin, v);
}

void LA_animate_2(LA_animation_t* ani, uint16_t tick)
{
	uint8_t v;
	if (ani->data[0] == 0)
		v = (uint16_t) LA_ease(tick, ani->duration, LA_EASE_SINUSOIDAL_IN);
	else if (ani->data[0] == 1)
		v = 255;
	else if (ani->data[0] == 2)
		v = (uint16_t) LA_ease_reverse(tick, ani->duration, LA_EASE_SINUSOIDAL_IN);
	else
		v = 0;

	LA_layer_clear(LA_LAYER_BACKGROUND, v);

	if (tick == ani->duration - 1)
	{
		ani->data[0] = ani->data[0] >= 3 ? 0 : ani->data[0] + 1;
		ani->start = LA_global.tick;
		ani->duration = 200;
	}
}

typedef void (*LA_animation_fun_t)(LA_animation_t* ani, uint16_t tick);

LA_animation_fun_t LA_animation_funcs[] = {
	LA_animate_0,
	LA_animate_1,
	LA_animate_2
};

void LA_create_animation_pixel(uint8_t layer, LA_coord_t coord, uint16_t duration, uint8_t ease_type, uint8_t descending, uint8_t max)
{
	LA_animation_t* ani = LA_create_animation(0, layer, coord, duration);
	if (ani != 0x0)
	{
		ANI0_DESCENDING(ani) = descending;
		ANI0_EASE_TYPE(ani) = ease_type;
		ANI0_MAX(ani) = max;
	}
}

void LA_create_animation_drawline(uint8_t layer, LA_coord_t c0, LA_coord_t c1, uint16_t duration)
{
	LA_coord_t c = c0;

	int8_t dx = LA_dir(c0.x, c1.x);
	int8_t dy = LA_dir(c0.y, c1.y);

	uint8_t cnt = 0;
	do
	{
		LA_create_animation_pixel(layer, c, duration, LA_EASE_QUARTIC_INOUT, 1, 255);

		if (c.x != c1.x) c.x += dx;
		else if (c.y != c1.y) c.y += dy;

		if (cnt++ > 40) return;
	}
	while (c.x != c1.x || c.y != c1.y);
}

void LA_create_animation_a_moving_pixel(uint8_t layer, LA_coord_t coord, uint16_t duration, uint8_t dir)
{
	LA_animation_t* ani = LA_create_animation(1, layer, coord, duration);
	if (ani != 0x0)
	{
		ANI1_DX(ani) = LA_8dir[dir*2];
		ANI1_DY(ani) = LA_8dir[dir*2 + 1];
		ANI1_MOVEINTERVAL(ani) = duration / 10;
		ANI1_MOVECNT(ani) = 0;
	}
}

void LA_create_animation_moving_pixels(uint8_t layer, LA_coord_t coord, uint16_t duration)
{
	uint8_t di = micros() & 7;

	LA_create_animation_a_moving_pixel(layer, coord, duration, di);

	di += 2; di &= 7;

	LA_create_animation_a_moving_pixel(layer, coord, duration, di);

	di += 2; di &= 7;

	LA_create_animation_a_moving_pixel(layer, coord, duration, di);
}

void LA_create_animation_background(uint16_t duration)
{
	LA_create_animation(2, LA_LAYER_BACKGROUND, LA_coord(0, 0), duration);
}

static uint8_t last_updated = 0;
uint8_t LA_update_animations()
{
	uint8_t animated = LA_global.animation_count != 0 ? 1 : 0;

	// clear
	LA_layer_clear(LA_LAYER_ANIMATION, 0);

	// animating
	for (uint8_t i=0; i<LA_global.animation_count; ++i)
	{
		LA_animation_t* ani = &LA_animations[i];
		if (ani->type < sizeof(LA_animation_funcs)/sizeof(LA_animation_fun_t))
		{
			LA_animation_fun_t fun = LA_animation_funcs[ani->type];
			uint16_t tick = LA_global.tick - ani->start;

			(*fun)(ani, tick);

			if (tick == ani->duration)
			{
				ani->type = 0xff;
				LA_layer_unref(ani->layer);
			}
		}
		else
		{
			print("invalid animation fun type=");
			printHex(ani->type);
			print(NL);
		}
	}

	// compacting
	for (uint8_t i=0; i<LA_global.animation_count; ++i)
	{
		// if finished animation
		if (LA_animations[i].type == 0xff)
		{
			while(i < LA_global.animation_count)
			{
				LA_global.animation_count--;
				if (LA_animations[LA_global.animation_count].type != 0xff)
				{
					LA_animations[i] = LA_animations[LA_global.animation_count];
					break;
				}
			}
		}
	}

	uint8_t updated = last_updated || animated;

	last_updated = animated;

	return updated;
}

uint8_t* LA_get_pagebuffer()
{
	return LA_pagebuffer;
}

int LA_get_pagebuffer_size()
{
	return sizeof(LA_pagebuffer);
}

void LA_pressAnimation_capability( uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("LA_pressAnimation_capability(type)");
		return;
	}

	LA_global.press_animation_type = args[0] <= 2 ? args[0] : 0;
}

void LA_press_capability( uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("LA_press_capability(y,x)");
		return;
	}

	uint8_t y = args[0];
	uint8_t x = args[1];
	LA_coord_t coord = LA_coord(x, y);

	// press
	if (stateType == 0 && state == 0x01)
	{
		//print("Pressed ("); printInt16(coord.x); print(","); printInt16(coord.y); print(")" NL);

		switch (LA_global.press_animation_type)
		{
			case 0:
				LA_create_animation_pixel(LA_LAYER_ANIMATION, coord, 100, LA_EASE_EXPONENTIAL_INOUT, 1, 255);
				break;

			case 1:
				LA_create_animation_drawline(LA_LAYER_ANIMATION, LA_global.last_coord, coord, 50);
				break;

			case 2:
				LA_create_animation_moving_pixels(LA_LAYER_ANIMATION, coord, 50);
				break;

			default:
				break;
		}

		LA_global.last_coord = coord;
	}
}

void LA_layer_set_bitmap(uint8_t layer, uint8_t* bitmap, uint8_t value)
{
	for (uint8_t y=0; y<LA_SCREEN_HEIGHT; ++y)
	{
		uint8_t mask = 0x80;
		for (uint8_t x=0; x<LA_SCREEN_WIDTH; ++x)
		{
			if (mask == 0)
			{
				mask = 0x80;
				bitmap++;
			}

			if ((*bitmap & mask) != 0)
			{
				LA_layer_set_pixel(layer, LA_coord(x, y), value);
			}

			mask = mask >> 1;
		}

		bitmap++;
	}
}

void LA_ledGuide_capability( uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("LA_ledGuide_capability(type)");
		return;
	}

	
	// press
	if (stateType == 0 && state == 0x01)
	{
		LA_layer_set_bitmap(LA_LAYER_GUIDE, args, 255);
		last_updated = 1;
	}
	// release
	else if (stateType == 0 && state == 0x03)
	{
		LA_layer_clear(LA_LAYER_GUIDE, 0);
		last_updated = 1;
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

	LA_create_animation_background(200);

	LA_layer_clear(LA_LAYER_BACKGROUND, 255);
}

inline uint8_t LA_is_layer_active(uint8_t lidx)
{
	return LA_layer_info[lidx].always_on || LA_layer_info[lidx].refcount > 0;
}

uint8_t LA_is_update_time_passed()
{
	static uint32_t last_update_10ms = 0;

	uint32_t now10 = millis() / 10;
	if (last_update_10ms != now10)
	{
//print("UP"NL);
		last_update_10ms = now10;
		return 1;
	}
	return 0;
}

void LA_clear_merged_layer()
{
	memset(LA_layer_merged, 0, sizeof(LA_layer_merged));
}

void LA_merge_layers()
{
	LA_clear_merged_layer();

	for (uint8_t lidx = 0; lidx < LA_MAX_LAYERS; ++lidx)
	{
		if (LA_is_layer_active(lidx))
		{
			uint8_t* psrc = &LA_layer[lidx][0][0];
			uint8_t* pdst = LA_layer_merged;

			uint8_t blend = LA_layer_info[lidx].blend_mode;
			if (blend == LA_BLEND_TRANSPARENT)
			{
				for (uint8_t i=0; i<LA_SCREEN_PIXELS; i++)
				{
					if (*pdst < *psrc)
						*pdst = *psrc;
					pdst++;
					psrc++;
				}
			}
			else if (blend == LA_BLEND_OVERWRITE)
			{
				for (uint8_t i=0; i<LA_SCREEN_PIXELS; i++)
				{
					if (*pdst < *psrc)
						*pdst = *psrc;
					pdst++;
					psrc++;
				}
				break;
			}
		}
	}
}

void LA_clear_pagebuffer()
{
	memset(LA_pagebuffer+2, 0, sizeof(LA_pagebuffer) - 2);

	LA_pagebuffer[0] = 0xE8;
	LA_pagebuffer[1] = 0x24;
}

void LA_update_pagebuffer()
{
	LA_clear_pagebuffer();

	uint8_t* screen_to_pagebuffer_map = (uint8_t*)LA_screen_to_pagebuffer_map;
	uint8_t* pagebuffer = LA_pagebuffer + 2;

	for (uint8_t i=0; i<LA_SCREEN_PIXELS; ++i)
	{
		uint8_t pbi = screen_to_pagebuffer_map[i];
		uint8_t value = LA_layer_merged[i];

		pagebuffer[pbi] = pagebuffer[pbi] > value
			? pagebuffer[pbi]
			: value;
	}
}

uint8_t LA_update()
{
	if (!LA_is_update_time_passed())
		return 0;

	uint8_t updated = LA_update_animations();
	if (updated)
	{
		LA_merge_layers();
		LA_update_pagebuffer();
	}

	LA_global.tick ++;

	LA_pagebuffer[0] = 0xE8;
	LA_pagebuffer[1] = 0x24;

	return updated;
}


#if 0
#endif
//uint8_t* LA_get_pagebuffer() { return 0; }
//uint8_t LA_get_pagebuffer_size() { return 0; }
//void LA_layer_capability( uint8_t state, uint8_t stateType, uint8_t *args ) {}
//void LA_press_capability( uint8_t state, uint8_t stateType, uint8_t *args ) {}
//void LA_setup() {}
//uint8_t LA_update() { return 0; }

