/*
 * EzGraph: Easy Graphic Library
 * Copyright(C) 2013 T.Sasaki
 * Copyright(C) 2016 Y. Doi (network extention)
 */

#include <stdio.h>
#include <stdlib.h>
#ifdef UseICONV
#include <iconv.h>
#endif
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <locale.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/xpm.h>
#include <X11/Xft/Xft.h>
#ifdef HAVE_XDBE
#include <X11/extensions/Xdbe.h>
#endif
#ifdef HAVE_NETEXT
#include <signal.h>
#endif

#include "EzGraph.h"
#include "mmlp.h"

/*******************************************************************
 * Declaration
 *******************************************************************/

/* Variable for EzGraph Lib. */
static int init_flg = 0;
static int loop = 0;
static int EzWidth = 0, EzHeight = 0;

/* Font-set */
static char *curfontname = NULL; /* current font name */
static double curfontsize; /* current font size */
#ifdef UseICONV
static iconv_t icd; /* iconv handler */
#endif

/* Event handler */
static EzMouseHandler mouse_handler = NULL;
static EzMouseHandler mouse_rel_handler = NULL;
static EzMouseMotionHandler mouse_motion_handler = NULL;
static EzKeyHandler key_handler = NULL;
static EzKeyHandler key_rel_handler = NULL;
static EzTimerHandler timer_handler = NULL;
#ifdef HAVE_NETEXT
static EzConnectHandler accept_handler = NULL;
static EzConnectHandler receive_handler = NULL;
static EzConnectHandler close_handler = NULL;
#endif
static unsigned long timer_interval = 0;
static struct timeb timer_ctr;
static Atom atom1,atom2;

/* Variable for X11 Lib. */
static Display *disp;
static int scr;
static Window win;
static GC gc, cls_gc;
static Pixmap pmap_cur;
#ifdef HAVE_XDBE
static int use_xdbe = 0;
#endif
static unsigned long black, white;
static XftFont *deffont;
static Drawable bdl_buf;
#define COLOR_CACHE_ENT 64
static struct {
    XColor col;
    XftColor xftcol;
    unsigned short r, g, b;
    unsigned int valid;
} color_cache[COLOR_CACHE_ENT];
XftColor xftcol; /* Current XftColor */
XftDraw *xftdraw, *xft_pmap_cur, *xft_bdl_buf;

#ifdef HAVE_ALSA
static no_sound = 0;
#endif

/* Private Functions */
static void init_lib(void);

/*******************************************************************
 * Body
 *******************************************************************/

/*
 * Public Functions
 */

#define init() if(!init_flg) {init_lib();}

void EzEnableXdbe(void){
    if(init_flg){
	return;
    }
#ifdef HAVE_XDBE
    use_xdbe = 1;
#endif
}

int EzIsXdbe(void){
    init();

#ifdef HAVE_XDBE
    return use_xdbe;
#else
    return 0;
#endif
}

void EzOpenWindow(int width, int height){
    int depth;
    XEvent ev;
    XSizeHints hints;
#ifdef HAVE_XDBE
    int maj_ver, min_ver;
#endif
  
    if(init_flg){
	return;
    }
    EzWidth = width;
    EzHeight = height;

#ifdef HAVE_ALSA
    int pcm;

    if((pcm = audio_open()) < 0){
	no_sound = 1;
    }
#endif

    if(width < 32 || width > 4096){
	fputs("EzGraph: width of the window must be 32-4096.\n", stderr);
	exit(1);
    }
    if(height < 32 || height > 2048){
	fputs("EzGraph: height of the window must be 32-2048.\n", stderr);
	exit(1);
    }
    
    if(setlocale(LC_CTYPE, "") == NULL){
	fputs("EzGraph: can not set locale.\n", stderr);
	exit(1);
    }
    
    disp = XOpenDisplay(NULL);
    if(disp == NULL){
	fputs("EzGraph: can not open display.\n", stderr);
	exit(1);
    }

#ifdef HAVE_XDBE
    if(XdbeQueryExtension(disp, &maj_ver, &min_ver) == 0){
	fputs("EzGraph: can not use Xdbe extension.\n", stderr);
	use_xdbe = 0;
    }
#endif
    
    scr = DefaultScreen(disp);
    black = BlackPixel(disp, scr);
    white = WhitePixel(disp, scr);
    depth = DefaultDepth(disp, scr);
    if(depth != 16 && depth != 24){
	fputs("EzGraph: support only 16 and 24 depth.\n", stderr);
	exit(1);
    }
    win = XCreateSimpleWindow(disp, RootWindow(disp, scr),
			      0, 0, EzWidth, EzHeight, 1,
			      white, black);
    pmap_cur = XCreatePixmap(disp, win, EzWidth, EzHeight, depth);
#ifdef HAVE_XDBE
    if(use_xdbe){
	bdl_buf = XdbeAllocateBackBufferName(disp, win, XdbeBackground);
    }else{
#endif
	bdl_buf = XCreatePixmap(disp, win, EzWidth, EzHeight, depth);
#ifdef HAVE_XDBE
    }
#endif
    hints.flags = PMinSize | PMaxSize;
    hints.min_width = hints.max_width = EzWidth;
    hints.min_height = hints.max_height = EzHeight;
    XSetWMNormalHints(disp, win, &hints);
    XStoreName(disp, win, "EzX-MIE");
    XSetIconName(disp, win, "EzX-MIE");
    
    gc = XCreateGC(disp, win, 0, NULL);
    cls_gc = XCreateGC(disp, win, 0, NULL);
    XSetForeground(disp, gc, white);
    XSetBackground(disp, gc, black);
    XSetForeground(disp, cls_gc, black);
    XSetBackground(disp, cls_gc, black);
    
    if((curfontname = strdup(EzDefaultFontName)) == NULL){
	fputs("EzGraph: no more memory.\n", stderr);
	exit(1);
    }
    curfontsize = 72 * (EzDefaultFontSize) /
      (((double)DisplayHeight(disp,scr)*25.4) /
       (DisplayHeightMM(disp,scr)));
    deffont = XftFontOpen(disp, scr,
			  XFT_FAMILY, XftTypeString, curfontname,
			  XFT_SIZE, XftTypeDouble, curfontsize,
			  NULL);
    xftdraw = XftDrawCreate(disp, win, DefaultVisual(disp, scr),
			    DefaultColormap(disp, scr));
    xft_pmap_cur = XftDrawCreate(disp, pmap_cur, DefaultVisual(disp, scr),
			    DefaultColormap(disp, scr));
    xft_bdl_buf = XftDrawCreate(disp, bdl_buf, DefaultVisual(disp, scr),
			    DefaultColormap(disp, scr));
    XSelectInput(disp, win, ExposureMask |
		 ButtonPressMask | ButtonReleaseMask | ButtonMotionMask |
		 KeyPressMask | KeyRelease);
    XMapWindow(disp, win);
    
    XftColorAllocName(disp, DefaultVisual(disp, scr),
			 DefaultColormap(disp, scr), "white", &xftcol);

#ifdef UseICONV
#ifndef EzCharCode
#error Set EzCharCode as "EUC-JP" or "SHIFT-JIS"
#endif
#define __to_str__(__A__) #__A__
#define __to_str(__A__) __to_str__(__A__)
    if((icd = iconv_open("UTF-8", __to_str(EzCharCode))) == (iconv_t)-1){
	fprintf(stderr,
		"EzGraph: cannot open iconv (%s).\n", __to_str(EzCharCode));
	exit(1);
    }
#undef __to_str__
#endif

    init_flg = 1;
    
    XFlush(disp);
    while(1){
	XNextEvent(disp, &ev);
	if(ev.type == Expose){
	    break;
	}
    }
    /* throw away events on the stack */
    usleep(200000);
    while(XPending(disp) > 0){
	XNextEvent(disp, &ev);
    }

    XSetForeground(disp, gc, black);
    XFillRectangle(disp, pmap_cur, gc, 0, 0, EzWidth, EzHeight);
#ifdef HAVE_XDBE
    if(!use_xdbe){
#endif
	XFillRectangle(disp, bdl_buf, gc, 0, 0, EzWidth, EzHeight);
#ifdef HAVE_XDBE
    }
#endif
    XSetForeground(disp, gc, white);
    atom1 = XInternAtom(disp, "WM_PROTOCOLS", False);
    atom2 = XInternAtom(disp, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(disp, win, &atom2,1);
}

int EzDumpWindow(char *file){
    FILE *fp;
    XImage *image;
    int x, y;
    unsigned char *p, r, g, b;

    if((fp=fopen(file, "w")) == NULL){
	return -1;
    }
    image = XGetImage(disp, win, 0, 0, EzWidth, EzHeight, AllPlanes, ZPixmap);
    fprintf(fp, "P3\n#\n%d %d\n255\n", EzWidth, EzHeight);
    p = (char *)image->data;
    for(y=0; y<EzHeight; y++){
	for(x=0; x<EzWidth; x++){
	    r = p[2];
	    g = p[1];
	    b = p[0];
	    fprintf(fp, " %u %u %u\n", r, g, b);
	    p+= 4;
	}
    }

    XDestroyImage(image);
    fclose(fp);
    return 0;
}

void EzDrawPoint(int x, int y){
    init();
    XDrawPoint(disp, win, gc, x, y);
    XDrawPoint(disp, pmap_cur, gc, x, y);
    XFlush(disp);
}

void EzDrawPointB(int x, int y){
    init();
    XDrawPoint(disp, bdl_buf, gc, x, y);
}

void EzDrawLine(int x1, int y1, int x2, int y2){
    init();
    XDrawLine(disp, win, gc, x1, y1, x2, y2);
    XDrawLine(disp, pmap_cur, gc, x1, y1, x2, y2);
    XFlush(disp);
}

void EzDrawLineB(int x1, int y1, int x2, int y2){
    init();
    XDrawLine(disp, bdl_buf, gc, x1, y1, x2, y2);
}

void EzDrawBox(int x1, int y1, int x2, int y2){
    int w = x2 - x1, h = y2 - y1;

    init();
    XDrawRectangle(disp, win, gc, x1, y1, w, h);
    XDrawRectangle(disp, pmap_cur, gc, x1, y1, w, h);
    XFlush(disp);
}

void EzDrawBoxB(int x1, int y1, int x2, int y2){
    int w = x2 - x1, h = y2 - y1;

    init();
    XDrawRectangle(disp, bdl_buf, gc, x1, y1, w, h);
}

void EzFillBox(int x1, int y1, int x2, int y2){
    int w = x2 - x1 + 1, h = y2 - y1 + 1;

    init();
    XFillRectangle(disp, win, gc, x1, y1, w, h);
    XFillRectangle(disp, pmap_cur, gc, x1, y1, w, h);
    XFlush(disp);
}

void EzFillBoxB(int x1, int y1, int x2, int y2){
    int w = x2 - x1 + 1, h = y2 - y1 + 1;

    init();
    XFillRectangle(disp, bdl_buf, gc, x1, y1, w, h);
}

void EzDrawCircle(int x, int y, unsigned int r){
    int x0 = x - r, y0 = y - r, r2 = 2 * r;

    init();
    XDrawArc(disp, win, gc, x0, y0, r2, r2, 0, 360 * 64);
    XDrawArc(disp, pmap_cur, gc, x0, y0, r2, r2, 0, 360 * 64);
    XFlush(disp);
}

void EzDrawCircleB(int x, int y, unsigned int r){
    int x0 = x - r, y0 = y - r, r2 = 2 * r;

    init();
    XDrawArc(disp, bdl_buf, gc, x0, y0, r2, r2, 0, 360 * 64);
}

void EzFillCircle(int x, int y, unsigned int r){
    int x0 = x - r, y0 = y - r, r2 = 2 * r;

    init();
    XFillArc(disp, win, gc, x0, y0, r2, r2, 0, 360 * 64);
    XFillArc(disp, pmap_cur, gc, x0, y0, r2, r2, 0, 360 * 64);
    XFlush(disp);
}

void EzFillCircleB(int x, int y, unsigned int r){
    int x0 = x - r, y0 = y - r, r2 = 2 * r;

    init();
    XFillArc(disp, bdl_buf, gc, x0, y0, r2, r2, 0, 360 * 64);
}

void EzDrawArc(int x, int y,
	       unsigned int rx, unsigned int ry,
	       int t1, int t2){
    int x0 = x - rx, y0 = y - ry;
    int rx2 = 2 * rx, ry2 = 2 * ry;
    int t1x = 64 * t1, t2x = 64 * (t2 - t1);

    init();
    XDrawArc(disp, win, gc, x0, y0, rx2, ry2, t1x, t2x);
    XDrawArc(disp, pmap_cur, gc, x0, y0, rx2, ry2, t1x, t2x);
    XFlush(disp);
}

void EzDrawArcB(int x, int y,
	       unsigned int rx, unsigned int ry,
	       int t1, int t2){
    int x0 = x - rx, y0 = y - ry;
    int rx2 = 2 * rx, ry2 = 2 * ry;
    int t1x = 64 * t1, t2x = 64 * (t2 - t1);

    init();
    XDrawArc(disp, bdl_buf, gc, x0, y0, rx2, ry2, t1x, t2x);
}

void EzFillArc(int x, int y,
	       unsigned int rx, unsigned int ry,
	       int t1, int t2){
    int x0 = x - rx, y0 = y - ry;
    int rx2 = 2 * rx, ry2 = 2 * ry;
    int t1x = 64 * t1, t2x = 64 * (t2 - t1);

    init();
    XFillArc(disp, win, gc, x0, y0, rx2, ry2, t1x, t2x);
    XFillArc(disp, pmap_cur, gc, x0, y0, rx2, ry2, t1x, t2x);
    XFlush(disp);
}

void EzFillArcB(int x, int y,
		unsigned int rx, unsigned int ry,
		int t1, int t2){
    int x0 = x - rx, y0 = y - ry;
    int rx2 = 2 * rx, ry2 = 2 * ry;
    int t1x = 64 * t1, t2x = 64 * (t2 - t1);

    init();
    XFillArc(disp, bdl_buf, gc, x0, y0, rx2, ry2, t1x, t2x);
}

void EzFillPolygon(int num, int *poly){
    XPoint *xpoints, *xptr;
    int i;

    init();
    if((xpoints =(XPoint *)malloc(num * sizeof(XPoint))) == NULL){
	fprintf(stderr, "EzGraph: no more memory.\n");
	exit(-1);
    }
    xptr = xpoints;
    for(i=0; i<num; i++){
	xptr->x = *(poly++);
	xptr->y = *(poly++);
	xptr++;
    }
    XFillPolygon(disp, win, gc, xpoints, num, Complex, CoordModeOrigin);
    XFillPolygon(disp, pmap_cur, gc, xpoints, num, Complex, CoordModeOrigin);
    XFlush(disp);
    free(xpoints);
}

void EzFillPolygonB(int num, int *poly){
    XPoint *xpoints, *xptr;
    int i;

    init();
    if((xpoints =(XPoint *)malloc(num * sizeof(XPoint))) == NULL){
	fprintf(stderr, "EzGraph: no more memory.\n");
	exit(-1);
    }
    xptr = xpoints;
    for(i=0; i<num; i++){
	xptr->x = *(poly++);
	xptr->y = *(poly++);
	xptr++;
    }
    XFillPolygon(disp, bdl_buf, gc, xpoints, num, Complex, CoordModeOrigin);
    free(xpoints);
}

#ifndef UseICONV
void EzDrawString(int x, int y, const char *s){
    init();
    XftDrawStringUtf8(xftdraw, &xftcol, deffont, x, y,
		      (FcChar8 *)s, strlen(s));
    XftDrawStringUtf8(xft_pmap_cur, &xftcol, deffont, x, y,
		      (FcChar8 *)s, strlen(s));
    XFlush(disp);
}

void EzDrawStringB(int x, int y, const char *s){
    init();
    XftDrawStringUtf8(xft_bdl_buf, &xftcol, deffont, x, y,
		      (FcChar8 *)s, strlen(s));
}
#else
void EzDrawString(int x, int y, const char *s){
    char *dst_buf;
    char *src_ptr, *dst_ptr;
    size_t src_len, dst_len;

    init();
    src_ptr = (char *)s;
    src_len = strlen(s);
    dst_buf = malloc(src_len*3+6);
    dst_ptr = dst_buf;
    dst_len = src_len*3+6;
    dst_len = iconv(icd, &src_ptr, &src_len, &dst_ptr, &dst_len);
    if(dst_len == -1){
	free(dst_buf);
	return;
    }
    *dst_ptr = '\0';

    XftDrawStringUtf8(xftdraw, &xftcol, deffont, x, y,
		      (FcChar8 *)dst_buf, strlen(dst_buf));
    XftDrawStringUtf8(xft_pmap_cur, &xftcol, deffont, x, y,
		      (FcChar8 *)dst_buf, strlen(dst_buf));
    free(dst_buf);
    XFlush(disp);
}

void EzDrawStringB(int x, int y, const char *s){
    char *dst_buf;
    char *src_ptr, *dst_ptr;
    size_t src_len, dst_len;

    init();
    src_ptr = (char *)s;
    src_len = strlen(s);
    dst_buf = malloc(src_len*3+6);
    dst_ptr = dst_buf;
    dst_len = src_len*3+6;
    dst_len = iconv(icd, &src_ptr, &src_len, &dst_ptr, &dst_len);
    if(dst_len == -1){
	free(dst_buf);
	return;
    }
    *dst_ptr = '\0';
    XftDrawStringUtf8(xft_bdl_buf, &xftcol, deffont, x, y,
		      (FcChar8 *)dst_buf, strlen(dst_buf));
    free(dst_buf);
}
#endif

void EzClear(void){
    init();
    XClearWindow(disp, win);
    XFillRectangle(disp, pmap_cur, cls_gc, 0, 0, EzWidth, EzHeight);
}

void EzShowBuffer(void){
    int y;
#ifdef HAVE_XDBE
    XdbeSwapInfo swap_info;

    init();

    swap_info.swap_window = win;
    swap_info.swap_action = XdbeBackground;

    if(use_xdbe){
	XdbeSwapBuffers(disp, &swap_info, 1);
    }else{
#endif
	/* I don't know why, but the following code (for version)
	 * is faster than the single XCopyArea version.
	 *
	 * XCopyArea(disp, bdl_buf, win, gc, 0, 0, EzWidth, EzHeight, 0, 0);
	 */
	for(y=0; y<EzHeight; y++){
	    XCopyArea(disp, bdl_buf, win, gc, 0, y, EzWidth, 1, 0, y);
	}
	XCopyArea(disp, bdl_buf, pmap_cur, gc, 0, 0, EzWidth, EzHeight, 0, 0);
#ifdef HAVE_XDBE
    }
#endif
    XFlush(disp);
#ifdef HAVE_XDBE
    if(!use_xdbe){
#endif
	XFillRectangle(disp, bdl_buf, cls_gc, 0, 0, EzWidth, EzHeight);
#ifdef HAVE_XDBE
    }
#endif
}

int EzSetColorByRGB(unsigned short r, unsigned short g, unsigned short b){
    int i;
    int idx;
    XColor col;
    XRenderColor xrcolor;
    static unsigned short _r=1, _g=1, _b=1;

    init();
    col.red = (r > 255 ? 65535 : r*257);
    col.green = (g > 255 ? 65535 : g*257);
    col.blue = (b > 255 ? 65535 : b*257);

    xrcolor.red = col.red;
    xrcolor.green = col.green;
    xrcolor.blue = col.blue;
    xrcolor.alpha = 0xffff;

    if(_r == col.red && _g == col.green && _b == col.blue){
	return 0;
    }

    idx = ((col.red+31) ^ (col.green+22) ^ (col.blue+99)) % COLOR_CACHE_ENT;
    if(color_cache[idx].valid == 1 &&
       color_cache[idx].r == col.red &&
       color_cache[idx].g == col.green &&
       color_cache[idx].b == col.blue){
	col = color_cache[idx].col;
	xftcol = color_cache[idx].xftcol;
    }else{
	if(XAllocColor(disp, DefaultColormap(disp, scr), &col) == 0){
	    return -1;
	}
	if(XftColorAllocValue(disp, DefaultVisual(disp, scr),
			      DefaultColormap(disp, scr),
			      &xrcolor, &xftcol) == 0){
	    return 1;
	}
	color_cache[idx].col = col;
	color_cache[idx].xftcol = xftcol;
	color_cache[idx].r = col.red;
	color_cache[idx].g = col.green;
	color_cache[idx].b = col.blue;
	color_cache[idx].valid = 1;
    }
    XSetForeground(disp, gc, col.pixel);
    return 0;
}

int EzSetColorByName(const char *name){
    XColor n_col, t_col;

    init();
    if(XAllocNamedColor(disp, DefaultColormap(disp, scr),
			name, &n_col, &t_col) == 0){
	return -1;
    }
    XSetForeground(disp, gc, n_col.pixel);
    if(XftColorAllocName(disp, DefaultVisual(disp, scr),
			 DefaultColormap(disp, scr),
			 name, &xftcol) == 0){
	return -1;
    }

    return 0;
}

int EzSetFont(const char *fn){
    char *oldfontname;
    XftFont *font;

    init();
    oldfontname = curfontname;
    font = XftFontOpen(disp, scr,
			  XFT_FAMILY, XftTypeString, fn,
			  XFT_SIZE, XftTypeDouble, curfontsize,
			  NULL);
    if(font == NULL){
	return -1;
    }
    if((curfontname = strdup(fn)) == NULL){
	return -1;
    }
    XftFontClose(disp, deffont);
    deffont = font;
    free(oldfontname);
    return 0;
}

int EzSetFontSize(double fs){
    double oldfontsize;
    XftFont *font;

    init();
    oldfontsize = curfontsize;
    fs *= 72.0 /
      ((((double)DisplayHeight(disp,scr))*25.4) / 
       (DisplayHeightMM(disp,scr)));

    font = XftFontOpen(disp, scr,
			  XFT_FAMILY, XftTypeString, curfontname,
			  XFT_SIZE, XftTypeDouble, fs,
			  NULL);
    if(font == NULL){
	return -1;
    }
    curfontsize = fs;
    XftFontClose(disp, deffont);
    deffont = font;
    return 0;
}

void EzSetMouseHandler(EzMouseHandler bh){
    mouse_handler = bh;
}

void EzSetMouseReleaseHandler(EzMouseHandler bh){
    mouse_rel_handler = bh;
}

void EzSetMouseMotionHandler(EzMouseMotionHandler bh){
    mouse_motion_handler = bh;
}

void EzSetKeyHandler(EzKeyHandler kh){
    key_handler = kh;
}

void EzSetKeyReleaseHandler(EzKeyHandler kh){
    key_rel_handler = kh;
}

void EzSetTimerHandler(EzTimerHandler th, unsigned long msec){
    init();
    timer_handler = th;
    timer_interval = msec;
    ftime(&timer_ctr);
    msec += timer_ctr.millitm;
    timer_ctr.time += msec/1000;
    timer_ctr.millitm = msec % 1000;
}

#ifdef HAVE_NETEXT
void EzSetAcceptHandler(EzConnectHandler ah){
    accept_handler = ah;
}
void EzSetConnectionCloseHandler(EzConnectHandler cch){
    close_handler = cch;
}
void EzSetReceiveHandler(EzConnectHandler rh){
    receive_handler = rh;
}

extern void EzCheckConnection(EzConnectHandler,
                              EzConnectHandler);
extern FILE* EzAccept(void);
#endif

void EzEventLoop(void){
    XEvent ev;
    KeySym keysym;
    struct timeb cur;
    int num;
    char buf[10];
    int expose;
    
    init();
    if(loop){
	/* Nested function call */
	fprintf(stderr,
		"Can not call EzEventLoop() function in an event "
		"handler function.\n");
	exit(1);
    }
    loop = 1;
    while(loop){
	expose = 0;
	while(XPending(disp) > 0){
	    XNextEvent(disp, &ev);
	    switch(ev.type){
	      case Expose:
		expose = 1;
		break;
	      case MotionNotify:
		if(mouse_motion_handler != NULL){
		    mouse_motion_handler(ev.xmotion.x, ev.xmotion.y);
		}
		break;
	      case ButtonPress:
		if(mouse_handler != NULL){
		    mouse_handler(ev.xbutton.x,
				   ev.xbutton.y,
				   ev.xbutton.button);
		}else{
		    if(ev.xbutton.button == 3){
			exit(0);
		    }
		}
		break;
	      case ButtonRelease:
		if(mouse_rel_handler != NULL){
		    mouse_rel_handler(ev.xbutton.x,
				   ev.xbutton.y,
				   ev.xbutton.button);
		}
		break;
	      case KeyPress:
		num = XLookupString((XKeyEvent *)&ev, buf, 10, &keysym, NULL);
		if(key_handler != NULL){
		    if(num == 0){
			key_handler((int)keysym);
		    }else{
			key_handler(buf[0]);
		    }
		}else{
		    if(buf[0] == 'q'){
			exit(0);
		    }
		}
		break;
	      case KeyRelease:
		num = XLookupString((XKeyEvent *)&ev, buf, 10, &keysym, NULL);
		if(key_rel_handler != NULL){
		    if(num == 0){
			key_rel_handler((int)keysym);
		    }else{
			key_rel_handler(buf[0]);
		    }
		}
		break;
	      case ClientMessage:
		if(ev.xclient.message_type == atom1 &&
		   ev.xclient.data.l[0] == atom2){
		    XCloseDisplay(disp);
		    exit(0);
		}
		break;
	    }
	}
	if(expose){
	    XCopyArea(disp, pmap_cur, win, gc,
		      0, 0, EzWidth, EzHeight, 0, 0);
	}
#ifdef HAVE_NETEXT
	if(accept_handler != NULL){
	    FILE* connect = EzAccept();
	    if(connect != NULL){
		accept_handler(connect);
	    }
	}
	EzCheckConnection(close_handler, receive_handler);
#endif
	if(timer_handler != NULL){
	    unsigned long msec;
	    ftime(&cur);
	    if(cur.time > timer_ctr.time ||
	             (cur.time == timer_ctr.time &&
		      cur.millitm >= timer_ctr.millitm)){
		msec = cur.millitm+timer_interval;
		timer_ctr.time = cur.time+msec/1000;
		timer_ctr.millitm = msec % 1000;
		timer_handler();
	    }
	}
	usleep(1000);
    }
}

long EzGetTimeLeft(void){
    long msec;
    struct timeb cur;

    ftime(&cur);

    msec = timer_ctr.time - cur.time;
    msec *= 1000;
    msec += (timer_ctr.millitm - cur.millitm);
    return msec;
}
    
void EzExitEventLoop(void){
    loop = 0;
}

int EzQueryKeymap(char keys[32]){
    Window w;
    int i;

    init();
    XGetInputFocus(disp, &w, &i);
    if(w == win){
	return XQueryKeymap(disp, keys);
    }else{
	memset(keys, 0x00, 32);
	return 0;
    }
}

int EzIsKeyPress(char keys[32], int ez_code){
    int code;

    init();
    code = XKeysymToKeycode(disp, ez_code) & 0x00ff;
    if(keys[(code >> 3)] & (1<<(code & 0x07))){
	return 1;
    }
    return 0;
}

EzImage EzReadXPM(char **data){
    EzImage ei;
    Pixmap pm_image, pm_mask;
    int w, h;

    init();
    XpmCreatePixmapFromData(disp, win, data, &pm_image, &pm_mask, NULL);
    sscanf(*data, "%d %d", &w, &h);
    ei.width = w;
    ei.height = h;
    ei.image = (unsigned long)pm_image;
    ei.mask = (unsigned long)pm_mask;
    return ei;
}

void EzPut(int x, int y, EzImage ez_image){
    init();
    if(ez_image.mask != 0){
	XSetClipMask(disp, gc, (Pixmap)ez_image.mask);
	XSetClipOrigin(disp, gc, x, y);
    }
    XCopyArea(disp, (Pixmap)ez_image.image, win, gc,
	      0, 0, ez_image.width, ez_image.height, x, y);
    XCopyArea(disp, (Pixmap)ez_image.image, pmap_cur, gc,
	      0, 0, ez_image.width, ez_image.height, x, y);
    XSetClipMask(disp, gc, None);
    XFlush(disp);
    return;
}

void EzPutB(int x, int y, EzImage ez_image){
    init();
    if(ez_image.mask != 0){
	XSetClipMask(disp, gc, (Pixmap)ez_image.mask);
	XSetClipOrigin(disp, gc, x, y);
    }
    XCopyArea(disp, (Pixmap)ez_image.image, bdl_buf, gc,
	      0, 0, ez_image.width, ez_image.height, x, y);
    XFlush(disp);
    XSetClipMask(disp, gc, None);
    return;
}

void EzSetMML(const char *mml){
#ifdef HAVE_ALSA
    init();
    if(no_sound){
	return;
    }
    setmmltext(mml);
#endif

}

void EzPlayMML(void){
#ifdef HAVE_ALSA
    init();
    if(no_sound){
	return;
    }
    mml_play();
#endif
}

void EzStopMML(void){
#ifdef HAVE_ALSA
    init();
    if(no_sound){
	return;
    }
    mml_stop();
#endif
}

/*
 * Private Functions
 */

static void init_lib(void){
#ifdef HAVE_NETEXT
    signal(SIGPIPE, SIG_IGN);
#endif
    EzOpenWindow(EzDefaultWidth, EzDefaultHeight);
}
