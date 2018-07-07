#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <string.h>
#include "audio.h"

#define WBUFS 256
#define FCLOCK 1996800.0
#define MEMTABSIZE 1000

static double timbre_proc0(double x, double dx, double* a, int na){
    static int table_created = 0;
    static double mem_wave[MEMTABSIZE];
    int n, i;

    if(table_created == 0){
	int i;
	double t;
	table_created = 1;
	
	for(i = 0; i < MEMTABSIZE; i++){
	    t = (double)i / MEMTABSIZE;
	    t = sin(t * (2 * M_PI));
	    mem_wave[i] = t;
	}
    }

    x = fmod(x, 1.0);
    dx *= MEMTABSIZE;
    x  *= MEMTABSIZE;
    for(i = 0; i < na; i++){
	n = (int)x;
	a[i] = mem_wave[n];
	x += dx;
	while(x >= MEMTABSIZE){
	    x -= MEMTABSIZE;
	}
    }
    return x * (1.0/MEMTABSIZE);
}

static double timbre_proc1(double x, double dx, double* a, int na){
    int i;

    x = fmod(x, 1.0);
    for(i = 0; i < na; i++){
	if(x < 0.5){
	    a[i] = 1;
	}else{
	    a[i] = -1;
	}
	x += dx;
	while(x >= 1){
	    x--;
	}
    }
    return x;
}

static double timbre_proc2(double x, double dx, double* a, int na){
    int i;

    x = fmod(x, 1.0) * 2;
    dx *= 2;
    for(i = 0; i < na; i++){
	a[i] = x - 1;
	x += dx;
	while(x >= 2){
	    x -= 2;
	}
    }
    return x * 0.5;
}

static double timbre_proc3(double x, double dx, double* a, int na){
    const double t0 = 10.0;
    const double t1 = 20.0;
    double y;
    int i;

    x = fmod(x, 1.0) * t1;
    dx *= t1;
    for(i = 0; i < na; i++){
	y = exp(t0 - x);
	a[i] = (1 - y) / (1 + y);
	x += dx;
	while(x >= t1){
	    x -= t1;
	}
    }
    return x / t1;
}

static double timbre_proc4(double x, double dx, double* a, int na){
    int i;
    static unsigned long seed = 1;
    unsigned char c;

    x = timbre_proc0(x, dx, a, na);

    for(i = 0; i < na; i++){
	seed = seed * 29989 + 11; /* next random sequence */
	c = (unsigned char)seed % 255;
	a[i] = a[i] * c * (1.0/255.0);
    }
    return x;
}

static double (* timbre_proc)(double,double,double*,int) = timbre_proc0;

void set_timbre(int y){
    switch(y){
      case 0:
	timbre_proc = timbre_proc0;
	break;
      case 1:
	timbre_proc = timbre_proc1;
	break;
      case 2:
	timbre_proc = timbre_proc2;
	break;
      case 3:
	timbre_proc = timbre_proc3;
	break;
      case 4:
	timbre_proc = timbre_proc4;
	break;
      case 5:
	break;
      default:
	fprintf(stderr, "Warning: Invalid timbre type: %d\n", y);
	break;
    }
}

static void gen_wave0(audio_word* a, int n, double f, double vol);
static void gen_wave1(audio_word* a, int n, double f, double vol, double t);
static void gen_wave2(audio_word* a, int n, double f, double vol, double t);
static void gen_wave3(audio_word* a, int n, double f, double vol, double t);
static void gen_wave4(audio_word* a, int n, double f, double vol, double t);
static void gen_wave5(audio_word* a, int n, double f, double vol, double t);
static void gen_wave6(audio_word* a, int n, double f, double vol, double t);
static void gen_wave7(audio_word* a, int n, double f, double vol, double t);
static void gen_wave8(audio_word* a, int n, double f, double vol, double t);

int audio_gen_wave(audio_word* a,	/* wave_data */
		   int n,	/* number of wave_data */
		   double f,	/* frequency (Hz) */
		   double vol,	/* volume [0.0, 1.0]*/
		   unsigned short m,	/* M */
		   char s){	/* S */
    double t;

    t = (double)m * 256.0 / FCLOCK;
    vol *= AMAX;

    if(m == 0){
	gen_wave0(a, n, f, vol);
	return 1;
    }

    switch(s){
      case -1:
	gen_wave0(a, n, f, vol);
	return 1;
      case 0:
      case 1:
      case 2:
      case 3:
      case 9:
	gen_wave1(a, n, f, vol, t);
	return 1;
      case 4:
      case 5:
      case 6:
      case 7:
      case 15:
	gen_wave2(a, n, f, vol, t);
	return 1;
      case 8:
	gen_wave3(a, n, f, vol, t);
	return 1;
      case 10:
	gen_wave4(a, n, f, vol, t);
	return 1;
      case 11:
	gen_wave5(a, n, f, vol, t);
	return 1;
      case 12:
	gen_wave6(a, n, f, vol, t);
	return 1;
      case 13:
	gen_wave7(a, n, f, vol, t);
	return 1;
      case 14:
	gen_wave8(a, n, f, vol, t);
	return 1;
    }
    return 0;
}

#define VOLUME_RANGE(x) ((x) < -AMAX ? -AMAX : ((x) > AMAX) ? AMAX : (x))

static void gen_wave0(audio_word* a, int n, double f, double vol){
    double wbuf[WBUFS];
    int wn, i;
    double t, x, d;

    t = 0;
    d = f * (1.0/SAMP_RATE);
    while(n > 0){
	if(n >= WBUFS){
	    wn = WBUFS;
	}else{
	    wn = n;
	}
	t = timbre_proc(t, d, wbuf, wn);
	for(i = 0; i < wn; i++){
	    x = vol * wbuf[i];
	    a[i] = (audio_word)VOLUME_RANGE(x);
	}
	n -= wn;
	a += wn;
    }
}

static void gen_wave1(audio_word* a, int n, double f, double vol, double mt){
    double wbuf[WBUFS];
    int wn, i, loop;
    double t, x, y, d;

    t = 0;
    loop = 1;
    d = f * (1.0/SAMP_RATE);
    y = vol / mt;
    while(n > 0 && loop){
	if(n >= WBUFS){
	    wn = WBUFS;
	}else{
	    wn = n;
	}
	if(t + wn * (1.0/SAMP_RATE) > mt){
	    wn = (mt - t) * SAMP_RATE;
	    loop = 0;
	}

	timbre_proc(t * f, d, wbuf, wn);

	for(i = 0; i < wn; i++){
	    x = wbuf[i] * y * (mt - t);
	    a[i] = (audio_word)VOLUME_RANGE(x);
	    t += (1.0/SAMP_RATE);
	}

	a += wn;
	n -= wn;
    }
    memset(a, 0, n * sizeof(audio_word));
}


static void gen_wave2(audio_word* a, int n, double f, double vol, double mt){
    double wbuf[WBUFS];
    int wn, i, loop;
    double t, x, y, d;

    t = 0;
    loop = 1;
    d = f * (1.0/SAMP_RATE);
    y = vol / mt;
    while(n > 0 && loop){
	if(n >= WBUFS){
	    wn = WBUFS;
	}else{
	    wn = n;
	}
	if(t + wn * (1.0/SAMP_RATE) > mt){
	    wn = (mt - t) * SAMP_RATE;
	    loop = 0;
	}

	timbre_proc(t * f, d, wbuf, wn);

	for(i = 0; i < wn; i++){
	    x = wbuf[i] * y * t;
	    a[i] = (audio_word)VOLUME_RANGE(x);
	    t += (1.0/SAMP_RATE);
	}

	a += wn;
	n -= wn;
    }
    memset(a, 0, n * sizeof(audio_word));
}

static void gen_wave3(audio_word* a, int n, double f, double vol, double mt){
    double wbuf[WBUFS];
    int wn, i;
    double t, x, y, d;

    t = 0;
    d = f * (1.0/SAMP_RATE);
    y = vol / mt;
    while(n > 0){
	if(n >= WBUFS){
	    wn = WBUFS;
	}else{
	    wn = n;
	}

	timbre_proc(t * f, d, wbuf, wn);

	for(i = 0; i < wn; i++){
	    x = wbuf[i] * y * (mt - t);
	    a[i] = (audio_word)VOLUME_RANGE(x);
	    t += (1.0/SAMP_RATE);
	    while(t >= mt){
		t -= mt;
	    }
	}
	a += wn;
	n -= wn;
    }
}

static void gen_wave4(audio_word* a, int n, double f, double vol, double mt){
    double wbuf[WBUFS];
    int i, j, wn;
    double t, x, y, d, tc, mt2;

    t = 0;
    d = f * (1.0/SAMP_RATE);
    i = 0;
    while(i < n){
	if(i + WBUFS < n){
	    wn = WBUFS;
	}else{
	    wn = n - i;
	}
	t = timbre_proc(t, d, wbuf, wn);
	for(j = 0; j < wn; j++){
	    a[i + j] = (audio_word)(wbuf[j] * AMAX);
	}
	i += wn;
    }

    tc = t = 0;
    i = 0;
    mt2 = mt * 2;
    y = vol / mt * (1.0/AMAX);
    while(i < n){
	for(; i < n; i++){
	    x = (double)a[i] * y * (mt - tc);
	    a[i] = (audio_word)VOLUME_RANGE(x);
	    t += 1.0 / SAMP_RATE;
	    tc += 1.0 / SAMP_RATE;
	    if(tc >= mt){
		break;
	    }
	}
	for(; i < n; i++){
	    x = (double)a[i] * y * (tc - mt);
	    a[i] = (audio_word)VOLUME_RANGE(x);
	    t += 1.0 / SAMP_RATE;
	    tc += 1.0 / SAMP_RATE;
	    if(tc >= mt2){
		tc -= mt2;
		break;
	    }
	}
    }
}

static void gen_wave5(audio_word* a, int n, double f, double vol, double mt){
    double wbuf[WBUFS];
    int wn, i, loop;
    double t, x, y, d;

    t = 0;
    loop = 1;
    d = f * (1.0/SAMP_RATE);
    y = vol / mt;
    while(n > 0 && loop){
	if(n >= WBUFS){
	    wn = WBUFS;
	}else{
	    wn = n;
	}
	if(t + wn * (1.0/SAMP_RATE) > mt){
	    wn = (mt - t) * SAMP_RATE;
	    loop = 0;
	}

	timbre_proc(t * f, d, wbuf, wn);

	for(i = 0; i < wn; i++){
	    x = wbuf[i] * y * (mt - t);
	    a[i] = (audio_word)VOLUME_RANGE(x);
	    t += (1.0/SAMP_RATE);
	}

	a += wn;
	n -= wn;
    }
    while(n > 0){
	if(n >= WBUFS){
	    wn = WBUFS;
	}else{
	    wn = n;
	}
	t = timbre_proc(t, d, wbuf, wn);
	for(i = 0; i < wn; i++){
	    x = vol * wbuf[i];
	    a[i] = (audio_word)VOLUME_RANGE(x);
	}
	n -= wn;
	a += wn;
    }
}

static void gen_wave6(audio_word* a, int n, double f, double vol, double mt){
    double wbuf[WBUFS];
    int wn, i;
    double t, x, y, d;

    t = 0;
    d = f * (1.0/SAMP_RATE);
    y = vol / mt;
    while(n > 0){
	if(n >= WBUFS){
	    wn = WBUFS;
	}else{
	    wn = n;
	}

	timbre_proc(t * f, d, wbuf, wn);

	for(i = 0; i < wn; i++){
	    x = wbuf[i] * y * t;
	    a[i] = (audio_word)VOLUME_RANGE(x);
	    t += (1.0/SAMP_RATE);
	    while(t >= mt){
		t -= mt;
	    }
	}
	a += wn;
	n -= wn;
    }
}

static void gen_wave7(audio_word* a, int n, double f, double vol, double mt){
    double wbuf[WBUFS];
    int wn, i, loop;
    double t, x, y, d;

    t = 0;
    loop = 1;
    d = f * (1.0/SAMP_RATE);
    while(n > 0 && loop){
	if(n >= WBUFS){
	    wn = WBUFS;
	}else{
	    wn = n;
	}
	if(t + wn * (1.0/SAMP_RATE) > mt){
	    wn = (mt - t) * SAMP_RATE;
	    loop = 0;
	}

	timbre_proc(t * f, d, wbuf, wn);

	y = vol / mt;
	for(i = 0; i < wn; i++){
	    x = wbuf[i] * y * t;
	    a[i] = (audio_word)VOLUME_RANGE(x);
	    t += (1.0/SAMP_RATE);
	}

	a += wn;
	n -= wn;
    }

    t *= f;
    while(n > 0){
	if(n >= WBUFS){
	    wn = WBUFS;
	}else{
	    wn = n;
	}
	t = timbre_proc(t, d, wbuf, wn);
	for(i = 0; i < wn; i++){
	    x = vol * wbuf[i];
	    a[i] = (audio_word)VOLUME_RANGE(x);
	}
	n -= wn;
	a += wn;
    }
}

static void gen_wave8(audio_word* a, int n, double f, double vol, double mt){
    double wbuf[WBUFS];
    int i, j, wn;
    double t, x, y, d, tc, mt2;

    t = 0;
    d = f * (1.0/SAMP_RATE);
    i = 0;
    while(i < n){
	if(i + WBUFS < n){
	    wn = WBUFS;
	}else{
	    wn = n - i;
	}
	t = timbre_proc(t, d, wbuf, wn);
	for(j = 0; j < wn; j++){
	    a[i + j] = (audio_word)(wbuf[j] * AMAX);
	}
	i += wn;
    }

    tc = t = 0;
    i = 0;
    mt2 = mt * 2;
    y = vol / mt * (1.0/AMAX);
    while(i < n){
	for(; i < n; i++){
	    x = (double)a[i] * y * tc;
	    a[i] = (audio_word)VOLUME_RANGE(x);
	    t += 1.0 / SAMP_RATE;
	    tc += 1.0 / SAMP_RATE;
	    if(tc >= mt){
		break;
	    }
	}
	for(; i < n; i++){
	    x = (double)a[i] * y * (mt2 - tc);
	    a[i] = (audio_word)VOLUME_RANGE(x);
	    t += 1.0 / SAMP_RATE;
	    tc += 1.0 / SAMP_RATE;
	    if(tc >= mt2){
		tc -= mt2;
		break;
	    }
	}
    }
}
