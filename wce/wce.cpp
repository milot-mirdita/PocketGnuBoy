#include "PocketGnuboy.h"

extern "C" {

#include "defs.h"

time_t time(time_t* t)
{
        SYSTEMTIME st, st1970;
        FILETIME ft, ft1970;

        memset(&st1970, 0, sizeof(st1970));
        st1970.wYear = 1970;
        st1970.wMonth = 1;
        st1970.wDay = 1;
        SystemTimeToFileTime(&st1970, &ft1970);

        GetSystemTime(&st);
        SystemTimeToFileTime(&st, &ft);

        ((LARGE_INTEGER*)&ft)->QuadPart
                = ((LARGE_INTEGER*)&ft)->QuadPart - ((LARGE_INTEGER*)&ft1970)->QuadPart;

        return (time_t)(((LARGE_INTEGER*)&ft)->QuadPart / 10000000);
}

char *strdup(const char *src)
{
        return _strdup(src);
}

int strcasecmp(const char *a, const char *b)
{
        return _stricmp(a, b);
}

}
