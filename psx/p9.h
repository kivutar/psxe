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

#define printf print

static int
putchar(int c)
{
	return print("%c", c);
}

static void
exit(int status)
{
	exits(status ? "error" : nil);
}

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#define EOF (-1)

typedef struct p9_file {
	int fd;
} FILE;

static FILE*
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

static int
fclose(FILE *fp)
{
	int r;

	if (!fp)
		return -1;

	r = close(fp->fd);
	free(fp);
	return r;
}

static int
fseek(FILE *fp, long offset, int whence)
{
	if (!fp)
		return -1;

	return seek(fp->fd, offset, whence) < 0 ? -1 : 0;
}

static long
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

static size_t
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

static size_t
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

static int
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

static int
isdigit(int c)
{
	return c >= '0' && c <= '9';
}

static int
isalpha(int c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static int
isspace(int c)
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

#define __attribute__(x)

#define __FILE__ "unknown"
#define __LINE__ 0

#define floorf(x) ((float)floor((double)(x)))
#define ceilf(x) ((float)ceil((double)(x)))
#define roundf(x) ((float)(((x) >= 0) ? floor((double)(x) + 0.5) : ceil((double)(x) - 0.5)))

static int
__builtin_ssub_overflow(int a, int b, int *r)
{
  vlong v = (vlong)a - (vlong)b;
  *r = (int)v;
  return (v > 2147483647LL) || (v < -2147483648LL);
}

static int
__builtin_clz(unsigned int x)
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

#define INT8_MIN (-128)
#define INT8_MAX 127
#define INT16_MIN (-32768)
#define INT16_MAX 32767
#define INT32_MIN (-2147483647 - 1)
#define INT32_MAX 2147483647

#endif
