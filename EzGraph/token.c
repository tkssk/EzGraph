#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "mmlp.h"

int mml_token_type;
int mml_token_val;

static char *mml_text = NULL, *mml_text_base = NULL;
static int lineno = 1;
static char token_buff[MAXLEN];

#define DEFAULT_STACK_SIZE 16

struct textbuffStack {
    int loop_counter;
    int lineno;
    int channel;
    char* text;
    struct textbuff* pos;
};
/* Store the current text status */

static struct textbuff* current_pos;
static struct textbuffStack* textbuffStack;
static int textbuffStack_ptr;
static int textbuffStack_size = DEFAULT_STACK_SIZE;

void setmmltext(char *text){
    int text_len;

    if(mml_text_base != NULL){
	free(mml_text_base);
	mml_text_base = NULL;
    }
    if(text == NULL || (text_len = strlen(text)) == 0){
	return;
    }
    mml_text_base = xmalloc(text_len + 2);

    if(text[text_len - 1] != '\n'){
	mml_text_base = xmalloc(text_len + 2);
	strcpy(mml_text_base, text);
	strcat(mml_text_base, "\n");
    }else{
	mml_text_base = strdup(text);
    }
    mml_text = mml_text_base;
    resetmml();
}

int getlineno(void){
    return lineno;
}

void resetmml(void){
    mml_text = mml_text_base;
    mml_token_type = 0;
    mml_token_val = 0;
    lineno = 1;
}

static void skip_space(void){
    do {
	while(*mml_text == ' ' || *mml_text == '\t'){
	    mml_text++;
	}

	if(*mml_text == '\\' && *(mml_text + 1) == '\n'){
	    mml_text += 2;
	    lineno++;
	    continue;
	}
	if(*mml_text == '%'){
	    while(*mml_text != '\n' && *mml_text != '\0'){
		mml_text++;
	    }

	    if(*mml_text == '\n'){
		mml_text++;
		lineno++;
		continue;
	    }
	}

	if(*mml_text == '\0'){
	    break;
	}
	break;
    } while(1);
}

int gettoken(void){
    register char* tmp;
    static int offset_level[] = {9, 11, 0, 2, 4, 5, 7};
			      /* A  B   C  D  E  F  G */
    static int diff_code_level[12] = {0,0,0,0,0,0,0,0,0,0,0,0};

  next_get_token:
    skip_space();

    if(*mml_text == '\0'){
	mml_token_type = EOF;
	mml_token_val = *mml_text;
	return 1;
    }

    switch(*mml_text){
      case '~':
	mml_text++;
	skip_space();
	fputc(*mml_text, stderr);
	if(*mml_text < 'A' || *mml_text > 'G'){
	    illegal_command();
	    goto next_get_token;
	}
	mml_token_val = offset_level[*mml_text++ - 'A'];
	skip_space();
	fputc(*mml_text, stderr);
	if(*mml_text == '+' || *mml_text == '#'){
	    diff_code_level[mml_token_val] = 1;
	}else if(*mml_text == '-'){
	    diff_code_level[mml_token_val] = -1;
	}else if(*mml_text == '/'){
	    diff_code_level[mml_token_val] = 0;
	}else{
	    illegal_command();
	    goto next_get_token;
	}
	mml_text++;
	goto next_get_token;

      case 'A':
      case 'B':
      case 'C':
      case 'D':
      case 'E':
      case 'F':
      case 'G':
	mml_token_type = PLAY_SCALE;
	mml_token_val = offset_level[*mml_text++ - 'A'];
	skip_space();
	if(*mml_text == '+' || *mml_text == '#'){
	    mml_token_val = (1 + mml_token_val) % 12;
	    mml_text++;
	}else if(*mml_text == '-')
	{
	    mml_token_val = (11 + mml_token_val) % 12;
	    mml_text++;
	}else if(*mml_text == '/')
	{
	    mml_text++;
	}else{
	    mml_token_val = (mml_token_val + 12 + diff_code_level[mml_token_val]) % 12;
	}
	return 1;
      case 'N':
	mml_token_type = PLAY_SCALEN;
	mml_text++;
	if(!isdigit(*mml_text)){
	    illegal_command();
	    goto next_get_token;
	}
	tmp = token_buff;
	while(isdigit(*mml_text)){
	  *tmp++ = *mml_text++;
      }
	*tmp++ = '\0';
	mml_token_val = atoi(token_buff);
	return 1;
      case '+':
      case '#':
      case '-':
      case '.':
      case '&':
	mml_token_type = OP;
	mml_token_val = *mml_text++;
	return 1;
      case 'L':
      case 'Q':
      case 'O':
      case 'T':
      case 'R':
      case 'M':
      case 'S':
      case '<':
      case '>':
      case '{':
      case '}':
	mml_token_type = COMMAND;
	mml_token_val = *mml_text++;
	return 1;
      case 'V':
	mml_token_type = COMMAND;
	mml_token_val = 'V';
	mml_text++;
	return 1;
      case '@':
	mml_token_type = COMMAND;
	if(*(mml_text + 1) == 'V'){
	    mml_token_val = 'v';
	    mml_text += 2;
	    return 1;
	}
	mml_token_val = *mml_text++;
	return 1;
      case 'Y':
	mml_token_type = COMMAND;
	mml_token_val = *mml_text++;
	return 1;
      case ',':
	mml_token_type = EOPart;
	mml_token_val = *mml_text++;
	return 1;
      case '\n':
      case '\r':
	mml_token_type = EOPhase;
	mml_token_val = *mml_text++;
	lineno++;
	return 1;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
	tmp = token_buff;
	while(isdigit(*mml_text)){
	    *tmp++ = *mml_text++;
	}
	*tmp++ = '\0';
	mml_token_type = NUMBER;
	mml_token_val = atoi(token_buff);
	return 1;
      case '|':
	mml_token_type = START_LOOP;
	mml_text++;
	skip_space();
	if(*mml_text != ':'){
	    illegal_command();
	    goto next_get_token;
	}
	mml_text++;
	skip_space();

	if(isdigit(*mml_text)){
	    tmp = token_buff;
	    while(isdigit(*mml_text)){
		*tmp++ = *mml_text++;
	    }
	    *tmp++ = '\0';
	    mml_token_val = atoi(token_buff) - 1;
	}else{
	    mml_token_val = 1;
	}
	return 1;
      case ':':
	mml_token_type = END_LOOP;
	mml_text++;
	skip_space();
	if(*mml_text != '|'){
	    illegal_command();
	    goto next_get_token;
	}
	mml_text++;
	return 1;
    }

    mml_text++;
    goto next_get_token;
}

void begin_loop(int n, int current_channel){
    if(textbuffStack == NULL){
	textbuffStack = (struct textbuffStack *)
	    xmalloc(textbuffStack_size * sizeof(struct textbuffStack));
    }else if(textbuffStack_size == textbuffStack_ptr){
	textbuffStack_size *= 2;
	textbuffStack = (struct textbuffStack *)
	    realloc(textbuffStack,
		    textbuffStack_size * sizeof(struct textbuffStack));
	if(textbuffStack == NULL){
	    fprintf(stderr, "Memory exhausted.\n");
	    quit(1);
	}
    }

    textbuffStack[textbuffStack_ptr].loop_counter = n;
    textbuffStack[textbuffStack_ptr].lineno = lineno;
    textbuffStack[textbuffStack_ptr].channel = current_channel;
    textbuffStack[textbuffStack_ptr].text = mml_text;
    textbuffStack[textbuffStack_ptr].pos = current_pos;
    textbuffStack_ptr++;
}

void end_loop(int *current_channel){
    int x = textbuffStack_ptr - 1;

    if(x < 0){
	fprintf(stderr, "illegal loop command, in %d\n", lineno);
	quit(1);
    }

    if(textbuffStack[x].loop_counter <= 0){
	textbuffStack_ptr = x;
	return;
    }

    textbuffStack[x].loop_counter--;
    lineno           = textbuffStack[x].lineno;
    *current_channel = textbuffStack[x].channel;
    mml_text         = textbuffStack[x].text;
    current_pos      = textbuffStack[x].pos;
}
