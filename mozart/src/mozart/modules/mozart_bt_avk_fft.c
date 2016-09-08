#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

#define FFT_N (1024 * 2)
#define PI 3.14159265358
#define BAND_COUNT 43

typedef struct {
	float real;
	float img;
} complex;

void fft();
void ifft();
void change();
void mozart_bt_avk_init_fft();
void mozart_bt_avk_uninit_fft();
void add(complex, complex, complex *);
void mul(complex, complex, complex *);
void sub(complex, complex, complex *);
void divi(complex, complex, complex *);

complex x[FFT_N], *W;
int size_x = FFT_N;
float srt(complex x);

int mozart_bt_avk_compute_height(short *buff, int *height)
{
	unsigned int flag[43] = {
		360, 200, 370, 360, 230, 340, 79, 320,  72,  300,
		350, 75,  330, 77,  79,  310, 83, 189,  87,  90,
		102, 103, 260, 116, 123, 130, 137, 141, 148, 152,
		250, 169, 177, 85,  192, 380, 232, 240, 160, 109,
		270, 280, 290
	};

	float value = 0;
	int i;
	for (i = 0; i < 2048; i++) {
		x[i].real = buff[i];
		x[i].img = 0;
	}

	fft();

	for (i = 0; i < BAND_COUNT; i++) {
		value = 20 * log10(srt(x[flag[i]])) - 55;
		if (value > 0)
			height[i] = value;
		else
			height[i] = 3;
	}

	return 0;
}

void fft()
{
	int i = 0, j = 0, k = 0, l = 0;
	complex up, down, product;
	change();
	for (i = 0; i < log(size_x) / log(2); i++) {
		l = 1 << i;
		for (j = 0; j < size_x; j += 2 * l) {
			for (k = 0; k < l; k++) {
				mul(x[j + k + l], W[size_x * k / 2 / l], &product);
				add(x[j + k], product, &up);
				sub(x[j + k], product, &down);
				x[j + k] = up;
				x[j + k + l] = down;
			}
		}
	}
}

void ifft()
{
	int i = 0, j = 0, k = 0, l = size_x;
	complex up, down;
	for (i = 0; i < (int)(log(size_x) / log(2)); i++) {
		l /= 2;
		for (j = 0; j < size_x; j += 2 * l) {
			for ( k = 0; k < l; k++) {
				add(x[ j + k], x[j + k + l], &up);
				up.real /= 2;
				up.img /= 2;
				sub(x[j + k], x[j + k + l], &down);
				down.real /= 2;
				down.img /= 2;
				divi(down, W[size_x * k / 2 / l], &down);
				x[j + k] = up;
				x[j + k + l] = down;
			}
		}
	}
	change();
}

void mozart_bt_avk_init_fft()
{
	int i;
	W = (complex *)malloc(sizeof(complex) * size_x);
	for (i = 0; i < size_x; i++) {
		W[i].real = cos(2 * PI / size_x * i);
		W[i].img = -1 * sin(2 * PI / size_x * i);
	}
}

void mozart_bt_avk_uninit_fft()
{
	free(W);
}

void change()
{
	complex temp;
	unsigned short i = 0, j = 0, k = 0;
	double t;
	for ( i = 0; i < size_x; i++) {
		k = i;
		j = 0;
		t = (log(size_x) / log(2));
		while (t-- > 0) {
			j = j << 1;
			j |= (k & 1);
			k = k >> 1;
		}
		if (j > i) {
			temp = x[i];
			x[i] = x[j];
			x[j] = temp;
		}
	}
}

void add(complex a, complex b, complex *c)
{
	c->real = a.real + b.real;
	c->img = a.img + b.img;
}

void mul(complex a, complex b, complex *c)
{
	c->real = a.real * b.real - a.img * b.img;
	c->img = a.real * b.img + a.img * b.real;
}

void sub(complex a, complex b, complex *c)
{
	c->real = a.real - b.real;
	c->img = a.img - b.img;
}

void divi(complex a, complex b, complex *c)
{
	c->real = (a.real * b.real + a.img * b.img) / (b.real * b.real + b.img * b.img);
	c->img = (a.img * b.real - a.real * b.img) / (b.real * b.real + b.img * b.img);
}

float srt(complex x)
{
	return (sqrt((x.real * x.real) + (x.img * x.img)));
}

