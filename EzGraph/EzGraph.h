#ifndef _EzGraph_H_
#define _EzGraph_H_

/*
 * EzGraph: Easy Graphic Library
 * Copyright(C) 2013-2016 T.Sasaki
 * Copyright(C) 2016 Y. Doi (network extention)
 */

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define EzMajorVer 3
#define EzMinorVer 0
#define EzPatchLev 1

#define EzDefaultWidth 600
#define EzDefaultHeight 600

#define EzDefaultFontName "serif"
#define EzDefaultFontSize 16.0

typedef struct EzImage_t {
    int width, height;
    unsigned long image, mask;
} EzImage;

typedef void(* EzMouseHandler)(int, int, unsigned int);
typedef void(* EzMouseMotionHandler)(int, int);
typedef void(* EzKeyHandler)(int);
typedef void(* EzTimerHandler)(void);
typedef void(* EzConnectHandler)(FILE*);

void EzEnableXdbe(void);
int EzIsXdbe(void);

void EzOpenWindow(int, int);

void EzDrawPoint(int, int);
void EzDrawPointB(int, int);
void EzDrawLine(int, int, int, int);
void EzDrawLineB(int, int, int, int);
void EzDrawBox(int, int, int, int);
void EzDrawBoxB(int, int, int, int);
void EzFillBox(int, int, int, int);
void EzFillBoxB(int, int, int, int);
void EzDrawCircle(int, int, unsigned int);
void EzDrawCircleB(int, int, unsigned int);
void EzFillCircle(int, int, unsigned int);
void EzFillCircleB(int, int, unsigned int);
void EzDrawArc(int, int, unsigned int, unsigned int, int, int);
void EzDrawArcB(int, int, unsigned int, unsigned int, int, int);
void EzFillArc(int, int, unsigned int, unsigned int, int, int);
void EzFillArcB(int, int, unsigned int, unsigned int, int, int);
void EzFillPolygon(int, int *);
void EzFillPolygonB(int, int *);
void EzDrawString(int, int, const char *);
void EzDrawStringB(int, int, const char *);
void EzClear(void);
void EzShowBuffer(void);

int EzSetColorByRGB(unsigned short, unsigned short, unsigned short);
int EzSetColorByName(const char *);

int EzSetFont(const char *);
int EzSetFontSize(double);

void EzSetMouseHandler(EzMouseHandler);
void EzSetMouseReleaseHandler(EzMouseHandler);
void EzSetMouseMotionHandler(EzMouseMotionHandler);
void EzSetKeyHandler(EzKeyHandler);
void EzSetKeyReleaseHandler(EzKeyHandler);
void EzSetTimerHandler(EzTimerHandler, unsigned long);
void EzEventLoop(void);
long EzGetTimeLeft(void);
void EzExitEventLoop(void);

int EzQueryKeymap(char [32]);
int EzIsKeyPress(char [32], int);
EzImage EzReadXPM(char **);
void EzPut(int, int, EzImage);
void EzPutB(int, int, EzImage);

void EzSetMML(const char *);
void EzPlayMML(void);
void EzStopMML(void);

void EzSetAcceptHandler(EzConnectHandler);
void EzSetReceiveHandler(EzConnectHandler);
void EzSetConnectionCloseHandler(EzConnectHandler);
int EzListen(unsigned int);
void EzListenClose(void);
FILE* EzConnect(const char*, unsigned int);
void EzMonitor(FILE*);

/* EzGetKey */
#define EZ_Left       0xff51  /* Move left, left arrow */
#define EZ_Up         0xff52  /* Move up, up arrow */
#define EZ_Right      0xff53  /* Move right, right arrow */
#define EZ_Down       0xff54  /* Move down, down arrow */
#define EZ_BackSpace  0xff08  /* Back space, back char */
#define EZ_Tab        0xff09  /* Tab */
#define EZ_Return     0xff0d  /* Return, enter */
#define EZ_Escape     0xff1b  /* Escape */
#define EZ_Delete     0xffff  /* Delete, rubout */
#define EZ_Home       0xff50  /* Home */
#define EZ_Page_Up    0xff55  /* Page Up */
#define EZ_Page_Down  0xff56  /* Page Down */
#define EZ_Insert     0xff63  /* Insert, insert here */
#define EZ_End        0xff57  /* EOL */
#define EZ_Begin      0xff58  /* BOL */

#define EZ_KP_Multiply 0xffaa /* Numeric Keypad '*' */
#define EZ_KP_Add     0xffab /* Numeric Keypad '+' */
#define EZ_KP_Separator 0xffac  /* Separator, often comma */
#define EZ_KP_Subtract 0xffad /* Numeric Keypad '-' */
#define EZ_KP_Decimal 0xffae 
#define EZ_KP_Divide  0xffaf /* Numeric Keypad '/' */
#define EZ_KP_0       0xffb0 /* Numeric Keypad 0 */
#define EZ_KP_1       0xffb1 /* Numeric Keypad 1 */
#define EZ_KP_2       0xffb2 /* Numeric Keypad 2 */
#define EZ_KP_3       0xffb3 /* Numeric Keypad 3 */
#define EZ_KP_4       0xffb4 /* Numeric Keypad 4 */
#define EZ_KP_5       0xffb5 /* Numeric Keypad 5 */
#define EZ_KP_6       0xffb6 /* Numeric Keypad 6 */
#define EZ_KP_7       0xffb7 /* Numeric Keypad 7 */
#define EZ_KP_8       0xffb8 /* Numeric Keypad 8 */
#define EZ_KP_9       0xffb9 /* Numeric Keypad 9 */
#define EZ_KP_Enter   0xff8d /* Numeric Keypad Enter */

#define EZ_F1         0xffbe
#define EZ_F2         0xffbf
#define EZ_F3         0xffc0
#define EZ_F4         0xffc1
#define EZ_F5         0xffc2
#define EZ_F6         0xffc3
#define EZ_F7         0xffc4
#define EZ_F8         0xffc5
#define EZ_F9         0xffc6
#define EZ_F10        0xffc7
#define EZ_F11        0xffc8
#define EZ_F12        0xffc9
#define EZ_Shift_L    0xffe1  /* Left shift */
#define EZ_Shift_R    0xffe2  /* Right shift */
#define EZ_Control_L  0xffe3  /* Left control */
#define EZ_Control_R  0xffe4  /* Right control */
#define EZ_Caps_Lock  0xffe5  /* Caps lock */
#define EZ_Shift_Lock 0xffe6  /* Shift lock */
#define EZ_Meta_L     0xffe7  /* Left meta */
#define EZ_Meta_R     0xffe8  /* Right meta */
#define EZ_Alt_L      0xffe9  /* Left alt */
#define EZ_Alt_R      0xffea  /* Right alt */

#define EZ_space      0x0020  /* U+0020 SPACE */
#define EZ_exclam     0x0021  /* U+0021 EXCLAMATION MARK */
#define EZ_quotedbl   0x0022  /* U+0022 QUOTATION MARK */
#define EZ_numbersign 0x0023  /* U+0023 NUMBER SIGN */
#define EZ_dollar     0x0024  /* U+0024 DOLLAR SIGN */
#define EZ_percent    0x0025  /* U+0025 PERCENT SIGN */
#define EZ_ampersand  0x0026  /* U+0026 AMPERSAND */
#define EZ_apostrophe 0x0027  /* U+0027 APOSTROPHE */
#define EZ_quoteright 0x0027  /* deprecated */
#define EZ_parenleft  0x0028  /* U+0028 LEFT PARENTHESIS */
#define EZ_parenright 0x0029  /* U+0029 RIGHT PARENTHESIS */
#define EZ_asterisk   0x002a  /* U+002A ASTERISK */
#define EZ_plus       0x002b  /* U+002B PLUS SIGN */
#define EZ_comma      0x002c  /* U+002C COMMA */
#define EZ_minus      0x002d  /* U+002D HYPHEN-MINUS */
#define EZ_period     0x002e  /* U+002E FULL STOP */
#define EZ_slash      0x002f  /* U+002F SOLIDUS */
#define EZ_0          0x0030  /* U+0030 DIGIT ZERO */
#define EZ_1          0x0031  /* U+0031 DIGIT ONE */
#define EZ_2          0x0032  /* U+0032 DIGIT TWO */
#define EZ_3          0x0033  /* U+0033 DIGIT THREE */
#define EZ_4          0x0034  /* U+0034 DIGIT FOUR */
#define EZ_5          0x0035  /* U+0035 DIGIT FIVE */
#define EZ_6          0x0036  /* U+0036 DIGIT SIX */
#define EZ_7          0x0037  /* U+0037 DIGIT SEVEN */
#define EZ_8          0x0038  /* U+0038 DIGIT EIGHT */
#define EZ_9          0x0039  /* U+0039 DIGIT NINE */
#define EZ_colon      0x003a  /* U+003A COLON */
#define EZ_semicolon  0x003b  /* U+003B SEMICOLON */
#define EZ_less       0x003c  /* U+003C LESS-THAN SIGN */
#define EZ_equal      0x003d  /* U+003D EQUALS SIGN */
#define EZ_greater    0x003e  /* U+003E GREATER-THAN SIGN */
#define EZ_question   0x003f  /* U+003F QUESTION MARK */
#define EZ_at         0x0040  /* U+0040 COMMERCIAL AT */
#define EZ_A          0x0041  /* U+0041 LATIN CAPITAL LETTER A */
#define EZ_B          0x0042  /* U+0042 LATIN CAPITAL LETTER B */
#define EZ_C          0x0043  /* U+0043 LATIN CAPITAL LETTER C */
#define EZ_D          0x0044  /* U+0044 LATIN CAPITAL LETTER D */
#define EZ_E          0x0045  /* U+0045 LATIN CAPITAL LETTER E */
#define EZ_F          0x0046  /* U+0046 LATIN CAPITAL LETTER F */
#define EZ_G          0x0047  /* U+0047 LATIN CAPITAL LETTER G */
#define EZ_H          0x0048  /* U+0048 LATIN CAPITAL LETTER H */
#define EZ_I          0x0049  /* U+0049 LATIN CAPITAL LETTER I */
#define EZ_J          0x004a  /* U+004A LATIN CAPITAL LETTER J */
#define EZ_K          0x004b  /* U+004B LATIN CAPITAL LETTER K */
#define EZ_L          0x004c  /* U+004C LATIN CAPITAL LETTER L */
#define EZ_M          0x004d  /* U+004D LATIN CAPITAL LETTER M */
#define EZ_N          0x004e  /* U+004E LATIN CAPITAL LETTER N */
#define EZ_O          0x004f  /* U+004F LATIN CAPITAL LETTER O */
#define EZ_P          0x0050  /* U+0050 LATIN CAPITAL LETTER P */
#define EZ_Q          0x0051  /* U+0051 LATIN CAPITAL LETTER Q */
#define EZ_R          0x0052  /* U+0052 LATIN CAPITAL LETTER R */
#define EZ_S          0x0053  /* U+0053 LATIN CAPITAL LETTER S */
#define EZ_T          0x0054  /* U+0054 LATIN CAPITAL LETTER T */
#define EZ_U          0x0055  /* U+0055 LATIN CAPITAL LETTER U */
#define EZ_V          0x0056  /* U+0056 LATIN CAPITAL LETTER V */
#define EZ_W          0x0057  /* U+0057 LATIN CAPITAL LETTER W */
#define EZ_X          0x0058  /* U+0058 LATIN CAPITAL LETTER X */
#define EZ_Y          0x0059  /* U+0059 LATIN CAPITAL LETTER Y */
#define EZ_Z          0x005a  /* U+005A LATIN CAPITAL LETTER Z */
#define EZ_bracketleft 0x005b  /* U+005B LEFT SQUARE BRACKET */
#define EZ_backslash  0x005c  /* U+005C REVERSE SOLIDUS */
#define EZ_bracketright 0x005d  /* U+005D RIGHT SQUARE BRACKET */
#define EZ_asciicircum 0x005e  /* U+005E CIRCUMFLEX ACCENT */
#define EZ_underscore 0x005f  /* U+005F LOW LINE */
#define EZ_grave      0x0060  /* U+0060 GRAVE ACCENT */
#define EZ_quoteleft  0x0060  /* Deprecated */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _EzGraph_H_ */
