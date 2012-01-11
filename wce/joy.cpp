#include "PocketGnuboy.h"
#include "wce.h"

extern "C" {

#include "defs.h"
#include "fb.h"
#include "hw.h"

void pad_set(byte k, int st);

/*
 * process hardware keys
 */

static int joy_code[JOY_MAX] = {
        0xC4, 0xC3, 0xC1, 0xC2, VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, 0xC5
};

static int turboa = 0;
static int turbob = 0;
static int turbosw_a = 0;
static int turbosw_b = 0;
static int vpadstate = VPAD_STATE_NONE;

void joy_init()
{
        ImmDisableIME(0);
        EnableHardwareKeyboard(FALSE);
        EnableHardwareKeyboard(TRUE);

        AllKeys(true);
}

void joy_close()
{
        AllKeys(false);
}
bool IsKeyPressed(int key)
{
	return GetAsyncKeyState(key) < 0;
}
void joy_process()
{
        BOOL push;
        // A
        if (joy_code[JOY_INDEX_A])
                push = (vpadstate & VPAD_STATE_A) || IsKeyPressed(joy_code[JOY_INDEX_A]);
        else
                push = (vpadstate & VPAD_STATE_A);
        if (turboa) {
                if (push) {
                        pad_set(PAD_A, turbosw_a);
                        turbosw_a = !turbosw_a;
                }
                else
                        pad_set(PAD_A, 0);
        }
        else pad_set(PAD_A, push);

        // B
        if (joy_code[JOY_INDEX_B])
                push = (vpadstate & VPAD_STATE_B) || IsKeyPressed(joy_code[JOY_INDEX_B]);
        else
                push = (vpadstate & VPAD_STATE_B);
        if (turbob) {
                if (push) {
                        pad_set(PAD_B, turbosw_b);
                        turbosw_b = !turbosw_b;
                }
                else
                        pad_set(PAD_B, 0);
        }
        else pad_set(PAD_B, push);

        // Select
        if (joy_code[JOY_INDEX_SELECT])
                pad_set(PAD_SELECT, IsKeyPressed(joy_code[JOY_INDEX_SELECT]));

        // Start
        if (joy_code[JOY_INDEX_START])
                pad_set(PAD_START, IsKeyPressed(joy_code[JOY_INDEX_START]));

        // Up
        if (joy_code[JOY_INDEX_UP])
                push = (vpadstate & VPAD_STATE_UP) || IsKeyPressed(joy_code[JOY_INDEX_UP]);
        else
                push = (vpadstate & VPAD_STATE_UP);
        pad_set(PAD_UP, push);

        // Down
        if (joy_code[JOY_INDEX_DOWN])
                push = (vpadstate & VPAD_STATE_DOWN) || IsKeyPressed(joy_code[JOY_INDEX_DOWN]);
        else
                push = (vpadstate & VPAD_STATE_DOWN);
        pad_set(PAD_DOWN, push);

        // Left
        if (joy_code[JOY_INDEX_LEFT])
                push = (vpadstate & VPAD_STATE_LEFT) || IsKeyPressed(joy_code[JOY_INDEX_LEFT]);
        else
                push = (vpadstate & VPAD_STATE_LEFT);
        pad_set(PAD_LEFT, push);

        // Right
        if (joy_code[JOY_INDEX_RIGHT])
                push = (vpadstate & VPAD_STATE_RIGHT) || IsKeyPressed(joy_code[JOY_INDEX_RIGHT]);
        else
                push = (vpadstate & VPAD_STATE_RIGHT);
        pad_set(PAD_RIGHT, push);

        // Pause
        if (joy_code[JOY_INDEX_ESCAPE]) {
                if (IsKeyPressed(joy_code[JOY_INDEX_ESCAPE]))
                        Pause();
        }
}

void joy_poll()
{
        /* do nothing here since key events have already been processed */
}

int joy_get_key(int index)
{
        return joy_code[index];
}

void joy_set_key(int index, int key)
{
        joy_code[index] = key;
}

int joy_get_turbo_a()
{
        return turboa;
}

int joy_get_turbo_b()
{
        return turbob;
}

void joy_set_turbo_a(int turbo)
{
        turboa = turbo;
}

void joy_set_turbo_b(int turbo)
{
        turbob = turbo;
}

void joy_set_vpadstate(int state)
{
        vpadstate = state;
}

} // extern "C"
