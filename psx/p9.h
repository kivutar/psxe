#ifndef PSX_P9_H
#define PSX_P9_H

#include <u.h>
#include <libc.h>

typedef u8int uint8_t;
typedef u16int uint16_t;
typedef u32int uint32_t;
typedef u64int uint64_t;
typedef s8int int8_t;
typedef s16int int16_t;
typedef s32int int32_t;
typedef s64int int64_t;
typedef intptr intptr_t;
typedef uintptr uintptr_t;

typedef uintptr size_t;
typedef intptr ssize_t;

typedef int bool;
#define true 1
#define false 0

#define NULL nil

/* Minimal libc naming shims for native Plan 9 builds. */
#ifndef printf
#define printf print
#endif
#ifndef putchar
static inline int
p9_putchar(int c)
{
	return print("%c", c);
}
#define putchar(c) p9_putchar((c))
#endif
#ifndef exit
static inline void
p9_exit(int status)
{
	exits(status ? "error" : nil);
}
#define exit(status) p9_exit((status))
#endif

/* Minimal stdio compatibility for code still using FILE*. */
#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif
#ifndef EOF
#define EOF (-1)
#endif

#ifndef _P9_FILE_COMPAT
#define _P9_FILE_COMPAT
typedef struct p9_file {
	int fd;
} FILE;

static inline FILE*
fopen(const char *path, const char *mode)
{
	int fd;
	int omode;
	FILE *fp;

	if (!path || !mode)
		return nil;

	omode = -1;

	switch (mode[0]) {
	case 'r':
		omode = OREAD;
		break;
	case 'w':
		omode = OWRITE | OTRUNC;
		break;
	case 'a':
		omode = OWRITE;
		break;
	}

	if (omode < 0)
		return nil;

	if (mode[0] == 'r') {
		fd = open(path, omode);
	} else {
		fd = create(path, omode, 0666);
		if (fd < 0)
			fd = open(path, omode);
	}

	if (fd < 0)
		return nil;

	fp = malloc(sizeof(*fp));
	if (!fp) {
		close(fd);
		return nil;
	}

	fp->fd = fd;

	if (mode[0] == 'a')
		(void)seek(fp->fd, 0, SEEK_END);

	return fp;
}

static inline int
fclose(FILE *fp)
{
	int r;

	if (!fp)
		return -1;

	r = close(fp->fd);
	free(fp);
	return r;
}

static inline int
fseek(FILE *fp, long offset, int whence)
{
	if (!fp)
		return -1;

	return seek(fp->fd, offset, whence) < 0 ? -1 : 0;
}

static inline long
ftell(FILE *fp)
{
	vlong pos;

	if (!fp)
		return -1;

	pos = seek(fp->fd, 0, SEEK_CUR);
	if (pos < 0)
		return -1;

	return (long)pos;
}

static inline size_t
fread(void *ptr, size_t size, size_t nmemb, FILE *fp)
{
	size_t total;
	size_t done;
	uchar *p;

	if (!fp || !ptr || size == 0 || nmemb == 0)
		return 0;

	total = size * nmemb;
	done = 0;
	p = (uchar*)ptr;

	while (done < total) {
		long r = read(fp->fd, p + done, total - done);

		if (r <= 0)
			break;

		done += r;
	}

	return done / size;
}

static inline size_t
fwrite(const void *ptr, size_t size, size_t nmemb, FILE *fp)
{
	size_t total;
	size_t done;
	const uchar *p;

	if (!fp || !ptr || size == 0 || nmemb == 0)
		return 0;

	total = size * nmemb;
	done = 0;
	p = (const uchar*)ptr;

	while (done < total) {
		long w = write(fp->fd, (void*)(p + done), total - done);

		if (w <= 0)
			break;

		done += w;
	}

	return done / size;
}

static inline int
fgetc(FILE *fp)
{
	uchar c;
	long r;

	if (!fp)
		return EOF;

	r = read(fp->fd, &c, 1);
	if (r != 1)
		return EOF;

	return (int)c;
}
#endif

#ifndef isdigit
static inline int
p9_isdigit(int c)
{
	return c >= '0' && c <= '9';
}
#define isdigit(c) p9_isdigit((c))
#endif

#ifndef isalpha
static inline int
p9_isalpha(int c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
#define isalpha(c) p9_isalpha((c))
#endif

#ifndef isspace
static inline int
p9_isspace(int c)
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}
#define isspace(c) p9_isspace((c))
#endif

/* GCC attributes are ignored by native 9front compilers. */
#ifndef __attribute__
#define __attribute__(x)
#endif

#define __FILE__ "unknown"
#define __LINE__ 0

/* Minimal assert replacement for native headers. */
#ifndef assert
#define assert(x) do { \
  if (!(x)) sysfatal("assert failed: %s:%d: %s", __FILE__, __LINE__, #x); \
} while (0)
#endif

/* Helpers for codepaths that expect C99 float math helpers. */
#ifndef floorf
#define floorf(x) ((float)floor((double)(x)))
#endif
#ifndef ceilf
#define ceilf(x) ((float)ceil((double)(x)))
#endif
#ifndef roundf
#define roundf(x) ((float)(((x) >= 0) ? floor((double)(x) + 0.5) : ceil((double)(x) - 0.5)))
#endif

/* GCC builtin compatibility shims used by the CPU core. */
#ifndef __GNUC__
static inline int
p9_builtin_ssub_overflow(int a, int b, int *r)
{
  vlong v = (vlong)a - (vlong)b;
  *r = (int)v;
  return (v > 2147483647LL) || (v < -2147483648LL);
}

static inline int
p9_builtin_clz(unsigned int x)
{
  int n = 0;

  if (x == 0)
    return 32;

  while ((x & 0x80000000U) == 0) {
    n++;
    x <<= 1;
  }

  return n;
}

#define __builtin_ssub_overflow(a, b, r) p9_builtin_ssub_overflow((a), (b), (r))
#define __builtin_clz(x) p9_builtin_clz((unsigned int)(x))
#endif

#define INT8_MIN (-128)
#define INT8_MAX 127
#define INT16_MIN (-32768)
#define INT16_MAX 32767
#define INT32_MIN (-2147483647 - 1)
#define INT32_MAX 2147483647

#endif
