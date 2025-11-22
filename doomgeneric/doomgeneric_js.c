#include "doomgeneric.h"
#include "doomkeys.h"
#include <ctype.h>
#include <string.h>
#include <emscripten.h>

uint32_t startTime;

uint32_t getTime() {
    return EM_ASM_INT({
        return Date.now();
    });
}

void DG_Init() {
    startTime = getTime();
}

void DG_SleepMs(uint32_t ms) {}

uint32_t DG_GetTicksMs() {
    uint32_t newTime = getTime();
    return newTime - startTime;
}

void DG_DrawFrame() {
    EM_ASM({
        DGJS_DrawFrame($0, $1, $2);
    }, DG_ScreenBuffer, DOOMGENERIC_RESX, DOOMGENERIC_RESY);
}

int DG_GetKey(int* pressed, unsigned char* key) {
    // borrowed from DoomPDF
    int key_data = EM_ASM_INT({
        let key_data = DGJS_GetKey();
        let key = key_data[0];
        let pressed = key_data[1];
        return (pressed << 8) | key;
    });

    if (key_data == 0)
        return 0;

    *pressed = key_data >> 8;
    *key = key_data & 0xFF;
    return 1;
}

// converts JS keycodes to Doom keys
int toDoomKey(int keycode) {
    if (keycode == 38)
        return KEY_UPARROW;
    if (keycode == 40)
        return KEY_DOWNARROW;
    if (keycode == 37)
        return KEY_LEFTARROW;
    if (keycode == 39)
        return KEY_RIGHTARROW;
    if (keycode == 27)
        return KEY_ESCAPE;
    if (keycode == 13)
        return KEY_ENTER;
    if (keycode == 32)
        return KEY_USE;
    if (keycode == 17)
        return KEY_FIRE;
    if (keycode == 9)
        return KEY_TAB;
    if (keycode == 16)
        return KEY_RSHIFT;

    return tolower(keycode);
}

void DG_SetWindowTitle(const char * title) {
    EM_ASM({
        DGJS_SetTitle($0, $1);
    }, title, strlen(title));
}

int main(int argc, char **argv) {
    return 0;
}