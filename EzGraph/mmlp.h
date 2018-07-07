#ifndef ___MMLP_H_
#define ___MMLP_H_

#define MAXLEN 256
#define CHANNEL_VMAX 0.6
#define INIT_VOL (CHANNEL_VMAX * 8.0 / 15.0)

#if !defined(INLINE)
#if defined(__GNUC__)
#define INLINE inline
#elif defined(sgi) || defined(__osf__)
#define INLINE __inline
#else
#define INLINE
#endif
#endif

enum mml_token_types{
    PLAY_SCALE,			/* [A-G][+#-]? */
    PLAY_SCALEN,		/* N[0-9]+ */
    OP,				/* +,#,-,.,& */
    NUMBER,			/* [0-9]+ */
    COMMAND,			/* V,L,Q,O,T,R,@,@V,M,S,<,>,{,} */
    START_LOOP,			/* |:[0-9]* */
    END_LOOP,			/* :| */
    EOPart,			/* End Of Part (,) */
    EOPhase			/* End Of Phase */
};

struct _channel_status{
    int t;		/* T */
    double l;		/* L [1, 64] */
    unsigned char q;	/* Q [1, 8] */
    unsigned char o;	/* O [1, 9] */
    double v;		/* V[0, 15] , @V[0, 127] => [0, CHANNEL_VMAX] */
    int neiro;		/* y */
    unsigned short m;	/* M */
    unsigned char s;	/* S */
};

extern int mml_token_type;
extern int mml_token_val;

double NtoHz(unsigned int n);

void setmmltext(char *);
int getlineno(void);
void resetmml(void);
int gettoken(void);

void illegal_command(void);
void* xmalloc(int nbyte);

void clear_play_buffer(void);
void start_position_play(void);
void make_play_wave(struct _channel_status *,
		    int channel, double t, double f, int tai);
void make_rest_wave(double t);
void play(void);
void begin_loop(int n, int);
void end_loop(int *);
void add_mmltext(char* text);
void ext_command(const char* cmd);
void quit(int status);

/* surround */
void exec_remote_clild(char** argv, int argc, int start_file_list);
void sync_play_start(char* parent_host);
void sync_play(void);
void close_parent_fd(void);

/* network.c */
int init_socket(int offset, char* myname);
void wait_client(int sfd, int* fd, int n);
int connect_server(char *hostname, int offset);

/* mmlp.c */
int write_open(void);
void audio_close(void);
int audio_write(char* buff, int n);
void mml_play(void);
void mml_stop(void);

#endif /* ___MMLP_H_ */
