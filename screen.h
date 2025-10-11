
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define SCREEN_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)

#define COLOR(_r, _g, _b)(uint8_t)(\
	(((_r) & 0x7) << 5)	|\
	(((_g) & 0x7) << 2) |\
	(((_b) & 0x3) << 0))

extern uint8_t _sbuffers[2][SCREEN_SIZE];
extern uint8_t _sback;

#define screen_set(_p, _x, _y)\
	(_sbuffers[_sback][((_y) * SCREEN_WIDTH + (_x))] = (_p))
