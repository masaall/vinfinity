
#include "types.h"
#include "defs.h"
#include "memlayout.h"
#include "screen.h"
#include "x86.h"

uint8_t *BUFFER = P2V((uint8_t*)0xa0000);

uint8_t _sbuffers[2][SCREEN_SIZE];
uint8_t _sback = 0;

#define CURRENT (_sbuffers[_sback])
#define SWAP() (_sback = 1 - _sback)

#define PALETTE_MASK 0x3c6
#define PALETTE_READ 0x3c7
#define PALETTE_WRITE 0x3c8
#define PALETTE_DATA 0x3c9

void screen_swap(void){
	memmove(BUFFER, &CURRENT, SCREEN_SIZE);
	SWAP();
}

void screen_init(void){
	outb(PALETTE_MASK, 0xff);
	outb(PALETTE_WRITE, 0);

	for (uint8_t i = 0; i < 255; i++){
		outb(PALETTE_DATA, (((i >> 5) & 0x7) * (256 / 8)) / 4);
		outb(PALETTE_DATA, (((i >> 2) & 0x7) * (256 / 8)) / 4);
		outb(PALETTE_DATA, (((i >> 0) & 0x3) * (256 / 4)) / 4);
	}

	outb(PALETTE_DATA, 0x3f);
	outb(PALETTE_DATA, 0x3f);
	outb(PALETTE_DATA, 0x3f);
}
