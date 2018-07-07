#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include "mmlp.h"
#include "audio.h"

static int current_channel;
static struct _channel_status *channel_status = NULL;
static int nchannel = 8;
static int parse_mml2(void);
static int parse_mml_command(void);
static snd_pcm_t *pcm_handle;
static char *snd_err_msg;
#define PCM_DEVICE "default"

void quit(int status){
    audio_close();
    exit(status);
}

void illegal_command(void){
    fprintf(stderr, "Illegal command in %d\n", getlineno());
}

void* xmalloc(int nbyte){
    void* p;

    if((p = (void *)malloc(nbyte)) == NULL && nbyte > 0){
	fprintf(stderr, "Memory exhausted.\n");
	quit(1);
    }

    return p;
}

static void init_channel1(int i){
    channel_status[i].t = 120;
    channel_status[i].l = 4.0;
    channel_status[i].q = 8;
    channel_status[i].o = 4;
    channel_status[i].v = INIT_VOL;
    channel_status[i].neiro = 0;
    channel_status[i].m = 256;
    channel_status[i].s = 13;
}

static void init_channel(void){
    int i;

    if(channel_status == NULL){
	channel_status = (struct _channel_status *)
	  xmalloc(nchannel * sizeof(struct _channel_status));
    }
    for(i = 0; i < nchannel; i++)
	init_channel1(i);
    current_channel = 0;
}

/* calclate play time [sec.] */
static INLINE double calc_playtime(double len){
    return (4.0 * 60.0) / channel_status[current_channel].t / len;
}

static void parse_play_scale(void){
    double play_time;
    int scale;
    double len;
    double f;
    int first_scale;
    int q8_mode;
    int tai_mode;
    double more;

    if(mml_token_type == PLAY_SCALE){
	first_scale = (channel_status[current_channel].o - 1) * 12 + mml_token_val;
    }else{
	first_scale = mml_token_val;
    }

    scale = first_scale;
    play_time = 0;
    q8_mode = 0;
    
  scale_concat:
    len = channel_status[current_channel].l;

    gettoken();
    if(mml_token_type == NUMBER){
	len = (double)mml_token_val;
	gettoken();
    }

    len = more = calc_playtime(len);

    if(mml_token_type == OP){
	while(mml_token_val == '.'){
	    gettoken();
	    more /= 2;
	    len += more;
	}
    }

    if(mml_token_type == OP && mml_token_val == '&'){
	gettoken();
	tai_mode = 1;
    }else{
	tai_mode = 0;
    }

    f = NtoHz(scale);
    play_time += len;

    if(tai_mode){
	if(mml_token_type == PLAY_SCALE || mml_token_type == PLAY_SCALEN){
	    if(mml_token_type == PLAY_SCALE){
		scale
		    = (channel_status[current_channel].o - 1) * 12 + mml_token_val;
	    }else{
		scale = mml_token_val;
	    }
	    if(scale == first_scale){
		goto scale_concat;
	    }
	}
	q8_mode = 1;
    }

    make_play_wave(channel_status, current_channel, play_time, f, q8_mode);
}

static void parse_mml_R(void){
    /* R[0-9]*\.* */
    double len, more;

    len = channel_status[current_channel].l;
    gettoken();
    if(mml_token_type == NUMBER){
	len = (double)mml_token_val;
	gettoken();
    }
    len = more = calc_playtime(len);

    if(mml_token_type == OP){
	while(mml_token_val == '.')	{
	    gettoken();
	    more /= 2;
	    len += more;
	}
    }

    make_rest_wave(len);
}

static void parse_mml_L(void){
    double len;
    int channel = current_channel;

    gettoken();

    if(mml_token_type != NUMBER){
	channel_status[channel].l = 4.0;
	return;
    }

    len = (double)mml_token_val;
    gettoken();

    if(mml_token_type == OP){
	while(mml_token_val == '.')	{
	    gettoken();
	    len /= 1.5;
	}
    }

    channel_status[channel].l = len;
}

static void parse_mml_Q(void){
    int channel = current_channel;
    gettoken();

    if(mml_token_type != NUMBER){
	channel_status[channel].q = 8;
	return;
    }
    channel_status[channel].q = mml_token_val;
    gettoken();
}

static void parse_mml_O(void){
    int channel = current_channel;
    gettoken();
    if(mml_token_type != NUMBER){
	channel_status[channel].o = 4;
	return;
    }
    channel_status[channel].o = mml_token_val;
    gettoken();
}

static void parse_mml_T(void)
{
    int channel = current_channel;
    gettoken();
    if(mml_token_type != NUMBER){
	channel_status[channel].t = 120;
	return;
    }
    channel_status[channel].t = mml_token_val;
    gettoken();
}

static void parse_mml_M(void){
    int channel = current_channel;
    gettoken();
    if(mml_token_type != NUMBER){
	channel_status[channel].m = 256;
	return;
    }
    channel_status[channel].m = mml_token_val;
    gettoken();
}

static void parse_mml_S(void){
    int channel = current_channel;
    gettoken();
    if(mml_token_type != NUMBER){
	channel_status[channel].s = 13;
	return;
    }
    channel_status[channel].s = mml_token_val;
    gettoken();
}

static void rset_volume(int n){
    channel_status[current_channel].v += n / 15.0 * CHANNEL_VMAX;
}

static void parse_mml_atto(void){
    int channel = current_channel;
    gettoken();

    switch(mml_token_type){
      case NUMBER:
	channel_status[channel].neiro = mml_token_val;
	gettoken();
	break;
      case COMMAND:
	if(mml_token_val == '>'){
	    gettoken();
	    if(mml_token_type == NUMBER){
		rset_volume(mml_token_val);
		gettoken();
	    }else{
		rset_volume(1);
	    }
	    return;
	} else if(mml_token_val == '<'){
	    gettoken();
	    if(mml_token_type == NUMBER){
		rset_volume(-mml_token_val);
		gettoken();
	    }else{
		rset_volume(-1);
	    }
	    return;
	}

	/* through */
      default:
	channel_status[channel].neiro = 0;
    }
}

static void parse_mml_neiro(void){
    int channel = current_channel;
    gettoken();

    switch(mml_token_type){
      case NUMBER:
	channel_status[channel].neiro = mml_token_val;
	gettoken();
	break;
      default:
	channel_status[channel].neiro = 0;
    }
}

static void parse_mml_down_octave(void){
    int channel = current_channel;
    gettoken();

    if(channel_status[channel].o == 1){
	illegal_command();
	return;
    }
    channel_status[channel].o--;
}

static void parse_mml_up_octave(void){
    int channel = current_channel;
    gettoken();

    if(channel_status[channel].o == 9){
	illegal_command();
	return;
    }
    channel_status[channel].o++;
}

static void parse_mml_V(void){
    int channel = current_channel;
    gettoken();
    if(mml_token_type != NUMBER){
	channel_status[channel].v = INIT_VOL;
    }else{
	channel_status[channel].v
	    = mml_token_val / 15.0 * CHANNEL_VMAX;
	gettoken();
    }
}

static void parse_mml_v(void){
    int channel = current_channel;
    gettoken();
    if(mml_token_type != NUMBER){
	channel_status[channel].v = INIT_VOL;
    }else{
	channel_status[channel].v
	    = mml_token_val / 127.0 * CHANNEL_VMAX;
	gettoken();
    }
}

static void parse_mml_Nthuplet(void){
    static int* scale_list = NULL;
    static int list_size = 0;
    static int list_max_size = 0;
    static const int default_list_size = 64;
    double len, play_time;
    int i;
    int nscale;

    nscale = 0;
    gettoken();
    while(1){
	if(mml_token_type == EOPhase ||
	   mml_token_type == EOPart ||
	   mml_token_type == EOF){
	    fprintf(stderr, "Parenthesis is mismatched in %d\n", getlineno());
	    break;
	}

	if(mml_token_type == COMMAND){
	    if(mml_token_val == '}'){
		gettoken();
		break;
	    }
	    parse_mml_command();
	    continue;
	}

	if(mml_token_type == START_LOOP){
	    begin_loop(mml_token_val, current_channel);
	    gettoken();
	    continue;
	}

	if(mml_token_type == END_LOOP){
	    end_loop(&current_channel);
	    gettoken();
	    continue;
	}

	if(mml_token_type != PLAY_SCALE && mml_token_type != PLAY_SCALEN){
	    gettoken();
	    continue;
	}

	if(list_size == list_max_size){
	    if(list_max_size == 0){
		list_max_size = default_list_size;
		scale_list = (int *)xmalloc(list_max_size * sizeof(int));
	    }else{
		list_max_size *= 2;
		scale_list = (int *)realloc(scale_list,
					    list_max_size * sizeof(int));

		if(scale_list == NULL){
		    fprintf(stderr, "Memory is exhausted.\n");
		    quit(1);
		}
	    }
	}
	if(mml_token_type == PLAY_SCALEN){
	    scale_list[list_size] = mml_token_val;
	}else{
	    scale_list[list_size]
		= (channel_status[current_channel].o - 1) * 12 + mml_token_val;
	}
	list_size++;
	nscale++;
	gettoken();
    }

    list_size -= nscale;

    if(nscale == 0){
	return;
    }
    if(mml_token_type == NUMBER){
	len = (double)mml_token_val;
	gettoken();
    }else{
	len = channel_status[current_channel].l;
    }

    if(mml_token_type == OP){
	while(mml_token_val == '.'){
	    gettoken();
	    len /= 1.5;
	}
    }
    play_time = calc_playtime(len);

    for(i = 0; i < nscale; i++){
	make_play_wave(channel_status, current_channel, play_time / nscale,
		       NtoHz(scale_list[list_size + i]), 0);
    }
}


static int parse_mml_command(void){
    switch(mml_token_val){
      case 'V':
	parse_mml_V();
	break;
      case 'v':		/* @V */
	parse_mml_v();
	break;
      case 'L':
	parse_mml_L();
	break;
      case 'Q':
	parse_mml_Q();
	break;
      case 'O':
	parse_mml_O();
	break;
      case 'T':
	parse_mml_T();
	break;
      case 'R':
	parse_mml_R();
	break;
      case '@':
	parse_mml_atto();
	break;
      case 'Y':
	parse_mml_neiro();
	break;
      case 'M':
	parse_mml_M();
	break;
      case 'S':
	parse_mml_S();
	break;
      case '<':
	parse_mml_down_octave();
	break;
      case '>':
	parse_mml_up_octave();
	break;
      case '{':
	parse_mml_Nthuplet();
	break;
      default:
	illegal_command();
	gettoken();
	return 0;
    }
    return 1;
}

static int parse_mml2(void){
    switch(mml_token_type){
      case EOPhase:
      case EOPart:
      case EOF:
	return 2;
      case PLAY_SCALE:
      case PLAY_SCALEN:
	parse_play_scale();
	break;
      case COMMAND:
	return parse_mml_command();
      case START_LOOP:
	begin_loop(mml_token_val, current_channel);
	gettoken();
	break;
      case END_LOOP:
	end_loop(&current_channel);
	gettoken();
	break;
      default:
	illegal_command();
	gettoken();
	return 0;
    }
    return 1;
}

static void parse_mml1(void){
    int status;

    while(1){
	status = parse_mml2();
	if(status == 2)
	    return;
    }
}

static void expand_channel(void){
    int i;

    channel_status = (struct _channel_status *)
	realloc(channel_status, 2 * nchannel * sizeof(struct _channel_status));
    if(channel_status == NULL){
	fprintf(stderr, "Memory exhausted.\n");
	quit(1);
    }

    for(i = nchannel; i < nchannel * 2; i++){
	init_channel1(i);
    }

    nchannel *= 2;
}	

static void parse_mml0(void){
    if(mml_token_type == EOPhase){
	current_channel = 0;
	gettoken();
	return;
    }

    while(1){
	if(mml_token_type == EOPart){
	    current_channel++;
	    if(current_channel == nchannel){
		expand_channel();
	    }
	    gettoken();
	    start_position_play();
	    continue;
	}

	if(mml_token_type == EOPhase || mml_token_type == EOF){
	    current_channel = 0;
	    gettoken();
	    break;
	}
	parse_mml1();
    }
}

int audio_open(void){
    unsigned int pcm;
    snd_pcm_hw_params_t *params;
    unsigned int rate = SAMP_RATE;
    int channels = 1;

    /* Open the PCM device in playback mode */
    if((pcm = snd_pcm_open(&pcm_handle, PCM_DEVICE,
			   SND_PCM_STREAM_PLAYBACK, 0)) < 0){
	snd_err_msg = "Can't open PCM device.";
	return pcm;
    }
    
    /* Allocate parameters object and fill it with default values*/
    snd_pcm_hw_params_alloca(&params);
    
    snd_pcm_hw_params_any(pcm_handle, params);
    
    /* Set parameters */
    if((pcm = snd_pcm_hw_params_set_access(pcm_handle, params,
				   SND_PCM_ACCESS_RW_INTERLEAVED)) < 0){
	snd_err_msg = "Can't set interleaved mode.";
	return pcm;
    }
    
    if((pcm = snd_pcm_hw_params_set_format(pcm_handle, params,
					   SND_PCM_FORMAT_S8)) < 0){
	snd_err_msg = "Can't set format.";
	return pcm;
    }
    
    if((pcm = snd_pcm_hw_params_set_channels(pcm_handle,
					     params, channels)) < 0){
	snd_err_msg = "Can't set channels number.";
	return pcm;
    }
    
    if((pcm = snd_pcm_hw_params_set_rate_near(pcm_handle,
					      params, &rate, 0)) < 0){
	snd_err_msg = "Can't set rate.";
	return pcm;
    }
    
    /* Write parameters */
    if((pcm = snd_pcm_hw_params(pcm_handle, params)) < 0){
	snd_err_msg = "Can't set harware parameters.";
	return pcm;
    }

    return 0;
}

void audio_close(void){
    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);
}

int audio_write(char* buff, int n){
    unsigned int pcm;

    if((pcm = snd_pcm_writei(pcm_handle, buff, n)) == -EPIPE){
	snd_pcm_prepare(pcm_handle);
    }else if (pcm < 0){
	snd_err_msg = "Can't write to audio device.";
	return pcm;
    }
    return 0;
}

static pthread_t tid;
static pthread_mutex_t tlock;
static int mml_init = 0;
typedef enum {mml_st_stop, mml_st_play} mml_state_t;
static mml_state_t mml_state = mml_st_stop;

static void *mml_thr_player(void *arg){
    pthread_mutex_lock(&tlock);
    init_channel();

    for(;;){
	resetmml();
	gettoken();
	while(mml_token_type != EOF){
	    clear_play_buffer();
	    parse_mml0();
	    play();
	}
    }
    // Never reach here
    pthread_mutex_unlock(&tlock);
    return NULL;
}

void mml_play(void){
    if(mml_state == mml_st_play){
	return;
    }
    if(!mml_init){
	pthread_mutex_init(&tlock, NULL);
	mml_init = 1;
    }
    pthread_create(&tid, NULL, mml_thr_player, NULL);
    mml_state = mml_st_play;
}

void mml_stop(void){
    if(mml_state == mml_st_stop){
	return;
    }
    pthread_cancel(tid);
    pthread_join(tid, NULL);
    pthread_mutex_unlock(&tlock);
    mml_state = mml_st_stop;
}

#if 0
int main(int argc, char** argv){
    int pcm;

    char *sng1 =
      "CDE2CDE2"
      "GEDCDED2"
      "CDE2CDE2"
      "GEDCDEC2"
      "GGEGAAG2"
      "EEDDC1";
    char *sng2 =
      "CCGGAAG2"

      "GGFFEED2"
      "GGFFEED2"
      "CCGGAAG2"
      "FFEEDDC2";

    if((pcm = audio_open()) < 0){
	fprintf(stderr, "%s\n\t%s\n", snd_err_msg, snd_strerror(pcm));
	exit(1);
    }

    setmmltext(sng1);
    mml_play();
    printf("Sleep...\n");
    sleep(60);
    mml_stop();

    setmmltext(sng2);
    mml_play();
    printf("Sleep...\n");
    sleep(10);
    mml_stop();

    mml_play();
    sleep(10);
    mml_stop();

    setmmltext(sng1);
    mml_play();
    printf("Sleep...\n");
    sleep(30);

    quit(0);
    return 0;
}
#endif
