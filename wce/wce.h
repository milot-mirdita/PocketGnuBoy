#ifndef __WCE_H__
#define __WCE_H__

extern "C" {
        void emu_init();
        void emu_close();
        void emu_reset();
        void emu_pause();
        void emu_resume();
        int emu_get_run();
        void emu_do_frame();

        int loader_init(char *s);
        void loader_unload();

        void lcd_set_contrast(int high);
        int lcd_get_contrast();

        char* get_romfile();
        int get_saveslot();
        void set_saveslot(int slot);
        void set_savedir(char* dir);
        void get_savedir(char* dir);
        void state_load(int n);
        void state_save(int n);

#define SCREENBMP_WIDTH         240
#define SCREENBMP_HEIGHT        216
        int vid_get_frameskip();
        void vid_set_frameskip(int skip);
        int vid_get_screenmode();
        void vid_set_screenmode(int mode);
        int vid_get_ddraw();
        void vid_set_ddraw(int mode);
        HDC vid_get_screen_dc();

        int pcm_get_enable();
        void pcm_set_enable(int enable);
        int pcm_get_stereo();
        void pcm_set_stereo(int stereomode);
        int pcm_get_samplerate();
        void pcm_set_samplerate(int rate);
        int pcm_get_16bits();
        void pcm_set_16bits(int b);
        int pcm_get_buffer_count();
        void pcm_set_buffer_count(int count);
        _int64 pcm_get_buffering_time();
        _int64 pcm_get_last_wait_time();

#define JOY_INDEX_A                     0
#define JOY_INDEX_B                     1
#define JOY_INDEX_SELECT        2
#define JOY_INDEX_START         3
#define JOY_INDEX_UP            4
#define JOY_INDEX_DOWN          5
#define JOY_INDEX_LEFT          6
#define JOY_INDEX_RIGHT         7
#define JOY_INDEX_ESCAPE        8
#define JOY_MAX                         9
        int joy_get_key(int index);
        void joy_set_key(int index, int key);
        int joy_get_turbo_a();
        int joy_get_turbo_b();
        void joy_set_turbo_a(int turbo);
        void joy_set_turbo_b(int turbo);

#define VPAD_STATE_NONE                 0x00
#define VPAD_STATE_UPLEFT               0x03
#define VPAD_STATE_UP                   0x01
#define VPAD_STATE_UPRIGHT              0x05
#define VPAD_STATE_LEFT                 0x02
#define VPAD_STATE_CENTER               0x00
#define VPAD_STATE_RIGHT                0x04
#define VPAD_STATE_DOWNLEFT             0x0A
#define VPAD_STATE_DOWN                 0x08
#define VPAD_STATE_DOWNRIGHT    0x0C
#define VPAD_STATE_A                    0x10
#define VPAD_STATE_B                    0x20
#define VPAD_STATE_AB                   0x30
        void joy_set_vpadstate(int state);

        DWORD timer_time_in_msec();
}

extern "C" HWND g_hwndMain;

#endif // __WCE_H__
