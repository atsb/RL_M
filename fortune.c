/* fortune.c */
#include "includes/larn.h"
#include "includes/fortune.h"
#include "includes/global.h"
#include "includes/io.h"

/* 
* ported the original larn12 code and made it work correctly
* previous code was very buggy and didn't work at all.
*/

static char* base = 0;
static char** flines = 0;
static int loaded = 0;
static int nlines = 0;

char*
fortune(const char* file)
{
    FILE* fp;
    long size;
    long i;
    char* p;

    if (!loaded)
    {
        fp = fopen(file, "rb");
        if (fp == 0)
            return 0;

        if (fseek(fp, 0L, SEEK_END) != 0)
        {
            fclose(fp);
            return 0;
        }

        size = ftell(fp);
        if (size <= 0)
        {
            fclose(fp);
            return 0;
        }

        if (fseek(fp, 0L, SEEK_SET) != 0)
        {
            fclose(fp);
            return 0;
        }

        base = (char*)malloc((size_t)size + 1);
        if (base == 0)
        {
            fclose(fp);
            return 0;
        }

        if (fread(base, 1, (size_t)size, fp) != (size_t)size)
        {
            fclose(fp);
            free(base);
            base = 0;
            return 0;
        }

        fclose(fp);
        base[size] = '\0';

        nlines = 0;
        for (i = 0; i < size; i++)
        {
            if (base[i] == '\n')
            {
                base[i] = '\0';
                nlines++;
            }
        }

        if (nlines <= 0)
        {
            free(base);
            base = 0;
            return 0;
        }

        flines = (char**)malloc((size_t)nlines * sizeof(char*));
        if (flines == 0)
        {
            free(base);
            base = 0;
            return 0;
        }

        p = base;
        for (i = 0; i < nlines; i++)
        {
            flines[i] = p;
            while (*p != '\0')
                p++;
            p++;
        }

        loaded = 1;
    }

    if (loaded && nlines > 0)
        return flines[rand() % nlines];

    return 0;
}
