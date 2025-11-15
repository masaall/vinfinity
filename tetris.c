
#include "types.h"
#include "defs.h"
#include "screen.h"
#include "font.h"

void render_menu(void){

	const char *play = "PRESS ENTER TO PLAY";

	font_str_doubled(play, 0, 0, COLOR(6, 6, 2));
}

void tetris(void){
	screen_init();
	render_menu();
	screen_swap();
}
