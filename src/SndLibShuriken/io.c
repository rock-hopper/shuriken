#include <mus-config.h>

#if (defined(__GNUC__)) && (!(defined(__cplusplus)))
  #define _GNU_SOURCE
  /* this is needed to get the vasprintf declaration */
#endif

#if USE_SND
  #include "snd.h"
#endif

#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>

#ifndef _MSC_VER
  #include <unistd.h>
#else
  #include <io.h>
  #pragma warning(disable: 4244)
#endif
#include <string.h>
#include <stdarg.h>

#ifdef _MSC_VER
  #include <direct.h>
#endif

#include <sys/stat.h>

#include "_sndlib.h"

#define HAVE_BYTESWAP_H __linux__


/* ---------------------------------------- */
/* in io.c but not in _sndlib.h (these are now internal to sndlib, but I don't like the C-oid "_" prefix): */
void mus_bint_to_char(unsigned char *j, int x);
int mus_char_to_bint(const unsigned char *inp);
void mus_lint_to_char(unsigned char *j, int x);
int mus_char_to_lint(const unsigned char *inp);
mus_long_t mus_char_to_llong(const unsigned char *inp);
mus_long_t mus_char_to_blong(const unsigned char *inp);
int mus_char_to_uninterpreted_int(const unsigned char *inp);
void mus_bfloat_to_char(unsigned char *j, float x);
float mus_char_to_bfloat(const unsigned char *inp);
void mus_lfloat_to_char(unsigned char *j, float x);
float mus_char_to_lfloat(const unsigned char *inp);
void mus_bshort_to_char(unsigned char *j, short x);
short mus_char_to_bshort(const unsigned char *inp);
void mus_lshort_to_char(unsigned char *j, short x);
short mus_char_to_lshort(const unsigned char *inp);
unsigned short mus_char_to_ubshort(const unsigned char *inp);
unsigned short mus_char_to_ulshort(const unsigned char *inp);
double mus_char_to_ldouble(const unsigned char *inp);
double mus_char_to_bdouble(const unsigned char *inp);
void mus_bdouble_to_char(unsigned char *j, double x);
void mus_blong_to_char(unsigned char *j, mus_long_t x);
void mus_llong_to_char(unsigned char *j, mus_long_t x);
unsigned int mus_char_to_ubint(const unsigned char *inp);
unsigned int mus_char_to_ulint(const unsigned char *inp);
/* ---------------------------------------- */

#define MUS_BYTE_TO_SAMPLE(n) (((mus_float_t)(n) / (mus_float_t)(1 << 7)))
#define MUS_SHORT_TO_SAMPLE(n) (((mus_float_t)(n) / (mus_float_t)(1 << 15)))
#define MUS_INT_TO_SAMPLE(n) (((mus_float_t)(n) / (mus_float_t)(1 << 23)))
#define MUS_INT24_TO_SAMPLE(n) (((mus_float_t)(n) / (mus_float_t)(1 << 23)))
#define MUS_SAMPLE_TO_INT(n) ((int)((n) * (1 << 23)))
#define MUS_SAMPLE_TO_INT24(n) ((int)((n) * (1 << 23)))
#define MUS_SAMPLE_TO_SHORT(n) ((short)((n) * (1 << 15)))
#define MUS_SAMPLE_TO_BYTE(n) ((char)((n) * (1 << 7)))


static mus_long_t mus_maximum_malloc = MUS_MAX_MALLOC_DEFAULT;

mus_long_t mus_max_malloc(void)
{
  return(mus_maximum_malloc);
}

mus_long_t mus_set_max_malloc(mus_long_t new_max)
{
  mus_maximum_malloc = new_max;
  return(new_max);
}



static mus_long_t mus_maximum_table_size = MUS_MAX_TABLE_SIZE_DEFAULT;

mus_long_t mus_max_table_size(void)
{
  return(mus_maximum_table_size);
}

mus_long_t mus_set_max_table_size(mus_long_t new_max)
{
  mus_maximum_table_size = new_max;
  return(new_max);
}



void mus_bint_to_char(unsigned char *j, int x)
{
  unsigned char *ox = (unsigned char *)&x;
#if MUS_LITTLE_ENDIAN
  j[0] = ox[3]; j[1] = ox[2]; j[2] = ox[1]; j[3] = ox[0];
#else
  memcpy((void *)j, (void *)ox, 4);
#endif
}


int mus_char_to_bint(const unsigned char *inp)
{
  int o;
  unsigned char *outp = (unsigned char *)&o;
#if MUS_LITTLE_ENDIAN
  outp[0] = inp[3]; outp[1] = inp[2]; outp[2] = inp[1]; outp[3] = inp[0];
#else
  memcpy((void *)outp, (void *)inp, 4);
#endif
  return(o);
}


void mus_lint_to_char(unsigned char *j, int x)
{
  unsigned char *ox = (unsigned char *)&x;
#if (!MUS_LITTLE_ENDIAN)
  j[0] = ox[3]; j[1] = ox[2]; j[2] = ox[1]; j[3] = ox[0];
#else
  memcpy((void *)j, (void *)ox, 4);
#endif
}


int mus_char_to_lint(const unsigned char *inp)
{
  int o;
  unsigned char *outp = (unsigned char *)&o;
#if (!MUS_LITTLE_ENDIAN)
  outp[0] = inp[3]; outp[1] = inp[2]; outp[2] = inp[1]; outp[3] = inp[0];
#else
  memcpy((void *)outp, (void *)inp, 4);
#endif
  return(o);
}


void mus_blong_to_char(unsigned char *j, mus_long_t x)
{
  unsigned char *ox = (unsigned char *)&x;
#if (!MUS_LITTLE_ENDIAN)
  memcpy((void *)j, (void *)ox, 8);
#else
  j[0] = ox[7]; j[1] = ox[6]; j[2] = ox[5]; j[3] = ox[4]; j[4] = ox[3]; j[5] = ox[2]; j[6] = ox[1]; j[7] = ox[0];
#endif
}


mus_long_t mus_char_to_blong(const unsigned char *inp)
{
  mus_long_t o;
  unsigned char *outp = (unsigned char *)&o;
#if MUS_LITTLE_ENDIAN
  outp[0] = inp[7]; outp[1] = inp[6]; outp[2] = inp[5]; outp[3] = inp[4]; outp[4] = inp[3]; outp[5] = inp[2]; outp[6] = inp[1]; outp[7] = inp[0];
#else
  memcpy((void *)outp, (void *)inp, 8);
#endif
  return(o);
}


void mus_llong_to_char(unsigned char *j, mus_long_t x)
{
  unsigned char *ox = (unsigned char *)&x;
#if (MUS_LITTLE_ENDIAN)
  memcpy((void *)j, (void *)ox, 8);
#else
  j[0] = ox[7]; j[1] = ox[6]; j[2] = ox[5]; j[3] = ox[4]; j[4] = ox[3]; j[5] = ox[2]; j[6] = ox[1]; j[7] = ox[0];
#endif
}


mus_long_t mus_char_to_llong(const unsigned char *inp)
{
  mus_long_t o;
  unsigned char *outp = (unsigned char *)&o;
#if (!MUS_LITTLE_ENDIAN)
  outp[0] = inp[7]; outp[1] = inp[6]; outp[2] = inp[5]; outp[3] = inp[4]; outp[4] = inp[3]; outp[5] = inp[2]; outp[6] = inp[1]; outp[7] = inp[0];
#else
  memcpy((void *)outp, (void *)inp, 8);
#endif
  return(o);
}


void mus_bfloat_to_char(unsigned char *j, float x)
{
  unsigned char *ox = (unsigned char *)&x;
#if MUS_LITTLE_ENDIAN
  j[0] = ox[3]; j[1] = ox[2]; j[2] = ox[1]; j[3] = ox[0];
#else
  memcpy((void *)j, (void *)ox, 4);
#endif
}


float mus_char_to_bfloat(const unsigned char *inp)
{
  float o;
  unsigned char *outp = (unsigned char *)&o;
#if MUS_LITTLE_ENDIAN
  outp[0] = inp[3]; outp[1] = inp[2]; outp[2] = inp[1]; outp[3] = inp[0];
#else
  memcpy((void *)outp, (void *)inp, 4);
#endif
  return(o);
}


void mus_lfloat_to_char(unsigned char *j, float x)
{
  unsigned char *ox = (unsigned char *)&x;
#if (!MUS_LITTLE_ENDIAN)
  j[0] = ox[3]; j[1] = ox[2]; j[2] = ox[1]; j[3] = ox[0];
#else
  memcpy((void *)j, (void *)ox, 4);
#endif
}


float mus_char_to_lfloat(const unsigned char *inp)
{
  float o;
  unsigned char *outp = (unsigned char *)&o;
#if (!MUS_LITTLE_ENDIAN)
  outp[0] = inp[3]; outp[1] = inp[2]; outp[2] = inp[1]; outp[3] = inp[0];
#else
  memcpy((void *)outp, (void *)inp, 4);
#endif
  return(o);
}


void mus_bshort_to_char(unsigned char *j, short x)
{
  unsigned char *ox = (unsigned char *)&x;
#if MUS_LITTLE_ENDIAN
  j[0] = ox[1]; j[1] = ox[0];
#else
  memcpy((void *)j, (void *)ox, 2); /* I wonder if this is faster */
#endif
}


short mus_char_to_bshort(const unsigned char *inp)
{
  short o;
  unsigned char *outp = (unsigned char *)&o;
#if MUS_LITTLE_ENDIAN
  outp[0] = inp[1]; outp[1] = inp[0];
#else
  memcpy((void *)outp, (void *)inp, 2);
#endif
  return(o);
}


void mus_lshort_to_char(unsigned char *j, short x)
{
  unsigned char *ox = (unsigned char *)&x;
#if (!MUS_LITTLE_ENDIAN)
  j[0] = ox[1]; j[1] = ox[0];
#else
  memcpy((void *)j, (void *)ox, 2);
#endif
}


short mus_char_to_lshort(const unsigned char *inp)
{
  short o;
  unsigned char *outp = (unsigned char *)&o;
#if (!MUS_LITTLE_ENDIAN)
  outp[0] = inp[1]; outp[1] = inp[0];
#else
  memcpy((void *)outp, (void *)inp, 2);
#endif
  return(o);
}


#if ((MUS_LITTLE_ENDIAN) && (!HAVE_BYTESWAP_H)) || ((!MUS_LITTLE_ENDIAN) && (HAVE_SUN))
static void mus_ubshort_to_char(unsigned char *j, unsigned short x)
{
  unsigned char *ox = (unsigned char *)&x;
#if MUS_LITTLE_ENDIAN
  j[0] = ox[1]; j[1] = ox[0];
#else
  memcpy((void *)j, (void *)ox, 2);
#endif
}
#endif


unsigned short mus_char_to_ubshort(const unsigned char *inp)
{
  unsigned short o;
  unsigned char *outp = (unsigned char *)&o;
#if MUS_LITTLE_ENDIAN
  outp[0] = inp[1]; outp[1] = inp[0];
#else
  memcpy((void *)outp, (void *)inp, 2);
#endif
  return(o);
}


#if (!MUS_LITTLE_ENDIAN) && (!HAVE_BYTESWAP_H)
static void mus_ulshort_to_char(unsigned char *j, unsigned short x)
{
  unsigned char *ox = (unsigned char *)&x;
#if (!MUS_LITTLE_ENDIAN)
  j[0] = ox[1]; j[1] = ox[0];
#else
  memcpy((void *)j, (void *)ox, 2);
#endif
}
#endif


unsigned short mus_char_to_ulshort(const unsigned char *inp)
{
  unsigned short o;
  unsigned char *outp = (unsigned char *)&o;
#if (!MUS_LITTLE_ENDIAN)
  outp[0] = inp[1]; outp[1] = inp[0];
#else
  memcpy((void *)outp, (void *)inp, 2);
#endif
  return(o);
}


void mus_bdouble_to_char(unsigned char *j, double x)
{
  unsigned char *ox = (unsigned char *)&x;
#if (MUS_LITTLE_ENDIAN)
  j[0] = ox[7]; j[1] = ox[6]; j[2] = ox[5]; j[3] = ox[4]; j[4] = ox[3]; j[5] = ox[2]; j[6] = ox[1]; j[7] = ox[0];
#else
  memcpy((void *)j, (void *)ox, 8);
#endif
}


double mus_char_to_ldouble(const unsigned char *inp)
{
  double o;
  unsigned char *outp = (unsigned char *)&o;
#if (MUS_LITTLE_ENDIAN)
  memcpy((void *)outp, (void *)inp, 8);
#else
  outp[0] = inp[7]; outp[1] = inp[6]; outp[2] = inp[5]; outp[3] = inp[4]; outp[4] = inp[3]; outp[5] = inp[2]; outp[6] = inp[1]; outp[7] = inp[0];
#endif
  return(o);
}


#if (!MUS_LITTLE_ENDIAN)
static void mus_ldouble_to_char(unsigned char *j, double x)
{
  unsigned char *ox = (unsigned char *)&x;
#if (MUS_LITTLE_ENDIAN)
  memcpy((void *)j, (void *)ox, 8);
#else
  j[0] = ox[7]; j[1] = ox[6]; j[2] = ox[5]; j[3] = ox[4]; j[4] = ox[3]; j[5] = ox[2]; j[6] = ox[1]; j[7] = ox[0];
#endif
}
#endif


double mus_char_to_bdouble(const unsigned char *inp)
{
  double o;
  unsigned char *outp = (unsigned char *)&o;
#if (MUS_LITTLE_ENDIAN)
  outp[0] = inp[7]; outp[1] = inp[6]; outp[2] = inp[5]; outp[3] = inp[4]; outp[4] = inp[3]; outp[5] = inp[2]; outp[6] = inp[1]; outp[7] = inp[0];
#else
  memcpy((void *)outp, (void *)inp, 8);
#endif
  return(o);
}


int mus_char_to_uninterpreted_int(const unsigned char *inp)
{
  int o;
  unsigned char *outp = (unsigned char *)&o;
  memcpy((void *)outp, (void *)inp, 4);
  return(o);
}


unsigned int mus_char_to_ubint(const unsigned char *inp)
{
  unsigned int o;
  unsigned char *outp = (unsigned char *)&o;
#if MUS_LITTLE_ENDIAN
  outp[0] = inp[3]; outp[1] = inp[2]; outp[2] = inp[1]; outp[3] = inp[0];
#else
  memcpy((void *)outp, (void *)inp, 4);
#endif
  return(o);
}


unsigned int mus_char_to_ulint(const unsigned char *inp)
{
  unsigned int o;
  unsigned char *outp = (unsigned char *)&o;
#if (!MUS_LITTLE_ENDIAN)
  outp[0] = inp[3]; outp[1] = inp[2]; outp[2] = inp[1]; outp[3] = inp[0];
#else
  memcpy((void *)outp, (void *)inp, 4);
#endif
  return(o);
}


#if HAVE_BYTESWAP_H
  #include <byteswap.h>
#endif

#if MUS_LITTLE_ENDIAN

  #if HAVE_BYTESWAP_H
    #define big_endian_short(n)                  ((short)(bswap_16((*((unsigned short *)n)))))
    #define big_endian_int(n)                    ((int)(bswap_32((*((unsigned int *)n)))))
    #define big_endian_unsigned_short(n)         ((unsigned short)(bswap_16((*((unsigned short *)n)))))
  #else
    #define big_endian_short(n)                  (mus_char_to_bshort(n))
    #define big_endian_int(n)                    (mus_char_to_bint(n))
    #define big_endian_unsigned_short(n)         (mus_char_to_ubshort(n))
  #endif

  #define big_endian_float(n)                    (mus_char_to_bfloat(n))
  #define big_endian_double(n)                   (mus_char_to_bdouble(n))
  /* why doesn't bswap work in this case? */

  #define little_endian_short(n)                 (*((short *)n))
  #define little_endian_int(n)                   (*((int *)n))
  #define little_endian_float(n)                 (*((float *)n))
  #define little_endian_double(n)                (*((double *)n))
  #define little_endian_unsigned_short(n)        (*((unsigned short *)n))

  #if HAVE_BYTESWAP_H
    #define set_big_endian_short(n, x)           (*((short *)n)) = ((short)(bswap_16(x)))
    #define set_big_endian_int(n, x)             (*((int *)n)) = ((int)(bswap_32(x)))
    #define set_big_endian_unsigned_short(n, x)  (*((unsigned short *)n)) = ((unsigned short)(bswap_16(x)))
  #else
    #define set_big_endian_short(n, x)           mus_bshort_to_char(n, x)
    #define set_big_endian_int(n, x)             mus_bint_to_char(n, x)
    #define set_big_endian_unsigned_short(n, x)  mus_ubshort_to_char(n, x)
  #endif

  #define set_big_endian_float(n, x)             mus_bfloat_to_char(n, x)
  #define set_big_endian_double(n, x)            mus_bdouble_to_char(n, x)

  #define set_little_endian_short(n, x)          (*((short *)n)) = x
  #define set_little_endian_int(n, x)            (*((int *)n)) = x
  #define set_little_endian_float(n, x)          (*((float *)n)) = x
  #define set_little_endian_double(n, x)         (*((double *)n)) = x
  #define set_little_endian_unsigned_short(n, x) (*((unsigned short *)n)) = x

#else

#if (!HAVE_SUN)
    #define big_endian_short(n)                  (*((short *)n))
    #define big_endian_int(n)                    (*((int *)n))
    #define big_endian_float(n)                  (*((float *)n))
    #define big_endian_double(n)                 (*((double *)n))
    #define big_endian_unsigned_short(n)         (*((unsigned short *)n))

    #define set_big_endian_short(n, x)           (*((short *)n)) = x
    #define set_big_endian_int(n, x)             (*((int *)n)) = x
    #define set_big_endian_float(n, x)           (*((float *)n)) = x
    #define set_big_endian_double(n, x)          (*((double *)n)) = x
    #define set_big_endian_unsigned_short(n, x)  (*((unsigned short *)n)) = x
  #else
    #define big_endian_short(n)                  (mus_char_to_bshort(n))
    #define big_endian_int(n)                    (mus_char_to_bint(n))
    #define big_endian_float(n)                  (mus_char_to_bfloat(n))
    #define big_endian_double(n)                 (mus_char_to_bdouble(n))
    #define big_endian_unsigned_short(n)         (mus_char_to_ubshort(n))

    #define set_big_endian_short(n, x)           mus_bshort_to_char(n, x)
    #define set_big_endian_int(n, x)             mus_bint_to_char(n, x)
    #define set_big_endian_float(n, x)           mus_bfloat_to_char(n, x)
    #define set_big_endian_double(n, x)          mus_bdouble_to_char(n, x)
    #define set_big_endian_unsigned_short(n, x)  mus_ubshort_to_char(n, x)
  #endif

  #if HAVE_BYTESWAP_H
    #define little_endian_short(n)               ((short)(bswap_16((*((unsigned short *)n)))))
    #define little_endian_int(n)                 ((int)(bswap_32((*((unsigned int *)n)))))
    #define little_endian_unsigned_short(n)      ((unsigned short)(bswap_16((*((unsigned short *)n)))))
  #else
    #define little_endian_short(n)               (mus_char_to_lshort(n))
    #define little_endian_int(n)                 (mus_char_to_lint(n))
    #define little_endian_unsigned_short(n)      (mus_char_to_ulshort(n))
  #endif
  #define little_endian_float(n)                 (mus_char_to_lfloat(n))
  #define little_endian_double(n)                (mus_char_to_ldouble(n))

  #if HAVE_BYTESWAP_H
    #define set_little_endian_short(n, x)        (*((short *)n)) = ((short)(bswap_16(x)))
    #define set_little_endian_int(n, x)          (*((int *)n)) = ((int)(bswap_32(x)))
    #define set_little_endian_unsigned_short(n, x) (*((unsigned short *)n)) = ((unsigned short)(bswap_16(x)))
  #else
    #define set_little_endian_short(n, x)        mus_lshort_to_char(n, x)
    #define set_little_endian_int(n, x)          mus_lint_to_char(n, x)
    #define set_little_endian_unsigned_short(n, x) mus_ulshort_to_char(n, x)
  #endif
  #define set_little_endian_float(n, x)          mus_lfloat_to_char(n, x)
  #define set_little_endian_double(n, x)         mus_ldouble_to_char(n, x)

#endif


/* ---------------- file descriptors ---------------- */

static bool clipping_default = false;
bool mus_clipping(void) {return(clipping_default);}
bool mus_set_clipping(bool new_value) {clipping_default = new_value; return(new_value);}


typedef struct {
  char *name;
  int data_format, bytes_per_sample, chans, header_type;
  bool clipping;
  mus_long_t data_location;
} io_fd;

static int io_fd_size = 0;
static io_fd **io_fds = NULL;
#define IO_FD_ALLOC_SIZE 8


int mus_file_open_descriptors(int tfd, const char *name, int format, int size /* datum size */, mus_long_t location, int chans, int type)
{
  int err = MUS_NO_ERROR;
  if (io_fd_size == 0)
    {
      io_fd_size = tfd + IO_FD_ALLOC_SIZE;
      io_fds = (io_fd **)calloc(io_fd_size, sizeof(io_fd *));
    }

  if (io_fds)
    {
      if (io_fd_size <= tfd)
	{
	  int i, lim;
	  lim = io_fd_size;
	  io_fd_size = tfd + IO_FD_ALLOC_SIZE;
	  io_fds = (io_fd **)realloc(io_fds, io_fd_size * sizeof(io_fd *));
	  for (i = lim; i < io_fd_size; i++) io_fds[i] = NULL;
	}

      if (io_fds[tfd] == NULL)
	io_fds[tfd] = (io_fd *)calloc(1, sizeof(io_fd));

      if (io_fds[tfd])
	{
	  io_fd *fd;
	  fd = io_fds[tfd];
	  fd->data_format = format;
	  fd->bytes_per_sample = size;
	  fd->data_location = location;
	  fd->clipping = clipping_default;
	  fd->header_type = type;
	  fd->chans = chans;
	  if (name)
	    {
	      fd->name = (char *)calloc(strlen(name) + 1, sizeof(char));
	      strcpy(fd->name, name);
	    }
	}
      else err = MUS_MEMORY_ALLOCATION_FAILED;
    }
  else err = MUS_MEMORY_ALLOCATION_FAILED;
  return(err);
}


bool mus_file_clipping(int tfd)
{
  io_fd *fd;
  if ((io_fds == NULL) || (tfd >= io_fd_size) || (tfd < 0) || (io_fds[tfd] == NULL)) return(false);
  fd = io_fds[tfd];
  return(fd->clipping);
}


int mus_file_set_clipping(int tfd, bool clipped)
{
  io_fd *fd;
  if ((io_fds == NULL) || (tfd >= io_fd_size) || (tfd < 0) || (io_fds[tfd] == NULL)) return(MUS_FILE_DESCRIPTORS_NOT_INITIALIZED);
  fd = io_fds[tfd];
  fd->clipping = clipped;
  return(MUS_NO_ERROR);
}


int mus_file_set_header_type(int tfd, int type)
{
  io_fd *fd;
  if ((io_fds == NULL) || (tfd >= io_fd_size) || (tfd < 0) || (io_fds[tfd] == NULL)) return(MUS_FILE_DESCRIPTORS_NOT_INITIALIZED);
  fd = io_fds[tfd];
  fd->header_type = type;
  return(MUS_NO_ERROR);
}


int mus_file_header_type(int tfd)
{
  io_fd *fd;
  if ((io_fds == NULL) || (tfd >= io_fd_size) || (tfd < 0) || (io_fds[tfd] == NULL)) return(MUS_FILE_DESCRIPTORS_NOT_INITIALIZED);
  fd = io_fds[tfd];
  return(fd->header_type);
}


char *mus_file_fd_name(int tfd)
{
  io_fd *fd;
  if ((io_fds == NULL) || (tfd >= io_fd_size) || (tfd < 0) || (io_fds[tfd] == NULL)) return(NULL);
  fd = io_fds[tfd];
  return(fd->name);
}


int mus_file_set_chans(int tfd, int chans)
{
  io_fd *fd;
  if ((io_fds == NULL) || (tfd >= io_fd_size) || (tfd < 0) || (io_fds[tfd] == NULL)) return(MUS_FILE_DESCRIPTORS_NOT_INITIALIZED);
  fd = io_fds[tfd];
  fd->chans = chans;
  return(MUS_NO_ERROR);
}


/* ---------------- open, creat, close ---------------- */

int mus_file_open_read(const char *arg) 
{
  int fd;
#if (defined(_MSC_VER) || __CYGWIN__)
  fd = OPEN(arg, O_RDONLY | O_BINARY, 0);
#else
  fd = OPEN(arg, O_RDONLY, 0);
#endif
  return(fd);
}


bool mus_file_probe(const char *arg) 
{
#ifndef _MSC_VER
  if (!arg) return(false);
  return(access(arg, F_OK) == 0);
#else
  int fd;
#ifdef O_NONBLOCK
  fd = OPEN(arg, O_RDONLY, O_NONBLOCK);
#else
  fd = OPEN(arg, O_RDONLY, 0);
#endif
  if (fd == -1) return(false);
  CLOSE(fd, arg);
  return(true);
#endif
}


int mus_file_open_write(const char *arg)
{
  int fd;
#if (defined(_MSC_VER) || __CYGWIN__)
  if ((fd = OPEN(arg, O_RDWR | O_BINARY, 0)) == -1)
    fd = CREAT(arg, S_IREAD | S_IWRITE);               /* 0x0100 | 0x0080 */
#else
  if ((fd = OPEN(arg, O_RDWR, 0)) == -1)
    fd = CREAT(arg, 0666);  /* equivalent to the new open(arg, O_RDWR | O_CREAT | O_TRUNC, 0666) */
#endif
  else lseek(fd, 0L, SEEK_END);
  return(fd);
}


int mus_file_create(const char *arg) 
{ 
#if (defined(_MSC_VER ) || __CYGWIN__)
  return(CREAT(arg, S_IREAD | S_IWRITE)); 
#else 
  return(CREAT(arg, 0666)); 
#endif 
} 


int mus_file_reopen_write(const char *arg)
{
  int fd;
#if (defined(_MSC_VER) || __CYGWIN__)
  fd = OPEN(arg, O_RDWR | O_BINARY, 0);
#else
  fd = OPEN(arg, O_RDWR, 0);
#endif
  return(fd);
}


int mus_file_close(int fd)
{
  io_fd *fdp;
  int close_result = 0;

  if ((io_fds == NULL) || (fd >= io_fd_size) || (fd < 0) || (io_fds[fd] == NULL)) return(MUS_FILE_DESCRIPTORS_NOT_INITIALIZED);
  fdp = io_fds[fd];

#if USE_SND
  CLOSE(fd, fdp->name);
#else
  close_result = close(fd);
#endif

  if (fdp->name) {free(fdp->name); fdp->name = NULL;}
  free(fdp);
  io_fds[fd] = NULL;

  if (close_result < 0)
    return(MUS_CANT_CLOSE_FILE);
  return(MUS_NO_ERROR);
}



/* ---------------- seek ---------------- */

mus_long_t mus_file_seek_frame(int tfd, mus_long_t frame)
{
  io_fd *fd;
  if (io_fds == NULL) 
    return(mus_error(MUS_FILE_DESCRIPTORS_NOT_INITIALIZED, "mus_file_seek_frame: no file descriptors!"));

  if (tfd >= io_fd_size)
    return(mus_error(MUS_FILE_DESCRIPTORS_NOT_INITIALIZED,
		     "mus_file_seek_frame: file descriptors not realloc'd? (tfd: %d, io_fd_size: %d)", tfd, io_fd_size));

  if ((tfd < 0) || 
      (io_fds[tfd] == NULL))
    return(mus_error(MUS_FILE_DESCRIPTORS_NOT_INITIALIZED, "mus_file_seek_frame: file descriptor = %d?", tfd));

  fd = io_fds[tfd];
  if (fd->data_format == MUS_UNKNOWN) 
    return(mus_error(MUS_NOT_A_SOUND_FILE, "mus_file_seek_frame: invalid data format for %s", fd->name));

  return(lseek(tfd, fd->data_location + (fd->chans * frame * fd->bytes_per_sample), SEEK_SET));
}



/* ---------------- mulaw/alaw conversions ----------------
 *
 *      x : input signal with max value 32767
 *     mu : compression parameter (mu = 255 used for telephony)
 *     y = (32767/log(1+mu))*log(1+mu*abs(x)/32767)*sign(x); -- this isn't right -- typo?
 */

/* from sox g711.c */
#define	QUANT_MASK	(0xf)		/* Quantization field mask. */
#define	SEG_SHIFT	(4)		/* Left shift for segment number. */

static short seg_end[8] = {0xFF, 0x1FF, 0x3FF, 0x7FF,  0xFFF, 0x1FFF, 0x3FFF, 0x7FFF};

static int search(int val, short *table, int size)
{
  int i;
  for (i = 0; i < size; i++) {if (val <= *table++) return(i);}
  return(size);
}

static unsigned char to_alaw(int pcm_val)
{
  int mask, seg;
  if (pcm_val >= 0) mask = 0xD5; else {mask = 0x55; pcm_val = -pcm_val - 8;}
  seg = search(pcm_val, seg_end, 8);
  if (seg >= 8)	return(0x7F ^ mask);
  else 
    {
      unsigned char aval;
      aval = seg << SEG_SHIFT;
      if (seg < 2) aval |= (pcm_val >> 4) & QUANT_MASK; else aval |= (pcm_val >> (seg + 3)) & QUANT_MASK;
      return(aval ^ mask);
    }
}

#define A_(a) MUS_SHORT_TO_SAMPLE(a)
static const mus_float_t mus_alaw[256] = {
  A_(-5504), A_(-5248), A_(-6016), A_(-5760), A_(-4480), A_(-4224), A_(-4992), A_(-4736), A_(-7552), A_(-7296), 
  A_(-8064), A_(-7808), A_(-6528), A_(-6272), A_(-7040), A_(-6784), A_(-2752), A_(-2624), A_(-3008), A_(-2880), 
  A_(-2240), A_(-2112), A_(-2496), A_(-2368), A_(-3776), A_(-3648), A_(-4032), A_(-3904), A_(-3264), A_(-3136), 
  A_(-3520), A_(-3392), A_(-22016), A_(-20992), A_(-24064), A_(-23040), A_(-17920), A_(-16896), A_(-19968), A_(-18944), 
  A_(-30208), A_(-29184), A_(-32256), A_(-31232), A_(-26112), A_(-25088), A_(-28160), A_(-27136), A_(-11008), A_(-10496), 
  A_(-12032), A_(-11520), A_(-8960), A_(-8448), A_(-9984), A_(-9472), A_(-15104), A_(-14592), A_(-16128), A_(-15616), 
  A_(-13056), A_(-12544), A_(-14080), A_(-13568), A_(-344), A_(-328), A_(-376), A_(-360), A_(-280), A_(-264), A_(-312), 
  A_(-296), A_(-472), A_(-456), A_(-504), A_(-488), A_(-408), A_(-392), A_(-440), A_(-424), A_(-88), A_(-72), A_(-120), 
  A_(-104), A_(-24), A_(-8), A_(-56), A_(-40), A_(-216), A_(-200), A_(-248), A_(-232), A_(-152), A_(-136), A_(-184), 
  A_(-168), A_(-1376), A_(-1312), A_(-1504), A_(-1440), A_(-1120), A_(-1056), A_(-1248), A_(-1184), A_(-1888), A_(-1824), 
  A_(-2016), A_(-1952), A_(-1632), A_(-1568), A_(-1760), A_(-1696), A_(-688), A_(-656), A_(-752), A_(-720), A_(-560), 
  A_(-528), A_(-624), A_(-592), A_(-944), A_(-912), A_(-1008), A_(-976), A_(-816), A_(-784), A_(-880), A_(-848), A_(5504), 
  A_(5248), A_(6016), A_(5760), A_(4480), A_(4224), A_(4992), A_(4736), A_(7552), A_(7296), A_(8064), A_(7808), A_(6528), 
  A_(6272), A_(7040), A_(6784), A_(2752), A_(2624), A_(3008), A_(2880), A_(2240), A_(2112), A_(2496), A_(2368), A_(3776), 
  A_(3648), A_(4032), A_(3904), A_(3264), A_(3136), A_(3520), A_(3392), A_(22016), A_(20992), A_(24064), A_(23040), A_(17920), 
  A_(16896), A_(19968), A_(18944), A_(30208), A_(29184), A_(32256), A_(31232), A_(26112), A_(25088), A_(28160), A_(27136), 
  A_(11008), A_(10496), A_(12032), A_(11520), A_(8960), A_(8448), A_(9984), A_(9472), A_(15104), A_(14592), A_(16128), 
  A_(15616), A_(13056), A_(12544), A_(14080), A_(13568), A_(344), A_(328), A_(376), A_(360), A_(280), A_(264), A_(312), 
  A_(296), A_(472), A_(456), A_(504), A_(488), A_(408), A_(392), A_(440), A_(424), A_(88), A_(72), A_(120), A_(104), 
  A_(24), A_(8), A_(56), A_(40), A_(216), A_(200), A_(248), A_(232), A_(152), A_(136), A_(184), A_(168), A_(1376), 
  A_(1312), A_(1504), A_(1440), A_(1120), A_(1056), A_(1248), A_(1184), A_(1888), A_(1824), A_(2016), A_(1952), A_(1632), 
  A_(1568), A_(1760), A_(1696), A_(688), A_(656), A_(752), A_(720), A_(560), A_(528), A_(624), A_(592), A_(944), A_(912), 
  A_(1008), A_(976), A_(816), A_(784), A_(880), A_(848)
};



#define	BIAS		(0x84)		/* Bias for linear code. */

static unsigned char to_mulaw(int pcm_val)
{
  int mask;
  int seg;
  if (pcm_val < 0) {pcm_val = BIAS - pcm_val; mask = 0x7F;} else {pcm_val += BIAS; mask = 0xFF;}
  seg = search(pcm_val, seg_end, 8);
  if (seg >= 8) return(0x7F ^ mask);
  else 
    {
      unsigned char uval;
      uval = (seg << 4) | ((pcm_val >> (seg + 3)) & 0xF);
      return(uval ^ mask);
    }
}

#define MU_(a) MUS_SHORT_TO_SAMPLE(a)
static const mus_float_t mus_mulaw[256] = {
  MU_(-32124), MU_(-31100), MU_(-30076), MU_(-29052), MU_(-28028), MU_(-27004), MU_(-25980), MU_(-24956), MU_(-23932), MU_(-22908), 
  MU_(-21884), MU_(-20860), MU_(-19836), MU_(-18812), MU_(-17788), MU_(-16764), MU_(-15996), MU_(-15484), MU_(-14972), MU_(-14460), 
  MU_(-13948), MU_(-13436), MU_(-12924), MU_(-12412), MU_(-11900), MU_(-11388), MU_(-10876), MU_(-10364), MU_(-9852), MU_(-9340), 
  MU_(-8828), MU_(-8316), MU_(-7932), MU_(-7676), MU_(-7420), MU_(-7164), MU_(-6908), MU_(-6652), MU_(-6396), MU_(-6140), MU_(-5884), 
  MU_(-5628), MU_(-5372), MU_(-5116), MU_(-4860), MU_(-4604), MU_(-4348), MU_(-4092), MU_(-3900), MU_(-3772), MU_(-3644), MU_(-3516), 
  MU_(-3388), MU_(-3260), MU_(-3132), MU_(-3004), MU_(-2876), MU_(-2748), MU_(-2620), MU_(-2492), MU_(-2364), MU_(-2236), MU_(-2108), 
  MU_(-1980), MU_(-1884), MU_(-1820), MU_(-1756), MU_(-1692), MU_(-1628), MU_(-1564), MU_(-1500), MU_(-1436), MU_(-1372), MU_(-1308), 
  MU_(-1244), MU_(-1180), MU_(-1116), MU_(-1052), MU_(-988), MU_(-924), MU_(-876), MU_(-844), MU_(-812), MU_(-780), MU_(-748), 
  MU_(-716), MU_(-684), MU_(-652), MU_(-620), MU_(-588), MU_(-556), MU_(-524), MU_(-492), MU_(-460), MU_(-428), MU_(-396), 
  MU_(-372), MU_(-356), MU_(-340), MU_(-324), MU_(-308), MU_(-292), MU_(-276), MU_(-260), MU_(-244), MU_(-228), MU_(-212), 
  MU_(-196), MU_(-180), MU_(-164), MU_(-148), MU_(-132), MU_(-120), MU_(-112), MU_(-104), MU_(-96), MU_(-88), MU_(-80), MU_(-72), 
  MU_(-64), MU_(-56), MU_(-48), MU_(-40), MU_(-32), MU_(-24), MU_(-16), MU_(-8), MU_(0), MU_(32124), MU_(31100), MU_(30076), 
  MU_(29052), MU_(28028), MU_(27004), MU_(25980), MU_(24956), MU_(23932), MU_(22908), MU_(21884), MU_(20860), MU_(19836), 
  MU_(18812), MU_(17788), MU_(16764), MU_(15996), MU_(15484), MU_(14972), MU_(14460), MU_(13948), MU_(13436), MU_(12924), 
  MU_(12412), MU_(11900), MU_(11388), MU_(10876), MU_(10364), MU_(9852), MU_(9340), MU_(8828), MU_(8316), MU_(7932), MU_(7676), 
  MU_(7420), MU_(7164), MU_(6908), MU_(6652), MU_(6396), MU_(6140), MU_(5884), MU_(5628), MU_(5372), MU_(5116), MU_(4860), 
  MU_(4604), MU_(4348), MU_(4092), MU_(3900), MU_(3772), MU_(3644), MU_(3516), MU_(3388), MU_(3260), MU_(3132), MU_(3004), 
  MU_(2876), MU_(2748), MU_(2620), MU_(2492), MU_(2364), MU_(2236), MU_(2108), MU_(1980), MU_(1884), MU_(1820), MU_(1756), 
  MU_(1692), MU_(1628), MU_(1564), MU_(1500), MU_(1436), MU_(1372), MU_(1308), MU_(1244), MU_(1180), MU_(1116), MU_(1052), 
  MU_(988), MU_(924), MU_(876), MU_(844), MU_(812), MU_(780), MU_(748), MU_(716), MU_(684), MU_(652), MU_(620), MU_(588), 
  MU_(556), MU_(524), MU_(492), MU_(460), MU_(428), MU_(396), MU_(372), MU_(356), MU_(340), MU_(324), MU_(308), MU_(292), 
  MU_(276), MU_(260), MU_(244), MU_(228), MU_(212), MU_(196), MU_(180), MU_(164), MU_(148), MU_(132), MU_(120), MU_(112), 
  MU_(104), MU_(96), MU_(88), MU_(80), MU_(72), MU_(64), MU_(56), MU_(48), MU_(40), MU_(32), MU_(24), MU_(16), MU_(8), MU_(0)
};


#define B_(a) MUS_BYTE_TO_SAMPLE(a)
static const mus_float_t mus_byte[256] = {
  B_(0), B_(1), B_(2), B_(3), B_(4), B_(5), B_(6), B_(7), B_(8), B_(9), B_(10), B_(11), B_(12), B_(13), B_(14), B_(15), B_(16), 
  B_(17), B_(18), B_(19), B_(20), B_(21), B_(22), B_(23), B_(24), B_(25), B_(26), B_(27), B_(28), B_(29), B_(30), B_(31), B_(32), 
  B_(33), B_(34), B_(35), B_(36), B_(37), B_(38), B_(39), B_(40), B_(41), B_(42), B_(43), B_(44), B_(45), B_(46), B_(47), B_(48), 
  B_(49), B_(50), B_(51), B_(52), B_(53), B_(54), B_(55), B_(56), B_(57), B_(58), B_(59), B_(60), B_(61), B_(62), B_(63), B_(64), 
  B_(65), B_(66), B_(67), B_(68), B_(69), B_(70), B_(71), B_(72), B_(73), B_(74), B_(75), B_(76), B_(77), B_(78), B_(79), B_(80), 
  B_(81), B_(82), B_(83), B_(84), B_(85), B_(86), B_(87), B_(88), B_(89), B_(90), B_(91), B_(92), B_(93), B_(94), B_(95), B_(96), 
  B_(97), B_(98), B_(99), B_(100), B_(101), B_(102), B_(103), B_(104), B_(105), B_(106), B_(107), B_(108), B_(109), B_(110), B_(111), 
  B_(112), B_(113), B_(114), B_(115), B_(116), B_(117), B_(118), B_(119), B_(120), B_(121), B_(122), B_(123), B_(124), B_(125), 
  B_(126), B_(127), B_(-128), B_(-127), B_(-126), B_(-125), B_(-124), B_(-123), B_(-122), B_(-121), B_(-120), B_(-119), B_(-118), 
  B_(-117), B_(-116), B_(-115), B_(-114), B_(-113), B_(-112), B_(-111), B_(-110), B_(-109), B_(-108), B_(-107), B_(-106), B_(-105), 
  B_(-104), B_(-103), B_(-102), B_(-101), B_(-100), B_(-99), B_(-98), B_(-97), B_(-96), B_(-95), B_(-94), B_(-93), B_(-92), B_(-91), 
  B_(-90), B_(-89), B_(-88), B_(-87), B_(-86), B_(-85), B_(-84), B_(-83), B_(-82), B_(-81), B_(-80), B_(-79), B_(-78), B_(-77), 
  B_(-76), B_(-75), B_(-74), B_(-73), B_(-72), B_(-71), B_(-70), B_(-69), B_(-68), B_(-67), B_(-66), B_(-65), B_(-64), B_(-63), 
  B_(-62), B_(-61), B_(-60), B_(-59), B_(-58), B_(-57), B_(-56), B_(-55), B_(-54), B_(-53), B_(-52), B_(-51), B_(-50), B_(-49), 
  B_(-48), B_(-47), B_(-46), B_(-45), B_(-44), B_(-43), B_(-42), B_(-41), B_(-40), B_(-39), B_(-38), B_(-37), B_(-36), B_(-35), 
  B_(-34), B_(-33), B_(-32), B_(-31), B_(-30), B_(-29), B_(-28), B_(-27), B_(-26), B_(-25), B_(-24), B_(-23), B_(-22), B_(-21), 
  B_(-20), B_(-19), B_(-18), B_(-17), B_(-16), B_(-15), B_(-14), B_(-13), B_(-12), B_(-11), B_(-10), B_(-9), B_(-8), B_(-7), 
  B_(-6), B_(-5), B_(-4), B_(-3), B_(-2), B_(-1)
};


#define UB_(a) MUS_BYTE_TO_SAMPLE(a)
static const mus_float_t mus_ubyte[256] = {
  UB_(-128), UB_(-127), UB_(-126), UB_(-125), UB_(-124), UB_(-123), UB_(-122), UB_(-121), UB_(-120), UB_(-119), UB_(-118), UB_(-117), 
  UB_(-116), UB_(-115), UB_(-114), UB_(-113), UB_(-112), UB_(-111), UB_(-110), UB_(-109), UB_(-108), UB_(-107), UB_(-106), UB_(-105), 
  UB_(-104), UB_(-103), UB_(-102), UB_(-101), UB_(-100), UB_(-99), UB_(-98), UB_(-97), UB_(-96), UB_(-95), UB_(-94), UB_(-93), UB_(-92), 
  UB_(-91), UB_(-90), UB_(-89), UB_(-88), UB_(-87), UB_(-86), UB_(-85), UB_(-84), UB_(-83), UB_(-82), UB_(-81), UB_(-80), UB_(-79), 
  UB_(-78), UB_(-77), UB_(-76), UB_(-75), UB_(-74), UB_(-73), UB_(-72), UB_(-71), UB_(-70), UB_(-69), UB_(-68), UB_(-67), UB_(-66), 
  UB_(-65), UB_(-64), UB_(-63), UB_(-62), UB_(-61), UB_(-60), UB_(-59), UB_(-58), UB_(-57), UB_(-56), UB_(-55), UB_(-54), UB_(-53), 
  UB_(-52), UB_(-51), UB_(-50), UB_(-49), UB_(-48), UB_(-47), UB_(-46), UB_(-45), UB_(-44), UB_(-43), UB_(-42), UB_(-41), UB_(-40), 
  UB_(-39), UB_(-38), UB_(-37), UB_(-36), UB_(-35), UB_(-34), UB_(-33), UB_(-32), UB_(-31), UB_(-30), UB_(-29), UB_(-28), UB_(-27), 
  UB_(-26), UB_(-25), UB_(-24), UB_(-23), UB_(-22), UB_(-21), UB_(-20), UB_(-19), UB_(-18), UB_(-17), UB_(-16), UB_(-15), UB_(-14), 
  UB_(-13), UB_(-12), UB_(-11), UB_(-10), UB_(-9), UB_(-8), UB_(-7), UB_(-6), UB_(-5), UB_(-4), UB_(-3), UB_(-2), UB_(-1), UB_(0), 
  UB_(1), UB_(2), UB_(3), UB_(4), UB_(5), UB_(6), UB_(7), UB_(8), UB_(9), UB_(10), UB_(11), UB_(12), UB_(13), UB_(14), UB_(15), 
  UB_(16), UB_(17), UB_(18), UB_(19), UB_(20), UB_(21), UB_(22), UB_(23), UB_(24), UB_(25), UB_(26), UB_(27), UB_(28), UB_(29), 
  UB_(30), UB_(31), UB_(32), UB_(33), UB_(34), UB_(35), UB_(36), UB_(37), UB_(38), UB_(39), UB_(40), UB_(41), UB_(42), UB_(43), 
  UB_(44), UB_(45), UB_(46), UB_(47), UB_(48), UB_(49), UB_(50), UB_(51), UB_(52), UB_(53), UB_(54), UB_(55), UB_(56), UB_(57), 
  UB_(58), UB_(59), UB_(60), UB_(61), UB_(62), UB_(63), UB_(64), UB_(65), UB_(66), UB_(67), UB_(68), UB_(69), UB_(70), UB_(71), 
  UB_(72), UB_(73), UB_(74), UB_(75), UB_(76), UB_(77), UB_(78), UB_(79), UB_(80), UB_(81), UB_(82), UB_(83), UB_(84), UB_(85), 
  UB_(86), UB_(87), UB_(88), UB_(89), UB_(90), UB_(91), UB_(92), UB_(93), UB_(94), UB_(95), UB_(96), UB_(97), UB_(98), UB_(99), 
  UB_(100), UB_(101), UB_(102), UB_(103), UB_(104), UB_(105), UB_(106), UB_(107), UB_(108), UB_(109), UB_(110), UB_(111), UB_(112), 
  UB_(113), UB_(114), UB_(115), UB_(116), UB_(117), UB_(118), UB_(119), UB_(120), UB_(121), UB_(122), UB_(123), UB_(124), UB_(125), 
  UB_(126), UB_(127)
};



/* ---------------- read ---------------- */

#define BUFLIM (64 * 1024)
#define UBYTE_ZERO 128
#define USHORT_ZERO 32768

#define MUS_SAMPLE_UNSCALED(n) ((n) / 32768.0)
/* see note in _sndlib.h" values are "unscaled" from the DAC's point of view */

static mus_float_t *swapped_shorts = NULL;
static void initialize_swapped_shorts(void)
{
  int i;
  swapped_shorts = (mus_float_t *)malloc(65536 * sizeof(mus_float_t));
  for (i = 0; i < 65536; i++)
    {
      signed short x;
      x = (signed short)(((i >> 8) & 0xff) | ((i & 0xff) << 8));
      swapped_shorts[i] = MUS_SHORT_TO_SAMPLE(x);
    }
}


static mus_long_t mus_read_any_1(int tfd, mus_long_t beg, int chans, mus_long_t nints, mus_float_t **bufs, mus_float_t **cm, char *inbuf)
{
  int format, siz, siz_chans;
  mus_long_t bytes, lim, leftover, total_read, k, loc, oldloc, buflim;
  unsigned char *jchar;
  static char *ur_charbuf = NULL;
  char *charbuf = NULL;
  mus_float_t *buffer;

  if (nints <= 0) return(0);

  if (!inbuf)
    {
      io_fd *fd;

      if ((io_fds == NULL) || (tfd >= io_fd_size) || (tfd < 0) || (io_fds[tfd] == NULL))
	return(mus_error(MUS_FILE_DESCRIPTORS_NOT_INITIALIZED, "mus_read: no file descriptors!"));

      fd = io_fds[tfd];
      if (fd->data_format == MUS_UNKNOWN) 
	return(mus_error(MUS_FILE_CLOSED, "mus_read: invalid data format for %s", fd->name));

      format = fd->data_format;
      siz = fd->bytes_per_sample;
      if ((format == MUS_OUT_FORMAT) && 
	  (chans == 1) && 
	  (beg == 0))
	{
	  ssize_t total;

	  bytes = nints * siz;
	  total = read(tfd, (char *)(bufs[0]), bytes);
	  if (total != bytes)
	    {
	      if (total <= 0)
		memset((void *)(bufs[0]), 0, bytes);
	      else
		{
		  int i, last;
		  last = beg + nints;
		  for (i = total / siz; i < last; i++)
		    bufs[0][i] = 0.0;
		}
	    }
	  return(total / siz);
	}

      if (ur_charbuf == NULL) 
	ur_charbuf = (char *)malloc(BUFLIM * sizeof(char)); 
      charbuf = ur_charbuf;
    }
  else
    {
      charbuf = inbuf;
      siz = mus_bytes_per_sample(tfd);
      format = tfd;
    }

  siz_chans = siz * chans;
  leftover = (nints * siz_chans);
  k = (BUFLIM) % siz_chans;
  if (k != 0) /* for example, 3 channel output of 1-byte (mulaw) samples will need a mod 3 buffer */
    buflim = (BUFLIM) - k;
  else buflim = BUFLIM;
  total_read = 0;
  loc = beg;

#if MUS_LITTLE_ENDIAN
  if ((format == MUS_BSHORT) && (!swapped_shorts))
    initialize_swapped_shorts();
#else
  if ((format == MUS_LSHORT) && (!swapped_shorts))
    initialize_swapped_shorts();
#endif

  while (leftover > 0)
    {
      mus_long_t loclim;
      mus_float_t *bufnow, *bufend, *bufend4;
	      
      bytes = leftover;
      if (bytes > buflim) 
	{
	  leftover = (bytes - buflim); 
	  bytes = buflim;
	} 
      else leftover = 0;
      if (!inbuf)
	{
	  ssize_t total;

	  total = read(tfd, charbuf, bytes); 
	  if (total <= 0) 
	    {
	      /* zero out trailing section (some callers don't check the returned value) -- this added 9-May-99 */

	      lim = beg + nints;
	      if (loc < lim)
		for (k = 0; k < chans; k++)
		  if ((cm == NULL) || (cm[k]))
		    {
		      mus_float_t *p;
		      p = bufs[k];
		      memset((void *)(p + loc), 0, (lim - loc) * sizeof(mus_float_t));
		    }
	      return(total_read);
	    }
	  lim = (int) (total / siz_chans);  /* this divide must be exact (hence the buflim calc above) */
	}
      else
	{
	  lim = nints; /* frames in this case */
	  leftover = 0;
	}
      total_read += lim;
      oldloc = loc;

      if ((chans == 1) &&
	  ((!cm) || (cm[0])))
	{
	  buffer = (mus_float_t *)(bufs[0]);
	  if (buffer)
	    {
	      loc = oldloc;
	      loclim = loc + lim;
	      bufnow = (mus_float_t *)(buffer + loc);
	      bufend = (mus_float_t *)(buffer + loclim - 1);
	      bufend4 = (mus_float_t *)(bufend - 4);

	      jchar = (unsigned char *)charbuf;
	      switch (format)
		{
		case MUS_BSHORT:      
#if MUS_LITTLE_ENDIAN
		  while (bufnow <= bufend4)
		    {
		      (*bufnow++) = swapped_shorts[(*((unsigned short *)jchar))];
		      jchar += 2;
		      (*bufnow++) = swapped_shorts[(*((unsigned short *)jchar))];
		      jchar += 2;
		      (*bufnow++) = swapped_shorts[(*((unsigned short *)jchar))];
		      jchar += 2;
		      (*bufnow++) = swapped_shorts[(*((unsigned short *)jchar))];
		      jchar += 2;
		    }
		  for (; bufnow <= bufend; jchar += 2)                          /* unsigned short here as charbuf loc is slower */
		    (*bufnow++) = swapped_shorts[(*((unsigned short *)jchar))]; /* bswap16 is much slower here because of the subsequent short->double conversion */
#else       
		  for (; bufnow <= bufend; jchar += 2) 
		    (*bufnow++) = MUS_SHORT_TO_SAMPLE(big_endian_short(jchar)); 
#endif
		  break;
		  
		case MUS_LSHORT: 
#if (!MUS_LITTLE_ENDIAN)
		  for (; bufnow <= bufend; jchar += 2) 
		    (*bufnow++) = swapped_shorts[(*((unsigned short *)jchar))];
#else
		  while (bufnow <= bufend4)
		    {
		      (*bufnow++) = MUS_SHORT_TO_SAMPLE(little_endian_short(jchar)); 
		      jchar += 2;
		      (*bufnow++) = MUS_SHORT_TO_SAMPLE(little_endian_short(jchar)); 
		      jchar += 2;
		      (*bufnow++) = MUS_SHORT_TO_SAMPLE(little_endian_short(jchar)); 
		      jchar += 2;
		      (*bufnow++) = MUS_SHORT_TO_SAMPLE(little_endian_short(jchar)); 
		      jchar += 2;
		    }
		  for (; bufnow <= bufend; jchar += 2) 
		    (*bufnow++) = MUS_SHORT_TO_SAMPLE(little_endian_short(jchar)); 
#endif
		  break;
		  
		case MUS_BINT:    
		  while (bufnow <= bufend4)
		    {
		      (*bufnow++) = MUS_INT_TO_SAMPLE(big_endian_int(jchar)); 
		      jchar += 4;
		      (*bufnow++) = MUS_INT_TO_SAMPLE(big_endian_int(jchar)); 
		      jchar += 4;
		      (*bufnow++) = MUS_INT_TO_SAMPLE(big_endian_int(jchar)); 
		      jchar += 4;
		      (*bufnow++) = MUS_INT_TO_SAMPLE(big_endian_int(jchar)); 
		      jchar += 4;
		    }
		  for (; bufnow <= bufend; jchar += 4) 
		    (*bufnow++) = MUS_INT_TO_SAMPLE(big_endian_int(jchar)); 
		  break;
		  
		case MUS_LINT: 
		  while (bufnow <= bufend4)
		    {
		      (*bufnow++) = MUS_INT_TO_SAMPLE(little_endian_int(jchar)); 
		      jchar += 4;
		      (*bufnow++) = MUS_INT_TO_SAMPLE(little_endian_int(jchar)); 
		      jchar += 4;
		      (*bufnow++) = MUS_INT_TO_SAMPLE(little_endian_int(jchar)); 
		      jchar += 4;
		      (*bufnow++) = MUS_INT_TO_SAMPLE(little_endian_int(jchar)); 
		      jchar += 4;
		    }
		  for (; bufnow <= bufend; jchar += 4) 
		    (*bufnow++) = MUS_INT_TO_SAMPLE(little_endian_int(jchar)); 
		  break;
		  
		case MUS_BINTN:              
		  for (; bufnow <= bufend; jchar += 4) 
		    (*bufnow++) = MUS_INT_TO_SAMPLE((big_endian_int(jchar) >> 8));
		  break;
		  
		case MUS_LINTN: 
		  for (; bufnow <= bufend; jchar += 4) 
		    (*bufnow++) = MUS_INT_TO_SAMPLE((little_endian_int(jchar) >> 8));
		  break;
		  
		case MUS_MULAW:  	              
		  while (bufnow <= bufend4)
		    {
		      (*bufnow++) = mus_mulaw[*jchar++]; 
		      (*bufnow++) = mus_mulaw[*jchar++]; 
		      (*bufnow++) = mus_mulaw[*jchar++]; 
		      (*bufnow++) = mus_mulaw[*jchar++]; 
		    }
		  for (; bufnow <= bufend; jchar++) 
		    (*bufnow++) = mus_mulaw[*jchar]; 
		  break;
		  
		case MUS_ALAW:       
		  while (bufnow <= bufend4)
		    {
		      (*bufnow++) = mus_alaw[*jchar++]; 
		      (*bufnow++) = mus_alaw[*jchar++]; 
		      (*bufnow++) = mus_alaw[*jchar++]; 
		      (*bufnow++) = mus_alaw[*jchar++]; 
		    }
		  for (; bufnow <= bufend; jchar++) 
		    (*bufnow++) = mus_alaw[*jchar]; 
		  break;
		  
		case MUS_BYTE:                
		  while (bufnow <= bufend4)
		    {
		      (*bufnow++) = mus_byte[(unsigned char)(*jchar++)];
		      (*bufnow++) = mus_byte[(unsigned char)(*jchar++)];
		      (*bufnow++) = mus_byte[(unsigned char)(*jchar++)];
		      (*bufnow++) = mus_byte[(unsigned char)(*jchar++)];
		    }
		  for (; bufnow <= bufend; jchar++)
		    (*bufnow++) = mus_byte[(unsigned char)(*jchar)];
		  break;
		  
		case MUS_UBYTE:     	      
		  while (bufnow <= bufend4)
		    {
		      (*bufnow++) = mus_ubyte[(unsigned char)(*jchar++)];
		      (*bufnow++) = mus_ubyte[(unsigned char)(*jchar++)];
		      (*bufnow++) = mus_ubyte[(unsigned char)(*jchar++)];
		      (*bufnow++) = mus_ubyte[(unsigned char)(*jchar++)];
		    }
		  for (; bufnow <= bufend; jchar++) 
		    (*bufnow++) = mus_ubyte[(unsigned char)(*jchar)];
		  break;
		  
		case MUS_BFLOAT:
		  while (bufnow <= bufend4)
		    {
		      (*bufnow++) = (mus_float_t)(big_endian_float(jchar));
		      jchar += 4;
		      (*bufnow++) = (mus_float_t)(big_endian_float(jchar));
		      jchar += 4;
		      (*bufnow++) = (mus_float_t)(big_endian_float(jchar));
		      jchar += 4;
		      (*bufnow++) = (mus_float_t)(big_endian_float(jchar));
		      jchar += 4;
		    }
		  for (; bufnow <= bufend; jchar += 4) 
		    (*bufnow++) = (mus_float_t)(big_endian_float(jchar));
		  break;
		  
		case MUS_BFLOAT_UNSCALED:
		  for (; bufnow <= bufend; jchar += 4) 
		    (*bufnow++) = (mus_float_t)(MUS_SAMPLE_UNSCALED(big_endian_float(jchar)));
		  break;
		  
		case MUS_BDOUBLE:   
		  while (bufnow <= bufend4)
		    {
		      (*bufnow++) = (mus_float_t)(big_endian_double(jchar));
		      jchar += 8;
		      (*bufnow++) = (mus_float_t)(big_endian_double(jchar));
		      jchar += 8;
		      (*bufnow++) = (mus_float_t)(big_endian_double(jchar));
		      jchar += 8;
		      (*bufnow++) = (mus_float_t)(big_endian_double(jchar));
		      jchar += 8;
		    }
		  for (; bufnow <= bufend; jchar += 8)
		    (*bufnow++) = (mus_float_t)(big_endian_double(jchar));
		  break;
		  
		case MUS_BDOUBLE_UNSCALED:   
		  for (; bufnow <= bufend; jchar += 8)
		    (*bufnow++) = (mus_float_t)(MUS_SAMPLE_UNSCALED(big_endian_double(jchar)));
		  break;
		  
		case MUS_LFLOAT:
		  while (bufnow <= bufend4)
		    {
		      (*bufnow++) = (mus_float_t)(little_endian_float(jchar));
		      jchar += 4;
		      (*bufnow++) = (mus_float_t)(little_endian_float(jchar));
		      jchar += 4;
		      (*bufnow++) = (mus_float_t)(little_endian_float(jchar));
		      jchar += 4;
		      (*bufnow++) = (mus_float_t)(little_endian_float(jchar));
		      jchar += 4;
		    }
		  for (; bufnow <= bufend; jchar += 4) 
		    (*bufnow++) = (mus_float_t)(little_endian_float(jchar));
		  break;
		  
		case MUS_LFLOAT_UNSCALED:    
		  for (; bufnow <= bufend; jchar += 4) 
		    (*bufnow++) = (mus_float_t)(MUS_SAMPLE_UNSCALED(little_endian_float(jchar)));
		  break;
		  
		case MUS_LDOUBLE:   
		  for (; bufnow <= bufend; jchar += 8) 
		    (*bufnow++) = (mus_float_t)(little_endian_double(jchar));
		  break;
		  
		case MUS_LDOUBLE_UNSCALED:   
		  for (; bufnow <= bufend; jchar += 8) 
		    (*bufnow++) = (mus_float_t)(MUS_SAMPLE_UNSCALED(little_endian_double(jchar)));
		  break;
		  
		case MUS_UBSHORT:   
		  while (bufnow <= bufend4)
		    {
		      (*bufnow++) = MUS_SHORT_TO_SAMPLE((int)(big_endian_unsigned_short(jchar)) - USHORT_ZERO);
		      jchar += 2;
		      (*bufnow++) = MUS_SHORT_TO_SAMPLE((int)(big_endian_unsigned_short(jchar)) - USHORT_ZERO);
		      jchar += 2;
		      (*bufnow++) = MUS_SHORT_TO_SAMPLE((int)(big_endian_unsigned_short(jchar)) - USHORT_ZERO);
		      jchar += 2;
		      (*bufnow++) = MUS_SHORT_TO_SAMPLE((int)(big_endian_unsigned_short(jchar)) - USHORT_ZERO);
		      jchar += 2;
		    }
		  for (; bufnow <= bufend; jchar += 2) 
		    (*bufnow++) = MUS_SHORT_TO_SAMPLE((int)(big_endian_unsigned_short(jchar)) - USHORT_ZERO);
		  break;
		  
		case MUS_ULSHORT:   
		  while (bufnow <= bufend4)
		    {
		      (*bufnow++) = MUS_SHORT_TO_SAMPLE((int)(little_endian_unsigned_short(jchar)) - USHORT_ZERO);
		      jchar += 2;
		      (*bufnow++) = MUS_SHORT_TO_SAMPLE((int)(little_endian_unsigned_short(jchar)) - USHORT_ZERO);
		      jchar += 2;
		      (*bufnow++) = MUS_SHORT_TO_SAMPLE((int)(little_endian_unsigned_short(jchar)) - USHORT_ZERO);
		      jchar += 2;
		      (*bufnow++) = MUS_SHORT_TO_SAMPLE((int)(little_endian_unsigned_short(jchar)) - USHORT_ZERO);
		      jchar += 2;
		    }
		  for (; bufnow <= bufend; jchar += 2) 
		    (*bufnow++) = MUS_SHORT_TO_SAMPLE((int)(little_endian_unsigned_short(jchar)) - USHORT_ZERO);
		  break;
		  
		case MUS_B24INT:
		  while (bufnow <= bufend4)
		    {
		      (*bufnow++) = MUS_INT24_TO_SAMPLE((int)(((jchar[0] << 24) | (jchar[1] << 16) | (jchar[2] << 8)) >> 8));
		      jchar += 3;
		      (*bufnow++) = MUS_INT24_TO_SAMPLE((int)(((jchar[0] << 24) | (jchar[1] << 16) | (jchar[2] << 8)) >> 8));
		      jchar += 3;
		      (*bufnow++) = MUS_INT24_TO_SAMPLE((int)(((jchar[0] << 24) | (jchar[1] << 16) | (jchar[2] << 8)) >> 8));
		      jchar += 3;
		      (*bufnow++) = MUS_INT24_TO_SAMPLE((int)(((jchar[0] << 24) | (jchar[1] << 16) | (jchar[2] << 8)) >> 8));
		      jchar += 3;
		    }
		  for (; bufnow <= bufend; jchar += 3) 
		    (*bufnow++) = MUS_INT24_TO_SAMPLE((int)(((jchar[0] << 24) | (jchar[1] << 16) | (jchar[2] << 8)) >> 8));
		  break;
		  
		case MUS_L24INT:  
		  while (bufnow <= bufend4)
		    {
		      (*bufnow++) = MUS_INT24_TO_SAMPLE((int)(((jchar[2] << 24) | (jchar[1] << 16) | (jchar[0] << 8)) >> 8));
		      jchar += 3;
		      (*bufnow++) = MUS_INT24_TO_SAMPLE((int)(((jchar[2] << 24) | (jchar[1] << 16) | (jchar[0] << 8)) >> 8));
		      jchar += 3;
		      (*bufnow++) = MUS_INT24_TO_SAMPLE((int)(((jchar[2] << 24) | (jchar[1] << 16) | (jchar[0] << 8)) >> 8));
		      jchar += 3;
		      (*bufnow++) = MUS_INT24_TO_SAMPLE((int)(((jchar[2] << 24) | (jchar[1] << 16) | (jchar[0] << 8)) >> 8));
		      jchar += 3;
		    }
		  for (; bufnow <= bufend; jchar += 3) 
		    (*bufnow++) = MUS_INT24_TO_SAMPLE((int)(((jchar[2] << 24) | (jchar[1] << 16) | (jchar[0] << 8)) >> 8));
		  break;
		}
	      loc = loclim;
	    }
	}
      else
	{
	  for (k = 0; k < chans; k++)
	    {
	      if ((cm == NULL) || (cm[k]))
		{
		  buffer = (mus_float_t *)(bufs[k]);
		  if (buffer)
		    {
		      loc = oldloc;
		      loclim = loc + lim;
		      bufnow = (mus_float_t *)(buffer + loc);
		      bufend = (mus_float_t *)(buffer + loclim - 1);
		      bufend4 = (mus_float_t *)(bufend - 4);
		      
		      jchar = (unsigned char *)charbuf;
		      jchar += (k * siz);
		      switch (format)
			{
			case MUS_BSHORT:      
#if MUS_LITTLE_ENDIAN
			  while (bufnow <= bufend4) 
			    {
			      (*bufnow++) = swapped_shorts[(*((unsigned short *)jchar))];
			      jchar += siz_chans;
			      (*bufnow++) = swapped_shorts[(*((unsigned short *)jchar))];
			      jchar += siz_chans;
			      (*bufnow++) = swapped_shorts[(*((unsigned short *)jchar))];
			      jchar += siz_chans;
			      (*bufnow++) = swapped_shorts[(*((unsigned short *)jchar))];
			      jchar += siz_chans;
			    }
			  for (; bufnow <= bufend; jchar += siz_chans) 
			    (*bufnow++) = swapped_shorts[(*((unsigned short *)jchar))];
#else       
			  for (; bufnow <= bufend; jchar += siz_chans) 
			    (*bufnow++) = MUS_SHORT_TO_SAMPLE(big_endian_short(jchar)); 
#endif
			  break;
			  
			case MUS_LSHORT: 
#if (!MUS_LITTLE_ENDIAN)
			  for (; bufnow <= bufend; jchar += siz_chans) 
			    (*bufnow++) = swapped_shorts[(*((unsigned short *)jchar))];
#else
			  while (bufnow <= bufend4) 
			    {
			      (*bufnow++) = MUS_SHORT_TO_SAMPLE(little_endian_short(jchar)); 
			      jchar += siz_chans;
			      (*bufnow++) = MUS_SHORT_TO_SAMPLE(little_endian_short(jchar)); 
			      jchar += siz_chans;
			      (*bufnow++) = MUS_SHORT_TO_SAMPLE(little_endian_short(jchar)); 
			      jchar += siz_chans;
			      (*bufnow++) = MUS_SHORT_TO_SAMPLE(little_endian_short(jchar)); 
			      jchar += siz_chans;
			    }
			  for (; bufnow <= bufend; jchar += siz_chans) 
			    (*bufnow++) = MUS_SHORT_TO_SAMPLE(little_endian_short(jchar)); 
#endif
			  break;
			  
			case MUS_BINT:              
			  for (; bufnow <= bufend; jchar += siz_chans) 
			    (*bufnow++) = MUS_INT_TO_SAMPLE(big_endian_int(jchar)); 
			  break;
			  
			case MUS_LINT: 
			  for (; bufnow <= bufend; jchar += siz_chans) 
			    (*bufnow++) = MUS_INT_TO_SAMPLE(little_endian_int(jchar)); 
			  break;
			  
			case MUS_BINTN:              
			  for (; bufnow <= bufend; jchar += siz_chans) 
			    (*bufnow++) = MUS_INT_TO_SAMPLE((big_endian_int(jchar) >> 8));
			  break;
			  
			case MUS_LINTN: 
			  for (; bufnow <= bufend; jchar += siz_chans) 
			    (*bufnow++) = MUS_INT_TO_SAMPLE((little_endian_int(jchar) >> 8));
			  break;
			  
			case MUS_MULAW:  	              
			  for (; bufnow <= bufend; jchar += siz_chans) 
			    (*bufnow++) = mus_mulaw[*jchar]; 
			  break;
			  
			case MUS_ALAW:                  
			  for (; bufnow <= bufend; jchar += siz_chans) 
			    (*bufnow++) = mus_alaw[*jchar]; 
			  break;
			  
			case MUS_BYTE:                
			  for (; bufnow <= bufend; jchar += siz_chans)
			    (*bufnow++) = mus_byte[(unsigned char)(*jchar)];
			  break;
			  
			case MUS_UBYTE:     	      
			  for (; bufnow <= bufend; jchar += siz_chans) 
			    (*bufnow++) = mus_ubyte[(unsigned char)(*jchar)];
			  break;
			  
			case MUS_BFLOAT:
			  for (; bufnow <= bufend; jchar += siz_chans) 
			    (*bufnow++) = (mus_float_t)(big_endian_float(jchar));
			  break;
			  
			case MUS_BFLOAT_UNSCALED:
			  for (; bufnow <= bufend; jchar += siz_chans) 
			    (*bufnow++) = (mus_float_t)(MUS_SAMPLE_UNSCALED(big_endian_float(jchar)));
			  break;
			  
			case MUS_BDOUBLE:   
			  for (; bufnow <= bufend; jchar += siz_chans)
			    (*bufnow++) = (mus_float_t)(big_endian_double(jchar));
			  break;
			  
			case MUS_BDOUBLE_UNSCALED:   
			  for (; bufnow <= bufend; jchar += siz_chans)
			    (*bufnow++) = (mus_float_t)(MUS_SAMPLE_UNSCALED(big_endian_double(jchar)));
			  break;
			  
			case MUS_LFLOAT:
			  for (; bufnow <= bufend; jchar += siz_chans) 
			    (*bufnow++) = (mus_float_t)(little_endian_float(jchar));
			  break;
			  
			case MUS_LFLOAT_UNSCALED:    
			  for (; bufnow <= bufend; jchar += siz_chans) 
			    (*bufnow++) = (mus_float_t)(MUS_SAMPLE_UNSCALED(little_endian_float(jchar)));
			  break;
			  
			case MUS_LDOUBLE:   
			  for (; bufnow <= bufend; jchar += siz_chans) 
			    (*bufnow++) = (mus_float_t)(little_endian_double(jchar));
			  break;
			  
			case MUS_LDOUBLE_UNSCALED:   
			  for (; bufnow <= bufend; jchar += siz_chans) 
			    (*bufnow++) = (mus_float_t)(MUS_SAMPLE_UNSCALED(little_endian_double(jchar)));
			  break;
			  
			case MUS_UBSHORT:   
			  for (; bufnow <= bufend; jchar += siz_chans) 
			    (*bufnow++) = MUS_SHORT_TO_SAMPLE((int)(big_endian_unsigned_short(jchar)) - USHORT_ZERO);
			  break;
			  
			case MUS_ULSHORT:   
			  for (; bufnow <= bufend; jchar += siz_chans) 
			    (*bufnow++) = MUS_SHORT_TO_SAMPLE((int)(little_endian_unsigned_short(jchar)) - USHORT_ZERO);
			  break;
			  
			case MUS_B24INT:
			  for (; bufnow <= bufend; jchar += siz_chans) 
			    (*bufnow++) = MUS_INT24_TO_SAMPLE((int)(((jchar[0] << 24) | (jchar[1] << 16) | (jchar[2] << 8)) >> 8));
			  break;
			  
			case MUS_L24INT:   
			  for (; bufnow <= bufend; jchar += siz_chans) 
			    (*bufnow++) = MUS_INT24_TO_SAMPLE((int)(((jchar[2] << 24) | (jchar[1] << 16) | (jchar[0] << 8)) >> 8));
			  break;
			}
		      loc = loclim;
		    }
		}
	    }
	}
    }
  return(total_read);
}


mus_long_t mus_file_read_any(int tfd, mus_long_t beg, int chans, mus_long_t nints, mus_float_t **bufs, mus_float_t **cm)
{
  return(mus_read_any_1(tfd, beg, chans, nints, bufs, cm, NULL));
}


mus_long_t mus_file_read_file(int tfd, mus_long_t beg, int chans, mus_long_t nints, mus_float_t **bufs)
{
  return(mus_read_any_1(tfd, beg, chans, nints, bufs, NULL, NULL));
}


mus_long_t mus_file_read_buffer(int charbuf_data_format, mus_long_t beg, int chans, mus_long_t nints, mus_float_t **bufs, char *charbuf)
{
  return(mus_read_any_1(charbuf_data_format, beg, chans, nints, bufs, NULL, charbuf)); 
}


mus_long_t mus_file_read(int tfd, mus_long_t beg, mus_long_t end, int chans, mus_float_t **bufs)
{
  mus_long_t num, rtn, k;
  num = (end - beg + 1);
  rtn = mus_read_any_1(tfd, beg, chans, num, bufs, NULL, NULL);
  if (rtn == MUS_ERROR) return(MUS_ERROR);
  if (rtn < num) 
    /* this zeroing can be fooled if the file is chunked and has trailing, non-data chunks */
    for (k = 0; k < chans; k++)
      {
	mus_long_t i;
	mus_float_t *buffer;
	buffer = bufs[k];
	i = rtn + beg;
	/* this happens routinely in mus_outa + initial write (reads ahead in effect) */
	memset((void *)(buffer + i), 0, (end - i + 1) * sizeof(mus_float_t));
      }
  return(num);
}


mus_long_t mus_file_read_chans(int tfd, mus_long_t beg, mus_long_t end, int chans, mus_float_t **bufs, mus_float_t **cm)
{
  /* an optimization of mus_file_read -- just reads the desired channels */
  mus_long_t num, rtn, k;

  num = (end - beg + 1);
  rtn = mus_read_any_1(tfd, beg, chans, num, bufs, cm, NULL);
  if (rtn == MUS_ERROR) return(MUS_ERROR);

  if (rtn < num) 
    for (k = 0; k < chans; k++)
      if ((cm == NULL) || (cm[k]))
	{
	  mus_long_t i;
	  mus_float_t *buffer;
	  buffer = bufs[k];
	  i = rtn + beg;
	  memset((void *)(buffer + i), 0, (end - i + 1) * sizeof(mus_float_t));
	}

  return(num);
}


/* ---------------- write ---------------- */

static int checked_write(int tfd, char *buf, mus_long_t chars)
{
  long long int bytes;
  bytes = (long long int)write(tfd, buf, chars);
  if (bytes != chars) 
    {
      io_fd *fd;
      if ((io_fds == NULL) || (tfd >= io_fd_size) || (tfd < 0) || (io_fds[tfd] == NULL))
	return(mus_error(MUS_FILE_DESCRIPTORS_NOT_INITIALIZED, "mus_write: no file descriptors!"));
      fd = io_fds[tfd];
      if (fd->data_format == MUS_UNKNOWN) 
	return(mus_error(MUS_FILE_CLOSED,
			 "attempt to write closed file %s",
			 fd->name));
      else
	return(mus_error(MUS_WRITE_ERROR,
			 "mus_write: write error for %s%s%s: only %lld of %lld" " bytes written",
			 fd->name, (errno) ? ": " : "", (errno) ? STRERROR(errno) : "",
			 bytes, chars));
    }
  return(MUS_NO_ERROR);
}


static mus_clip_handler_t *mus_clip_handler = NULL;
static bool (*clip_checker)(void) = NULL;

mus_clip_handler_t *mus_clip_set_handler(mus_clip_handler_t *new_clip_handler)
{
  mus_clip_handler_t *old_handler;
  old_handler = mus_clip_handler;
  mus_clip_handler = new_clip_handler;
  return(old_handler);
}

mus_clip_handler_t *mus_clip_set_handler_and_checker(mus_clip_handler_t *new_clip_handler, bool (*checker)(void))
{
  mus_clip_handler_t *old_handler;
  old_handler = mus_clip_handler;
  mus_clip_handler = new_clip_handler;
  clip_checker = checker;
  return(old_handler);
}


static int mus_write_1(int tfd, mus_long_t beg, mus_long_t end, int chans, mus_float_t **bufs, char *inbuf, bool clipped)
{
  int err, siz, siz_chans, data_format, val;
  mus_long_t bytes, k, lim, leftover, loc, oldloc, buflim;
  bool clipping = false;
  unsigned char *jchar;
  static char *ur_charbuf = NULL;
  char *charbuf = NULL;
  mus_float_t *buffer;

  if (chans <= 0) return(MUS_NO_ERROR); /* ?? */

  if (!inbuf)
    {
      io_fd *fd;
      if ((io_fds == NULL) || 
	  (tfd >= io_fd_size) || 
	  (tfd < 0) || 
	  (io_fds[tfd] == NULL))
	return(mus_error(MUS_FILE_DESCRIPTORS_NOT_INITIALIZED, "mus_write: no file descriptors!"));

      fd = io_fds[tfd];
      if (fd->data_format == MUS_UNKNOWN) 
	return(mus_error(MUS_FILE_CLOSED, "mus_write: invalid data format for %s", fd->name));

      siz = fd->bytes_per_sample;
      data_format = fd->data_format;
      clipping = fd->clipping;

      if ((data_format == MUS_OUT_FORMAT) && 
	  (chans == 1) && 
	  (!clipping) && 
	  (beg == 0))
	{
	  bytes = (end + 1) * siz;
	  return(checked_write(tfd, (char *)(bufs[0]), bytes));
	}
    }
  else
    {
      siz = mus_bytes_per_sample(tfd);
      data_format = tfd; /* in this case, tfd is the data format (see mus_file_write_buffer below) -- this should be changed! */
      clipping = clipped;
    }

  lim = (end - beg + 1);
  siz_chans = siz * chans;
  leftover = lim * siz_chans;
  k = (BUFLIM) % siz_chans;
  if (k != 0) 
    buflim = (BUFLIM) - k;
  else buflim = BUFLIM;
  loc = beg;

  if (inbuf)
    charbuf = inbuf;

  while (leftover > 0)
    {
      bytes = leftover;
      if (bytes > buflim) 
	{
	  leftover = (bytes - buflim); 
	  bytes = buflim;
	} 
      else leftover = 0;
      lim = (int)(bytes / siz_chans); /* see note above */
      oldloc = loc;

      for (k = 0; k < chans; k++)
	{
	  mus_long_t loclim;
	  mus_float_t *bufnow, *bufend, *bufend4;

	  if (bufs[k] == NULL) continue;
	  loc = oldloc;
	  buffer = (mus_float_t *)(bufs[k]);

	  if (clipping)
	    {
	      int clipend;
	      mus_float_t sample;

	      clipend = oldloc + lim;
	      bufnow = (mus_float_t *)(buffer + oldloc);
	      bufend = (mus_float_t *)(buffer + clipend - 1);
	      bufend4 = (mus_float_t *)(bufend - 4); 
	      /* we seem to be running off the end of this buffer somehow -- do we need channel-specific size checks? */

	      if (clip_checker) clip_checker();
	      if (mus_clip_handler)
		{
		  while (bufnow < bufend)
		    {
		      sample = (*bufnow);
		      if ((sample > 0.99999) || (sample < -1.0))
			(*bufnow) = (*mus_clip_handler)(sample);
		      bufnow++;
		      
		      sample = (*bufnow);
		      if ((sample > 0.99999) || (sample < -1.0))
			(*bufnow) = (*mus_clip_handler)(sample);
		      bufnow++;
		    }
		  for (; bufnow <= bufend; bufnow++)
		    {
		      sample = (*bufnow);
		      if ((sample > 0.99999) ||
			  (sample < -1.0))
			(*bufnow) = (*mus_clip_handler)(sample);
		    }
		}
	      else
		{
		  for (; bufnow <= bufend; bufnow++)
		    {
		      sample = (*bufnow);
		      if (sample > 0.99999)
			(*bufnow) = 0.99999;
		      else
			if (sample < -1.0)
			  (*bufnow) = -1.0;
		    }
		}
	    }

	  if ((data_format == MUS_OUT_FORMAT) && 
	      (chans == 1) && 
	      (beg == 0) &&
	      (!inbuf)) /* "tfd" can be data format */
	    {
	      bytes = (end + 1) * siz;
	      return(checked_write(tfd, (char *)(bufs[0]), bytes));
	    }

	  loclim = loc + lim;
	  if (!charbuf)
	    {
	      if (ur_charbuf == NULL)
		ur_charbuf = (char *)malloc(BUFLIM * sizeof(char)); 
	      /*
		ur_charbuf = (char *)calloc(BUFLIM, sizeof(char)); 
	      else memset((void *)ur_charbuf, 0, BUFLIM);
	      */
	      charbuf = ur_charbuf;
	    }

	  bufnow = (mus_float_t *)(buffer + loc);
	  bufend = (mus_float_t *)(buffer + loclim - 1);
	  bufend4 = (mus_float_t *)(bufend - 4);

	  jchar = (unsigned char *)charbuf; /* if to_buffer we should add the loop offset here, or never loop */
	  jchar += (k * siz); 
	  switch (data_format)
	    {
	    case MUS_BSHORT: 
	      while (bufnow <= bufend4)
		{
		  set_big_endian_short(jchar, MUS_SAMPLE_TO_SHORT(*bufnow++));
		  jchar += siz_chans;
		  set_big_endian_short(jchar, MUS_SAMPLE_TO_SHORT(*bufnow++));
		  jchar += siz_chans;
		  set_big_endian_short(jchar, MUS_SAMPLE_TO_SHORT(*bufnow++));
		  jchar += siz_chans;
		  set_big_endian_short(jchar, MUS_SAMPLE_TO_SHORT(*bufnow++));
		  jchar += siz_chans;
		}
	      for (; bufnow <= bufend; jchar += siz_chans) 
		set_big_endian_short(jchar, MUS_SAMPLE_TO_SHORT(*bufnow++));
	      break;

	    case MUS_LSHORT:   
	      for (; bufnow <= bufend; jchar += siz_chans) 
		set_little_endian_short(jchar, MUS_SAMPLE_TO_SHORT(*bufnow++));
	      break;

	    case MUS_BINT:   
	      for (; bufnow <= bufend; jchar += siz_chans) 
		set_big_endian_int(jchar, MUS_SAMPLE_TO_INT(*bufnow++));
	      break;

	    case MUS_LINT:   
	      for (; bufnow <= bufend; jchar += siz_chans) 
		set_little_endian_int(jchar, MUS_SAMPLE_TO_INT(*bufnow++));
	      break;

	    case MUS_BINTN:   
	      for (; bufnow <= bufend; jchar += siz_chans) 
		set_big_endian_int(jchar, MUS_SAMPLE_TO_INT(*bufnow++) << 8);
	      break;

	    case MUS_LINTN:   
	      for (; bufnow <= bufend; jchar += siz_chans) 
		set_little_endian_int(jchar, MUS_SAMPLE_TO_INT(*bufnow++) << 8);
	      break;

	    case MUS_MULAW:     
	      for (; bufnow <= bufend; jchar += siz_chans) 
		(*jchar) = to_mulaw(MUS_SAMPLE_TO_SHORT(*bufnow++));
	      break;

	    case MUS_ALAW:      
	      for (; bufnow <= bufend; jchar += siz_chans) 
		(*jchar) = to_alaw(MUS_SAMPLE_TO_SHORT(*bufnow++));
	      break;

	    case MUS_BYTE:    
	      for (; bufnow <= bufend; jchar += siz_chans) 
		(*((signed char *)jchar)) = MUS_SAMPLE_TO_BYTE(*bufnow++);
	      break;

	    case MUS_UBYTE:  
	      for (; bufnow <= bufend; jchar += siz_chans) 
		(*jchar) = MUS_SAMPLE_TO_BYTE(*bufnow++) + UBYTE_ZERO;
	      break;

	    case MUS_BFLOAT:    
	      for (; bufnow <= bufend; jchar += siz_chans) 
		set_big_endian_float(jchar, *bufnow++);
	      break;

	    case MUS_LFLOAT:    
	      while (bufnow <= bufend4)
		{
		  set_little_endian_float(jchar, *bufnow++);
		  jchar += siz_chans;
		  set_little_endian_float(jchar, *bufnow++);
		  jchar += siz_chans;
		  set_little_endian_float(jchar, *bufnow++);
		  jchar += siz_chans;
		  set_little_endian_float(jchar, *bufnow++);
		  jchar += siz_chans;
		}
	      for (; bufnow <= bufend; jchar += siz_chans) 
		set_little_endian_float(jchar, *bufnow++);
	      break;

	    case MUS_BDOUBLE:
	      for (; bufnow <= bufend; jchar += siz_chans) 
		set_big_endian_double(jchar, *bufnow++);
	      break;

	    case MUS_LDOUBLE:   
	      while (bufnow <= bufend4)
		{
		  set_little_endian_double(jchar, *bufnow++);
		  jchar += siz_chans;
		  set_little_endian_double(jchar, *bufnow++);
		  jchar += siz_chans;
		  set_little_endian_double(jchar, *bufnow++);
		  jchar += siz_chans;
		  set_little_endian_double(jchar, *bufnow++);
		  jchar += siz_chans;
		}
	      for (; bufnow <= bufend; jchar += siz_chans) 
		set_little_endian_double(jchar, *bufnow++);
	      break;

	    case MUS_BFLOAT_UNSCALED:    
	      for (; bufnow <= bufend; jchar += siz_chans) 
		set_big_endian_float(jchar, 32768.0 * (*bufnow++));
	      break;

	    case MUS_LFLOAT_UNSCALED:    
	      for (; bufnow <= bufend; jchar += siz_chans) 
		set_little_endian_float(jchar, 32768.0 * (*bufnow++));
	      break;

	    case MUS_BDOUBLE_UNSCALED:
	      for (; bufnow <= bufend; jchar += siz_chans) 
		set_big_endian_double(jchar, 32768.0 * (*bufnow++));
	      break;

	    case MUS_LDOUBLE_UNSCALED:   
	      for (; bufnow <= bufend; jchar += siz_chans) 
		set_little_endian_double(jchar, 32768.0 * (*bufnow++));
	      break;

	    case MUS_UBSHORT: 
	      for (; bufnow <= bufend; jchar += siz_chans) 
		set_big_endian_unsigned_short(jchar, (unsigned short)(MUS_SAMPLE_TO_SHORT(*bufnow++) + USHORT_ZERO));
	      break;

	    case MUS_ULSHORT: 
	      for (; bufnow <= bufend; jchar += siz_chans) 
		set_little_endian_unsigned_short(jchar, (unsigned short)(MUS_SAMPLE_TO_SHORT(*bufnow++) + USHORT_ZERO));
	      break;

	    case MUS_B24INT: 
	      {
		int c3;
		mus_long_t bk;
		bk = (k * 3);
		c3 = chans * 3;
		for (; bufnow <= bufend; bk += c3) 
		  {
		    val = MUS_SAMPLE_TO_INT24(*bufnow++);
		    charbuf[bk] = (val >> 16); 
		    charbuf[bk + 1] = (val >> 8); 
		    charbuf[bk + 2] = (val & 0xFF); 
		  }
	      }
	      break;

	    case MUS_L24INT:   
	      {
		int c3;
		mus_long_t bk;
		bk = (k * 3);
		c3 = chans * 3;
		for (; bufnow <= bufend; bk += c3)
		  {
		    val = MUS_SAMPLE_TO_INT24(*bufnow++);
		    charbuf[bk + 2] = (val >> 16); 
		    charbuf[bk + 1] = (val >> 8); 
		    charbuf[bk] = (val & 0xFF); 
		  }
	      }
	      break;
	    }
	  loc = loclim;
	}
      if (!inbuf)
	{
	  err = checked_write(tfd, charbuf, bytes);
	  if (err == MUS_ERROR) 
	    return(MUS_ERROR);
	}
    }
  return(MUS_NO_ERROR);
}


int mus_file_write(int tfd, mus_long_t beg, mus_long_t end, int chans, mus_float_t **bufs)
{
  return(mus_write_1(tfd, beg, end, chans, bufs, NULL, false)); /* if inbuf is NULL (as here), clipped is ignored, so the "false" could be anything */
}


int mus_file_write_file(int tfd, mus_long_t beg, mus_long_t end, int chans, mus_float_t **bufs)
{
  return(mus_write_1(tfd, beg, end, chans, bufs, NULL, false));
}


int mus_file_write_buffer(int charbuf_data_format, mus_long_t beg, mus_long_t end, int chans, mus_float_t **bufs, char *charbuf, bool clipped)
{
  return(mus_write_1(charbuf_data_format, beg, end, chans, bufs, charbuf, clipped));
}


/* for CLM */
void mus_reset_io_c(void) 
{
  io_fd_size = 0;
  io_fds = NULL;
  clipping_default = false;
  mus_clip_set_handler(NULL);
}


#if !(defined(_MSC_VER) && (!(defined(__CYGWIN__))))
static int sndlib_strlen(const char *str)
{
  /* strlen(NULL) -> seg fault! */
  if ((str) && (*str)) return(strlen(str));
  return(0);
}
#endif


static char *saved_cwd = NULL;
char *mus_getcwd(void)
{
  int i, path_max = 0;
  char *pwd = NULL, *res = NULL;
  if (saved_cwd) return(saved_cwd);
#ifndef _MSC_VER
  path_max = pathconf("/", _PC_PATH_MAX);
#endif
  if (path_max < 1024)
    {
#if defined(PATH_MAX)
      path_max = PATH_MAX;
#endif
      if (path_max < 1024) 
	path_max = 1024;
    }
  for (i = path_max;; i *= 2)
    {
      if (pwd) free(pwd);
      pwd = (char *)calloc(i, sizeof(char));
#if (defined(_MSC_VER) || __CYGWIN__)
      res = _getcwd(pwd, i);
#else
      res = getcwd(pwd, i);
#endif
      if (res) break;    /* NULL is returned if failure, but what about success? should I check errno=ERANGE? */
    }
  saved_cwd = pwd;
  return(pwd);
}


char *mus_expand_filename(const char *filename)
{
  /* fill out under-specified library pathnames etc */
#if defined(_MSC_VER) && (!(defined(__CYGWIN__)))
  return(mus_strdup(filename));
#else
  char *file_name_buf = NULL;
  char *tok = NULL, *orig = NULL;
  int i, j = 0, len = 0;

  if ((filename) && (*filename)) 
    len = strlen(filename); 
  else return(NULL);
  if (len == 0) return(NULL);

  orig = mus_strdup(filename);
  tok = orig;
  /* get rid of "//" */
  for (i = 0; i < len - 1; i++)
    {
      if ((tok[i] == '/') && 
	  (tok[i + 1] == '/')) 
	j = i + 1;
    }
  if (j > 0)
    {
      for (i = 0; j < len; i++, j++) 
	tok[i] = tok[j];
      tok[i] ='\0';
    }
  /* get rid of "~/" at start */
  if (tok[0] != '/')
    {
      char *home = NULL;
      if ((tok[0] == '~') && (home = getenv("HOME")))
	{
	  file_name_buf = (char *)calloc(len + sndlib_strlen(home) + 8, sizeof(char));
	  strcpy(file_name_buf, home);
	  strcat(file_name_buf, ++tok);
	}
      else
	{
	  char *pwd;
	  pwd = mus_getcwd();
	  file_name_buf = (char *)calloc(len + sndlib_strlen(pwd) + 8, sizeof(char));
	  strcpy(file_name_buf, pwd);
	  strcat(file_name_buf, "/");
	  if (tok[0])
	    strcat(file_name_buf, tok);
	}
    }
  else 
    {
      file_name_buf = (char *)calloc(len + 8, sizeof(char));
      strcpy(file_name_buf, tok);
    }
  /* get rid of "/../" and "/./" also "/." at end */
  {
    int slash_at = -1;
    bool found_one = true;
    while (found_one)
      {
	found_one = false;
	len = strlen(file_name_buf);
	for (i = 0; i < len - 3; i++)
	  if (file_name_buf[i] == '/')
	    {
	      if ((file_name_buf[i + 1] == '.') &&
		  (file_name_buf[i + 2] == '.') &&
		  (file_name_buf[i + 3] == '/'))
		{
		  i += 4;
		  for (j = slash_at + 1; i < len; i++, j++)
		    file_name_buf[j] = file_name_buf[i];
		  file_name_buf[j] = '\0';
		  found_one = true;
		  break;
		}
	      else
		{
		  if ((file_name_buf[i + 1] == '.') &&
		      (file_name_buf[i + 2] == '/'))
		    {
		      for (j = i + 3, i = i + 1; j < len; i++, j++)
			file_name_buf[i] = file_name_buf[j];
		      file_name_buf[i] = '\0';
		      found_one = true;
		    }
		  else slash_at = i;
		}
	    }
      }
    len = strlen(file_name_buf);
    if ((len > 1) &&
	(file_name_buf[len - 1] == '.') &&
	(file_name_buf[len - 2] == '/'))
      file_name_buf[len - 1] = '\0';
  }
  free(orig);
  return(file_name_buf);
#endif
}


char *mus_format(const char *format, ...)
{
  /* caller should free result */
#ifndef _MSC_VER
  va_list ap;
  int bytes;
  char *result = NULL;
  va_start(ap, format);
  bytes = vasprintf(&result, format, ap);
  va_end(ap);
  if (bytes == -1)
    return(NULL);
  return(result);
#else

  #define MUS_FORMAT_BUFFER_SIZE 256
  char *buf = NULL;
  int needed_bytes = 0;
  va_list ap;
  buf = (char *)calloc(MUS_FORMAT_BUFFER_SIZE, sizeof(char));

  va_start(ap, format);
  needed_bytes = vsnprintf(buf, MUS_FORMAT_BUFFER_SIZE, format, ap);
  va_end(ap);

  if (needed_bytes >= MUS_FORMAT_BUFFER_SIZE) /* "=" here because we need room for the trailing 0 */
    {
      free(buf);
      buf = (char *)calloc(needed_bytes + 1, sizeof(char));
      va_start(ap, format);
      vsnprintf(buf, needed_bytes + 1, format, ap);
      va_end(ap);
    }
  return(buf);
#endif
}


mus_float_t mus_fclamp(mus_float_t lo, mus_float_t val, mus_float_t hi) 
{
  if (val > hi) 
    return(hi); 
  else 
    if (val < lo) 
      return(lo); 
    else return(val);
}


int mus_iclamp(int lo, int val, int hi) 
{
  if (val > hi) 
    return(hi); 
  else 
    if (val < lo) 
      return(lo); 
    else return(val);
}


mus_long_t mus_oclamp(mus_long_t lo, mus_long_t val, mus_long_t hi) 
{
  if (val > hi) 
    return(hi); 
  else 
    if (val < lo) 
      return(lo); 
    else return(val);
}



/* raw sample data peak value (without per-sample conversion) */


/* ---------------- short ---------------- */

#define SHORT_BYTES 2

static void min_max_shorts(unsigned char *data, int bytes, int chan, int chans, mus_float_t *min_samp, mus_float_t *max_samp)
{
  short cur_min, cur_max, tmp;
  short *sbuf;
  int i, len, len2;

  sbuf = (short *)data;
  len = bytes / SHORT_BYTES;

  cur_min = sbuf[chan];
  cur_max = cur_min;

  len2 = len - 2 * chans;
  i = chan;       /* we read the first sample above, but we're reading by twos below */
  while (i <= len2)
    {
      tmp = sbuf[i];
      if (tmp < cur_min) cur_min = tmp; else if (tmp > cur_max) cur_max = tmp;
      i += chans;
      tmp = sbuf[i];
      if (tmp < cur_min) cur_min = tmp; else if (tmp > cur_max) cur_max = tmp;
      i += chans;
    }
  if (i < len)
    {
      tmp = sbuf[i];
      if (tmp < cur_min) cur_min = tmp; else if (tmp > cur_max) cur_max = tmp;
    }

  (*min_samp) = (mus_float_t)cur_min / (mus_float_t)(1 << 15);
  (*max_samp) = (mus_float_t)cur_max / (mus_float_t)(1 << 15);
}


static void min_max_switch_shorts(unsigned char *data, int bytes, int chan, int chans, mus_float_t *min_samp, mus_float_t *max_samp)
{
  short cur_min, cur_max;
  /* frame based */
  unsigned char *samp, *eod, *eod2;
  int bytes_per_frame;

  bytes_per_frame = chans * SHORT_BYTES;
  eod = (unsigned char *)(data + bytes);
  eod2 = (unsigned char *)(eod - 2 * bytes_per_frame);

#if MUS_LITTLE_ENDIAN
  cur_min = big_endian_short((unsigned char *)(data + (chan * SHORT_BYTES)));
#else
  cur_min = little_endian_short((unsigned char *)(data + (chan * SHORT_BYTES)));
#endif
  cur_max = cur_min;
  samp = (unsigned char *)(data + (chan * SHORT_BYTES));
  while (samp <= eod2)
    {
      short val;
#if MUS_LITTLE_ENDIAN
      val = big_endian_short(samp);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
      samp += bytes_per_frame;
      val = big_endian_short(samp);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
      samp += bytes_per_frame;
#else
      val = little_endian_short(samp);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
      samp += bytes_per_frame;
      val = little_endian_short(samp);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
      samp += bytes_per_frame;
#endif
    }
  if (samp < eod)
    {
      short val;
#if MUS_LITTLE_ENDIAN
      val = big_endian_short(samp);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
#else
      val = little_endian_short(samp);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
#endif
    }

  (*min_samp) = (mus_float_t)cur_min / (mus_float_t)(1 << 15);
  (*max_samp) = (mus_float_t)cur_max / (mus_float_t)(1 << 15);
}


/* ---------------- unsigned short ---------------- */

static void min_max_ushorts(unsigned char *data, int bytes, int chan, int chans, mus_float_t *min_samp, mus_float_t *max_samp)
{
  unsigned short cur_min, cur_max;
  unsigned short *sbuf;
  int i, len, len2;

  sbuf = (unsigned short *)data;
  len = bytes / SHORT_BYTES;
  len2 = len - 2 * chans;

  cur_min = sbuf[chan];
  cur_max = cur_min;
  i = chan;
  while (i <= len2)
    {
      if (sbuf[i] < cur_min) cur_min = sbuf[i]; else if (sbuf[i] > cur_max) cur_max = sbuf[i];
      i += chans;
      if (sbuf[i] < cur_min) cur_min = sbuf[i]; else if (sbuf[i] > cur_max) cur_max = sbuf[i];
      i += chans;
    }
  if (i < len)
    {
      if (sbuf[i] < cur_min) cur_min = sbuf[i]; else if (sbuf[i] > cur_max) cur_max = sbuf[i];
    }

  (*min_samp) = (mus_float_t)(cur_min - USHORT_ZERO) / (mus_float_t)(1 << 15);
  (*max_samp) = (mus_float_t)(cur_max - USHORT_ZERO) / (mus_float_t)(1 << 15);
}


static void min_max_switch_ushorts(unsigned char *data, int bytes, int chan, int chans, mus_float_t *min_samp, mus_float_t *max_samp)
{
  unsigned short cur_min, cur_max;
  /* frame based */
  unsigned char *samp, *eod, *eod2;
  int bytes_per_frame;

  bytes_per_frame = chans * SHORT_BYTES;
  eod = (unsigned char *)(data + bytes);
  eod2 = (unsigned char *)(eod - 2 * bytes_per_frame);

#if MUS_LITTLE_ENDIAN
  cur_min = big_endian_unsigned_short((unsigned char *)(data + (chan * SHORT_BYTES)));
#else
  cur_min = little_endian_unsigned_short((unsigned char *)(data + (chan * SHORT_BYTES)));
#endif
  cur_max = cur_min;

  samp = (unsigned char *)(data + (chan * SHORT_BYTES));
  while (samp <= eod2)
    {
      unsigned short val;
#if MUS_LITTLE_ENDIAN
      val = big_endian_unsigned_short(samp);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
      samp += bytes_per_frame;
      val = big_endian_unsigned_short(samp);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
      samp += bytes_per_frame;
#else
      val = little_endian_unsigned_short(samp);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
      samp += bytes_per_frame;
      val = little_endian_unsigned_short(samp);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
      samp += bytes_per_frame;
#endif
    }
  if (samp < eod)
    {
      unsigned short val;
#if MUS_LITTLE_ENDIAN
      val = big_endian_unsigned_short(samp);
#else
      val = little_endian_unsigned_short(samp);
#endif
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
    }

  (*min_samp) = (mus_float_t)(cur_min - USHORT_ZERO) / (mus_float_t)(1 << 15);
  (*max_samp) = (mus_float_t)(cur_max - USHORT_ZERO) / (mus_float_t)(1 << 15);
}


/* ---------------- int ---------------- */

#define INT_BYTES 4

static void min_max_ints(unsigned char *data, int bytes, int chan, int chans, mus_float_t *min_samp, mus_float_t *max_samp, bool standard)
{
  int cur_min, cur_max;
  int *sbuf;
  int i, len, len2;

  sbuf = (int *)data;
  len = bytes / INT_BYTES;
  len2 = len - 2;

  cur_min = sbuf[chan];
  cur_max = cur_min;

  i = chan;
  while (i <= len2)
    {
      if (sbuf[i] < cur_min) cur_min = sbuf[i]; else if (sbuf[i] > cur_max) cur_max = sbuf[i];
      i += chans;
      if (sbuf[i] < cur_min) cur_min = sbuf[i]; else if (sbuf[i] > cur_max) cur_max = sbuf[i];
      i += chans;
    }
  if (i < len)
    {
      if (sbuf[i] < cur_min) cur_min = sbuf[i]; else if (sbuf[i] > cur_max) cur_max = sbuf[i];
    }
  
  (*min_samp) = (mus_float_t)cur_min / (mus_float_t)(1 << 23);
  (*max_samp) = (mus_float_t)cur_max / (mus_float_t)(1 << 23);

  if (!standard)
    {
      (*min_samp) = (mus_float_t)(*min_samp) / (mus_float_t)(1 << 8);
      (*max_samp) = (mus_float_t)(*max_samp) / (mus_float_t)(1 << 8);
    }
}

/* (with-sound (:data-format mus-lintn :statistics #t) (fm-violin 0 1 440 .1)) */


static void min_max_switch_ints(unsigned char *data, int bytes, int chan, int chans, mus_float_t *min_samp, mus_float_t *max_samp, bool standard)
{
  int cur_min, cur_max;
  /* frame based */
  unsigned char *samp, *eod, *eod2;
  int bytes_per_frame;

  bytes_per_frame = chans * INT_BYTES;
  eod = (unsigned char *)(data + bytes);
  eod2 = (unsigned char *)(eod - 2 * bytes_per_frame);

#if MUS_LITTLE_ENDIAN
  cur_min = big_endian_int((unsigned char *)(data + (chan * INT_BYTES)));
#else
  cur_min = little_endian_int((unsigned char *)(data + (chan * INT_BYTES)));
#endif
  cur_max = cur_min;

  samp = (unsigned char *)(data + (chan * INT_BYTES));
  while (samp <= eod2)
    {
      int val;
#if MUS_LITTLE_ENDIAN
      val = big_endian_int(samp);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
      samp += bytes_per_frame;
      val = big_endian_int(samp);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
      samp += bytes_per_frame;
#else
      val = little_endian_int(samp);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
      samp += bytes_per_frame;
      val = little_endian_int(samp);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
      samp += bytes_per_frame;
#endif
    }
  if (samp < eod)
    {
      int val;
#if MUS_LITTLE_ENDIAN
      val = big_endian_int(samp);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
#else
      val = little_endian_int(samp);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
#endif
    }

  (*min_samp) = (mus_float_t)cur_min / (mus_float_t)(1 << 23);
  (*max_samp) = (mus_float_t)cur_max / (mus_float_t)(1 << 23);

  if (!standard)
    {
      (*min_samp) = (mus_float_t)(*min_samp) / (mus_float_t)(1 << 8);
      (*max_samp) = (mus_float_t)(*max_samp) / (mus_float_t)(1 << 8);
    }
}



/* ---------------- float ---------------- */

#define FLOAT_BYTES 4

static void min_max_floats(unsigned char *data, int bytes, int chan, int chans, mus_float_t *min_samp, mus_float_t *max_samp, bool unscaled)
{
  float cur_min, cur_max;
  float *sbuf;
  int i, len, len2;

  sbuf = (float *)data;
  len = bytes / FLOAT_BYTES;
  len2 = len - 2;

  cur_min = sbuf[chan];
  cur_max = cur_min;

  i = chan;
  while (i < len2)
    {
      if (sbuf[i] < cur_min) cur_min = sbuf[i]; else if (sbuf[i] > cur_max) cur_max = sbuf[i];
      i += chans;
      if (sbuf[i] < cur_min) cur_min = sbuf[i]; else if (sbuf[i] > cur_max) cur_max = sbuf[i];
      i += chans;
    }
  while (i < len)
    {
      if (sbuf[i] < cur_min) cur_min = sbuf[i]; else if (sbuf[i] > cur_max) cur_max = sbuf[i];
      i += chans;
    }

  if (unscaled)
    {
      (*min_samp) = cur_min / 32768.0;
      (*max_samp) = cur_max / 32768.0;
    }
  else
    {
      (*min_samp) = cur_min;
      (*max_samp) = cur_max;
    }
}


static void min_max_switch_floats(unsigned char *data, int bytes, int chan, int chans, mus_float_t *min_samp, mus_float_t *max_samp, bool unscaled)
{
  float cur_min, cur_max;
  /* frame based */
  unsigned char *samp, *eod, *eod2;
  int bytes_per_frame;

  bytes_per_frame = chans * FLOAT_BYTES;
  eod = (unsigned char *)(data + bytes);
  eod2 = (unsigned char *)(eod - 2 * bytes_per_frame);

#if MUS_LITTLE_ENDIAN
  cur_min = big_endian_float((unsigned char *)(data + (chan * FLOAT_BYTES)));
#else
  cur_min = little_endian_float((unsigned char *)(data + (chan * FLOAT_BYTES)));
#endif
  cur_max = cur_min;
  samp = (unsigned char *)(data + (chan * FLOAT_BYTES)); 
  while (samp <= eod2)
    {
      float val;
#if MUS_LITTLE_ENDIAN
      val = big_endian_float(samp);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
      samp += bytes_per_frame;
      val = big_endian_float(samp);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
      samp += bytes_per_frame;
#else
      val = little_endian_float(samp);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
      samp += bytes_per_frame;
      val = little_endian_float(samp);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
      samp += bytes_per_frame;
#endif
    }

  if (samp < eod)
    {
      float val;
#if MUS_LITTLE_ENDIAN
      val = big_endian_float(samp);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
#else
      val = little_endian_float(samp);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
#endif
    }

  if (unscaled)
    {
      (*min_samp) = cur_min / 32768.0;
      (*max_samp) = cur_max / 32768.0;
    }
  else
    {
      (*min_samp) = cur_min;
      (*max_samp) = cur_max;
    }
}



/* ---------------- double ---------------- */

#define DOUBLE_BYTES 8

static void min_max_doubles(unsigned char *data, int bytes, int chan, int chans, mus_float_t *min_samp, mus_float_t *max_samp, bool unscaled)
{
  double cur_min, cur_max;
  double *sbuf;
  int i, len, len2;

  sbuf = (double *)data;
  len = bytes / DOUBLE_BYTES;
  len2 = len - 2;

  cur_min = sbuf[chan];
  cur_max = cur_min;
  i = chan;
  while (i <= len2)
    {
      if (sbuf[i] < cur_min) cur_min = sbuf[i]; else if (sbuf[i] > cur_max) cur_max = sbuf[i];
      i += chans;
      if (sbuf[i] < cur_min) cur_min = sbuf[i]; else if (sbuf[i] > cur_max) cur_max = sbuf[i];
      i += chans;
    }
  if (i < len)
    {
      if (sbuf[i] < cur_min) cur_min = sbuf[i]; else if (sbuf[i] > cur_max) cur_max = sbuf[i];
    }

  if (unscaled)
    {
      (*min_samp) = cur_min / 32768.0;
      (*max_samp) = cur_max / 32768.0;
    }
  else
    {
      (*min_samp) = cur_min;
      (*max_samp) = cur_max;
    }
}


static void min_max_switch_doubles(unsigned char *data, int bytes, int chan, int chans, mus_float_t *min_samp, mus_float_t *max_samp, bool unscaled)
{
  double cur_min, cur_max;
  /* frame based */
  unsigned char *samp, *eod, *eod2;
  int bytes_per_frame;

  bytes_per_frame = chans * DOUBLE_BYTES;
  eod = (unsigned char *)(data + bytes);
  eod2 = (unsigned char *)(eod - 2 * bytes_per_frame);

#if MUS_LITTLE_ENDIAN
  cur_min = big_endian_double((unsigned char *)(data + (chan * DOUBLE_BYTES)));
#else
  cur_min = little_endian_double((unsigned char *)(data + (chan * DOUBLE_BYTES)));
#endif
  cur_max = cur_min;
  samp = (unsigned char *)(data + (chan * DOUBLE_BYTES)); 
  while (samp <= eod2)
    {
      double val;
#if MUS_LITTLE_ENDIAN
      val = big_endian_double(samp);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
      samp += bytes_per_frame;
      val = big_endian_double(samp);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
      samp += bytes_per_frame;
#else
      val = little_endian_double(samp);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
      samp += bytes_per_frame;
      val = little_endian_double(samp);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
      samp += bytes_per_frame;
#endif
    }
  if (samp < eod)
    {
      double val;
#if MUS_LITTLE_ENDIAN
      val = big_endian_double(samp);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
#else
      val = little_endian_double(samp);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
#endif
    }

  if (unscaled)
    {
      (*min_samp) = cur_min / 32768.0;
      (*max_samp) = cur_max / 32768.0;
    }
  else
    {
      (*min_samp) = cur_min;
      (*max_samp) = cur_max;
    }
}



/* ---------------- 3-byte samples ---------------- */

#define THREE_BYTES 3

#define BIG_THREE(Data, Loc)    ((int)(((Data[Loc + 0] << 24) + (Data[Loc + 1] << 16) + (Data[Loc + 2] << 8)) >> 8))
#define LITTLE_THREE(Data, Loc) ((int)(((Data[Loc + 2] << 24) + (Data[Loc + 1] << 16) + (Data[Loc + 0] << 8)) >> 8))

static void min_max_24s(unsigned char *data, int bytes, int chan, int chans, mus_float_t *min_samp, mus_float_t *max_samp, bool big_endian)
{
  int cur_min, cur_max;
  int i, k, bytes_per_frame, len, len2, offset;

  bytes_per_frame = chans * THREE_BYTES;
  len = bytes / bytes_per_frame;
  len2 = len - 2;
  offset = chan * THREE_BYTES;

  k = offset;

  if (big_endian)
    {
      cur_min = BIG_THREE(data, k);
      cur_max = cur_min;
      i = 0;
      /* k += bytes_per_frame; */

      while (i <= len2)
	{
	  int val;
	  val = BIG_THREE(data, k);
	  if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
	  i++;
	  k += bytes_per_frame;
	  val = BIG_THREE(data, k);
	  if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
	  i++;
	  k += bytes_per_frame;
	}
      while (i < len)
	{
	  int val;
	  val = BIG_THREE(data, k);
	  if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
	  i++;
	  k += bytes_per_frame;
	}
    }
  else
    {
      cur_min = LITTLE_THREE(data, k);
      cur_max = cur_min;
      i = 0;
      /* k += bytes_per_frame; */

      while (i <= len2)
	{
	  int val;
	  val = LITTLE_THREE(data, k);
	  if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
	  i++;
	  k += bytes_per_frame;
	  val = LITTLE_THREE(data, k);
	  if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
	  i++;
	  k += bytes_per_frame;
	}
      while (i < len)
	{
	  int val;
	  val = LITTLE_THREE(data, k);
	  if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
	  i++;
	  k += bytes_per_frame;
	}
    }

  (*min_samp) = (mus_float_t)cur_min / (mus_float_t)(1 << 23);
  (*max_samp) = (mus_float_t)cur_max / (mus_float_t)(1 << 23);
}



/* ---------------- mulaw, alaw, byte ---------------- */

static void min_max_mulaw(unsigned char *data, int bytes, int chan, int chans, mus_float_t *min_samp, mus_float_t *max_samp)
{
  mus_float_t cur_min, cur_max;
  int i, b2;

  cur_min = mus_mulaw[(int)data[chan]];
  cur_max = cur_min;

  i = chan;
  b2 = bytes - 2;
  while (i <= b2)
    {
      mus_float_t val;
      val = mus_mulaw[(int)data[i]];
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
      i += chans;
      val = mus_mulaw[(int)data[i]];
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
      i += chans;
    }
  if (i < bytes)
    {
      mus_float_t val;
      val = mus_mulaw[(int)data[i]];
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
    }

  (*min_samp) = cur_min;
  (*max_samp) = cur_max;
}


static void min_max_alaw(unsigned char *data, int bytes, int chan, int chans, mus_float_t *min_samp, mus_float_t *max_samp)
{
  mus_float_t cur_min, cur_max;
  int i, b2;

  cur_min = mus_alaw[(int)data[chan]];
  cur_max = cur_min;

  i = chan;
  b2 = bytes - 2;
  while (i <= b2)
    {
      mus_float_t val;
      val = mus_alaw[(int)data[i]];
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
      i += chans;
      val = mus_alaw[(int)data[i]];
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
      i += chans;
    }
  if (i < bytes)
    {
      mus_float_t val;
      val = mus_alaw[(int)data[i]];
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
    }

  (*min_samp) = cur_min;
  (*max_samp) = cur_max;
}


static void min_max_bytes(unsigned char *data, int bytes, int chan, int chans, mus_float_t *min_samp, mus_float_t *max_samp)
{
  signed char cur_min, cur_max;
  int i, b2;

  cur_min = (signed char)(data[chan]);
  cur_max = cur_min;

  i = chan;
  b2 = bytes - 2;
  while (i <= b2)
    {
      signed char val;
      val = (signed char)(data[i]);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
      i += chans;
      val = (signed char)(data[i]);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
      i += chans;
    }
  if (i < bytes)
    {
      signed char val;
      val = (signed char)(data[i]);
      if (val < cur_min) cur_min = val; else if (val > cur_max) cur_max = val;
    }

  (*min_samp) = (mus_float_t)cur_min / (mus_float_t)(1 << 7);
  (*max_samp) = (mus_float_t)cur_max / (mus_float_t)(1 << 7);
}


static void min_max_ubytes(unsigned char *data, int bytes, int chan, int chans, mus_float_t *min_samp, mus_float_t *max_samp)
{
  unsigned char cur_min, cur_max;
  int i, b2;

  cur_min = data[chan];
  cur_max = cur_min;

  i = chan;
  b2 = bytes - 2;
  while (i <= b2)
    {
      if (data[i] < cur_min) cur_min = data[i]; else if (data[i] > cur_max) cur_max = data[i];
      i += chans;
      if (data[i] < cur_min) cur_min = data[i]; else if (data[i] > cur_max) cur_max = data[i];
      i += chans;
    }
  if (i < bytes)
    {
      if (data[i] < cur_min) cur_min = data[i]; else if (data[i] > cur_max) cur_max = data[i];
    }

  (*min_samp) = (mus_float_t)(cur_min - UBYTE_ZERO) / (mus_float_t)(1 << 7);
  (*max_samp) = (mus_float_t)(cur_max - UBYTE_ZERO) / (mus_float_t)(1 << 7);
}


int mus_samples_bounds(unsigned char *data, int bytes, int chan, int chans, int format, mus_float_t *min_samp, mus_float_t *max_samp)
{
  switch (format)
    {
    case MUS_MULAW:
      min_max_mulaw(data, bytes, chan, chans, min_samp, max_samp);
      break;

    case MUS_ALAW:
      min_max_alaw(data, bytes, chan, chans, min_samp, max_samp);
      break;

    case MUS_BYTE:
      min_max_bytes(data, bytes, chan, chans, min_samp, max_samp);
      break;

    case MUS_UBYTE:
      min_max_ubytes(data, bytes, chan, chans, min_samp, max_samp);
      break;

    case MUS_L24INT:
      min_max_24s(data, bytes, chan, chans, min_samp, max_samp, false);
      break;

    case MUS_B24INT:
      min_max_24s(data, bytes, chan, chans, min_samp, max_samp, true);
      break;

#if MUS_LITTLE_ENDIAN

    case MUS_LSHORT:
      min_max_shorts(data, bytes, chan, chans, min_samp, max_samp);
      break;

    case MUS_BSHORT:
      min_max_switch_shorts(data, bytes, chan, chans, min_samp, max_samp);
      break;

    case MUS_ULSHORT:
      min_max_ushorts(data, bytes, chan, chans, min_samp, max_samp);
      break;

    case MUS_UBSHORT:
      min_max_switch_ushorts(data, bytes, chan, chans, min_samp, max_samp);
      break;

    case MUS_LINT:
    case MUS_LINTN:
      min_max_ints(data, bytes, chan, chans, min_samp, max_samp, format == MUS_LINT);
      break;

    case MUS_BINT:
    case MUS_BINTN:
      min_max_switch_ints(data, bytes, chan, chans, min_samp, max_samp, format == MUS_BINT);
      break;

    case MUS_LFLOAT:
    case MUS_LFLOAT_UNSCALED:
      min_max_floats(data, bytes, chan, chans, min_samp, max_samp, format == MUS_LFLOAT_UNSCALED);
      break;

    case MUS_BFLOAT:
    case MUS_BFLOAT_UNSCALED:
      min_max_switch_floats(data, bytes, chan, chans, min_samp, max_samp, format == MUS_BFLOAT_UNSCALED);
      break;

    case MUS_LDOUBLE:
    case MUS_LDOUBLE_UNSCALED:
      min_max_doubles(data, bytes, chan, chans, min_samp, max_samp, format == MUS_LDOUBLE_UNSCALED);
      break;

    case MUS_BDOUBLE:
    case MUS_BDOUBLE_UNSCALED:
      min_max_switch_doubles(data, bytes, chan, chans, min_samp, max_samp, format == MUS_BDOUBLE_UNSCALED);
      break;

#else /* big endian */

    case MUS_LSHORT:
      min_max_switch_shorts(data, bytes, chan, chans, min_samp, max_samp);
      break;

    case MUS_BSHORT:
      min_max_shorts(data, bytes, chan, chans, min_samp, max_samp);
      break;
    case MUS_ULSHORT:
      min_max_switch_ushorts(data, bytes, chan, chans, min_samp, max_samp);
      break;

    case MUS_UBSHORT:
      min_max_ushorts(data, bytes, chan, chans, min_samp, max_samp);
      break;

    case MUS_LINT:
    case MUS_LINTN:
      min_max_switch_ints(data, bytes, chan, chans, min_samp, max_samp, format == MUS_LINT);
      break;

    case MUS_BINT:
    case MUS_BINTN:
      min_max_ints(data, bytes, chan, chans, min_samp, max_samp, format == MUS_BINT);
      break;

    case MUS_LFLOAT:
    case MUS_LFLOAT_UNSCALED:
      min_max_switch_floats(data, bytes, chan, chans, min_samp, max_samp, format == MUS_LFLOAT_UNSCALED);
      break;

    case MUS_BFLOAT:
    case MUS_BFLOAT_UNSCALED:
      min_max_floats(data, bytes, chan, chans, min_samp, max_samp, format == MUS_BFLOAT_UNSCALED);
      break;

    case MUS_LDOUBLE:
    case MUS_LDOUBLE_UNSCALED:
      min_max_switch_doubles(data, bytes, chan, chans, min_samp, max_samp, format == MUS_LDOUBLE_UNSCALED);
      break;

    case MUS_BDOUBLE:
    case MUS_BDOUBLE_UNSCALED:
      min_max_doubles(data, bytes, chan, chans, min_samp, max_samp, format == MUS_BDOUBLE_UNSCALED);
      break;

#endif

    default:
      return(MUS_ERROR);
      break;
    }

  return(MUS_NO_ERROR);
}


char *mus_strdup(const char *str)
{
  char *newstr = NULL;
  if ((!str) || (!(*str))) return(NULL);
  newstr = (char *)malloc(strlen(str) + 1);
  if (newstr) strcpy(newstr, str);
  return(newstr);
}


int mus_strlen(const char *str)
{
  /* strlen(NULL) -> seg fault! */
  if ((str) && (*str)) return(strlen(str));
  return(0);
}


bool mus_strcmp(const char *str1, const char *str2)
{
  return((str1 == str2) ||
	 ((str1) && (str2) &&
	  (strcmp(str1, str2) == 0)));
}


char *mus_strcat(char *errmsg, const char *str, int *size)
{
  int new_len, err_size;
  new_len = (mus_strlen(str) + mus_strlen(errmsg));
  err_size = size[0];
  if (new_len >= err_size)
    {
      if ((err_size * 2) > new_len)
	err_size = err_size * 2;
      else err_size = new_len * 2;
      errmsg = (char *)realloc(errmsg, err_size * sizeof(char));
      size[0] = err_size;
    }
  strcat(errmsg, str);
  return(errmsg);
}


