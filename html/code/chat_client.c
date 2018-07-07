#include <EzGraph.h>
#include <stdio.h>
#include <ctype.h>

#define PORT 6002
#define BUF_MAX 8192
#define LINES 64
#define CHARS 80

FILE *server;
int ring_first = 0, str_index;
char *name, texts[LINES][CHARS], unsend_string[BUF_MAX];

void draw(void) {
  int i;
  for (i = 0; i < LINES; i++) {
	EzDrawStringB(0, (i + 2.5) * 16, texts[(i + ring_first + 1) % LINES]);
  }
  EzDrawStringB(0, 16, unsend_string);
  EzShowBuffer();
}

void receive_handler(FILE* server) { /* Message receive handler */
  int i;
  char buf[BUF_MAX];
  fgets(buf, BUF_MAX, server);  /* read one line */
  for (i = 0; (buf[i] != '\n') && (i < CHARS-1); i++)
	texts[ring_first][i] = buf[i]; /* copy CHARS charactors */
  texts[ring_first][i] = '\0';
  ring_first = (LINES + ring_first - 1) % LINES;
  draw();
}

void key_handler(int key) {
  switch (key) {
  case 0x0d: /* Return */
	/* send unsend_string to server */
	fprintf(server, "%s: %s\n", name, unsend_string);
  case 0x1b: /* Escape */
	str_index = 0;
	break;
  case 0x08: /* BackSpace */
	if (str_index > 0) str_index--;
	break;
  case 0xffe1: /* Shift */
	break;
  default:
	if (isascii(key))
	  unsend_string[str_index++] = key;
  }
  unsend_string[str_index] = '\0';
  draw();
}

/* Called when the connection is closed */
void close_handler(FILE* server) {
  puts("server closed");
  EzExitEventLoop();            /* Exit program */
}

int main(int argc, char *argv[]) {
  char *server_address;
  if (argc < 3) return 1;
  name = argv[1];
  server_address = argv[2];
  EzSetReceiveHandler(receive_handler);
  EzSetConnectionCloseHandler(close_handler);
  EzSetKeyHandler(key_handler);
  /* Connect to the server server_address:PORT */
  server = EzConnect(server_address, PORT);
  /* Exit if connection is failed. */
  if (server == NULL) return 1;
  EzEventLoop();
  return 0;
}
