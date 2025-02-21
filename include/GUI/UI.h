#ifndef UI_H
#define UI_H

#include <stdint.h>

typedef struct {
        uint8_t selected : 1,
                hover : 1;
        unsigned x,y, scl_x, scl_y;
	int rel_x;
}UI_Slider;

typedef struct {
	uint8_t pushed : 1,
		hover : 1;
	unsigned x, y, width, height;
	char (*press)(void);
}UI_Button;

UI_Slider init_UI_Slider(float x, float y, float s_x, float s_y) {
	return (UI_Slider){0, 0, x, y, s_x, s_y, 0};
}

UI_Button init_UI_Button(float x, float y, float w, float h) {
	return (UI_Button){0, 0, x, y, w, h};
}

#endif
