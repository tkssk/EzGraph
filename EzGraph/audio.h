#ifndef ___AUDIO_H_
#define ___AUDIO_H_

#define SAMP_RATE 8000
#define AMAX 127.0
typedef signed char audio_word;

void set_timbre(int y);
int audio_gen_wave(audio_word* a, /* wave_data */
		   int n,	/* number of wave_data */
		   double f,	/* frequency (Hz) */
		   double vol,	/* volume [0.0, 1.0]*/
		   unsigned short m, /* M: cycle */
		   char s);	     /* S: envelope pattern */

#endif /* ___AUDIO_H_ */

/*
 * envelope pattern:
 *
 * 0, 1, 2, 3, 9:
 * \
 *  \
 *   \_________________
 *
 *
 * 4, 5, 6, 7, 15:
 *
 *   /|
 *  / |
 * /  |________________
 *
 * 8:
 * \  |\  |\  |\  |\  
 *  \ | \ | \ | \ | \ 
 *   \|  \|  \|  \|  \
 *
 *
 * 10:
 * \    /\    /\    /
 *  \  /  \  /  \  / 
 *   \/    \/    \/  
 *
 * 11: ________________
 * \  |
 *  \ |
 *   \|
 *
 * 12:
 *   /|  /|  /|  /|  /|
 *  / | / | / | / | / |
 * /  |/  |/  |/  |/  |
 *
 * 13: __________________
 *    /
 *   /
 *  /
 *
 *
 * 14:
 *   /\    /\    /\    /\   
 *  /  \  /  \  /  \  /  \ 
 * /    \/    \/    \/    \
 *
 *
 * -1:
 * ________________________
 *
 *
 */
