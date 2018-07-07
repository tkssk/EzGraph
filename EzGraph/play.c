#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mmlp.h"
#include "audio.h"
#include "taiko1.h"

#define BUFFER_SIZE (8192 * 64)
#define PART_BUFFER_SIZE (8192 * 16)

typedef short buffer_type;

static int last_buffer;
static int buffer_ptr;
static int buffer_size = BUFFER_SIZE;
static buffer_type* buffer;
static int part_buffer_size = PART_BUFFER_SIZE;
static audio_word* part_buffer;
static char* play_buffer;
static int play_buffer_size;

void clear_play_buffer(void){
    static int init;

    if(!init){
	init = 1;
	buffer = (buffer_type *)xmalloc(BUFFER_SIZE * sizeof(buffer_type));
	part_buffer = (audio_word *)xmalloc(PART_BUFFER_SIZE);
    }
    buffer_ptr = 0;
    last_buffer = 0;
}

void start_position_play(void){
    buffer_ptr = 0;
}

static INLINE void require_buffer(int size){
    if(size > buffer_size){
	buffer_size = size * 2;
	buffer = (buffer_type *)realloc(buffer,
					buffer_size * sizeof(buffer_type));
	if(buffer == NULL){
	    fprintf(stderr, "Memory exhausted.\n");
	    exit(1);
	}
    }
}

static INLINE void require_part_buffer(int size){
    if(size > part_buffer_size){
	part_buffer_size = size * 2;
	part_buffer = (audio_word *)realloc(part_buffer, part_buffer_size);
	if(part_buffer == NULL){
	    fprintf(stderr, "Memory exhausted.\n");
	    exit(1);
	}
    }
}

void make_play_wave(struct _channel_status *channel_status,
		    int channel, double t, double f, int tai){
    int m, n, r, i;

    n = (int)(t * SAMP_RATE);
    r = 0;
    require_part_buffer(n);

    if(buffer_ptr + n > last_buffer){
	require_buffer(buffer_ptr + n);
	while(buffer_ptr + n > last_buffer){
	    buffer[last_buffer++] = 0;
	}
    }

    if(channel_status[channel].q < 8 && !tai){
	r = (int)((double)(8 - channel_status[channel].q) / 8.0 * n);
    }
    m = n - r;

    set_timbre(channel_status[channel].neiro);

    if(channel_status[channel].neiro != 5){
	if(audio_gen_wave(part_buffer,
			  m, f,
			  channel_status[channel].v,
			  channel_status[channel].m,
			  channel_status[channel].s) == 0){
	    illegal_command();
	    return;
	}

	if(!tai && m > 64){
	    for(i = 0; i < 32; i++){
		double a = (i / 32.0);
		part_buffer[0] *= a;
		part_buffer[m - i - 1] *= a;
	    }
	}
    }else{
	double v = channel_status[channel].v;
	for(i = 0; i < n - r && i < SIZEOF_TAIKO1; i++){
	    part_buffer[i] = (char)(v * taiko1[i]);
	}
	for(; i < n; i++){
	    part_buffer[i] = 0;
	}
    }

    for(i = n - r; i < n; i++){
	part_buffer[i] = 0;
    }

    /* marge `part_buffer' to `buffer' */
    for(i = 0; i < n; i++){
	buffer[buffer_ptr++] += part_buffer[i];
    }
}

void play(void){
    int i, x;
    char ch;

    if(last_buffer == 0){
	return;
    }

    if(play_buffer == NULL){
	play_buffer_size = BUFFER_SIZE;
	if(play_buffer_size < last_buffer)
	    play_buffer_size = last_buffer * 2;

	play_buffer = (char *)xmalloc(play_buffer_size);
    }else if(play_buffer_size < last_buffer){
	free(play_buffer);
	play_buffer_size = last_buffer * 2;

	play_buffer = (char *)xmalloc(play_buffer_size);
    }

    for(i = 0; i < last_buffer; i++){
	x = buffer[i];
	if(x > 127){
	    x = 127;
	}else if(x < -128){
	    x = -128;
	}
	ch = x;
	play_buffer[i] = ch;
    }

    audio_write(play_buffer, last_buffer);
}

void make_rest_wave(double t){
    int n;

    n = (int)(t * SAMP_RATE);
    require_part_buffer(n);

    if(buffer_ptr + n > last_buffer){
	require_buffer(buffer_ptr + n);
	while(buffer_ptr + n > last_buffer){
	    buffer[last_buffer++] = 0;
	}
    }
    buffer_ptr += n;
}
