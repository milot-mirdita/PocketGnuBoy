#include "PocketGnuboy.h"
#include "wce.h"

extern "C" {

#include "defs.h"
#include "pcm.h"
#include "sound.h"

#define BUF_SIZ                 4096
#define WOUT_TIMEOUT    1000

struct pcm pcm;

static int buffers = 12;
static int sound = 1;
static int stereo = 0;
static int bits = 0;
static int samplerate = 22050;
static int soundresume = 1;
static int cb_per_sample = 0;
static int cb_per_sec = 0;
static int total_bytes = 0;

static HANDLE semaph = 0;
static HWAVEOUT wout = 0;
static byte *buf = 0;
static WAVEHDR* hdr = 0;
static int curbuf = 0;
static int last_wait_time = 0;

static void CALLBACK woutcallback(HANDLE hwo, UINT msg,
                                                                  DWORD user, DWORD dw1, DWORD dw2)
{
        if (msg == WOM_DONE)
                ReleaseSemaphore((HANDLE) user, 1, NULL);
}

void pcm_init()
{
        WAVEFORMATEX format;
        int i;

        if (!sound) {
                pcm.stereo = 0;
                pcm.hz = 8000;
                pcm.len = BUF_SIZ;
                buf = (byte *) malloc(pcm.len * buffers);
                pcm.buf = buf;
                pcm.pos = 0;

                snd.rate = (1<<21) / pcm.hz;
                return;
        }

        semaph = CreateSemaphore(NULL, buffers - 1, buffers -1, NULL);

        format.wFormatTag = WAVE_FORMAT_PCM;
        format.nChannels = stereo ? 2 : 1;
        format.nSamplesPerSec = samplerate;
        format.wBitsPerSample = bits ? 16 : 8;
        format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
        format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
        format.cbSize = 0;
        if (waveOutOpen(&wout, WAVE_MAPPER, &format,
                                        (DWORD) woutcallback, (DWORD) semaph, CALLBACK_FUNCTION)
                != MMSYSERR_NOERROR)
                return;

        cb_per_sample = format.wBitsPerSample / 8;
        pcm.stereo = stereo;
        pcm.hz = samplerate;
        pcm.len = BUF_SIZ;
        buf = (byte *)malloc(pcm.len * buffers * cb_per_sample);
        pcm.buf = buf;
        pcm.pos = 0;
        snd.rate = (1<<21) / pcm.hz;

        hdr = (WAVEHDR*)malloc(sizeof(WAVEHDR) * buffers);
        for (i = 0; i < buffers; i++) {
                hdr[i].lpData = (LPSTR) (buf + pcm.len * i * cb_per_sample);
                hdr[i].dwBufferLength = pcm.len * cb_per_sample;
                hdr[i].dwBytesRecorded = 0;
                hdr[i].dwUser = 0;
                hdr[i].dwFlags = 0;
                hdr[i].dwLoops = 0;
                hdr[i].lpNext = NULL;
                hdr[i].reserved = 0;
                waveOutPrepareHeader(wout, &hdr[i], sizeof WAVEHDR);
        }

        curbuf = 0;
        total_bytes = 0;
        last_wait_time = 0;
        cb_per_sec = format.nAvgBytesPerSec;

        waveOutPause(wout);
        soundresume = 1;
}

void pcm_close()
{
        if (wout) {
                waveOutReset(wout);
                for (int i = 0; i < buffers; i++)
                        waveOutUnprepareHeader(wout, &hdr[i], sizeof WAVEHDR);
                waveOutClose(wout);
                wout = 0;
        }

        if (buf) {
                free(buf);
                buf = 0;
        }
        if (hdr) {
                free(hdr);
                hdr = 0;
        }
        if (semaph) {
                CloseHandle(semaph);
                semaph = 0;
        }
        memset(&pcm, 0, sizeof pcm);
}

void pcm_suspend()
{
        if (wout) {
                waveOutPause(wout);
                soundresume = 1;
        }
}

void pcm_resume()
{
}

int pcm_submit()
{
        if (!sound || !wout) {
                pcm.pos = 0;
                return 0;
        }

        if (pcm.pos) {
                hdr[curbuf].dwBufferLength = pcm.pos * cb_per_sample;
                hdr[curbuf].dwBytesRecorded = pcm.pos * cb_per_sample;
                waveOutPrepareHeader(wout, &hdr[curbuf], sizeof WAVEHDR);
                waveOutWrite(wout, &hdr[curbuf], sizeof WAVEHDR);
                total_bytes += pcm.pos * cb_per_sample;

                if (soundresume)
                        waveOutRestart(wout);

                if (g_fThrottling) {
                        int start = timer_time_in_msec();
                        if (WaitForSingleObject(semaph, WOUT_TIMEOUT) == WAIT_TIMEOUT) {
                                waveOutReset(wout);

                                while (ReleaseSemaphore(semaph, 1, NULL));
                                waveOutPause(wout);
                                soundresume = 1;
                        }
                        last_wait_time += timer_time_in_msec() - start;
                }

                curbuf = (curbuf + 1) % buffers;
                pcm.buf = buf + pcm.len * curbuf * cb_per_sample;
                pcm.pos = 0;
        }

        return 0;
}

int pcm_get_enable()
{
        return sound;
}

void pcm_set_enable(int enable)
{
        pcm_close();
        sound = enable;
        if (sound) pcm_init();
}

int pcm_get_stereo()
{
        return stereo;
}

void pcm_set_stereo(int stereomode)
{
        int open = wout ? 1 : 0;
        pcm_close();
        stereo = stereomode;
        if (open) pcm_init();
}

int pcm_get_samplerate()
{
        return samplerate;
}

void pcm_set_samplerate(int rate)
{
        int open = wout ? 1 : 0;
        pcm_close();
        samplerate = rate;
        if (open) pcm_init();
}

int pcm_get_16bits()
{
        return bits;
}

void pcm_set_16bits(int b)
{
        int open = wout ? 1 : 0;
        pcm_close();
        bits = b;
        if (open) pcm_init();
}

int pcm_get_buffer_count()
{
        return buffers;
}

void pcm_set_buffer_count(int count)
{
        int open = wout ? 1 : 0;
        pcm_close();
        buffers = count;
        if (open) pcm_init();
}

_int64 pcm_get_buffering_time()
{
        if (!wout) return 1;

        MMTIME mmt;
        mmt.wType = TIME_BYTES;
        waveOutGetPosition(wout, &mmt, sizeof(mmt));
        return (_int64)(total_bytes - mmt.u.cb) * 1000000 / cb_per_sec;
}

_int64 pcm_get_last_wait_time()
{
        if (!wout) return 0;

        _int64 ret = last_wait_time * 1000;
        last_wait_time = 0;
        return ret;
}

}
