#include <EzGraph.h>
#include <stdio.h>

#define PORT 6002
#define BUF_MAX 8192
#define CLIENT_MAX 32

FILE* clients[CLIENT_MAX];

void add_new_client(FILE* client) {
  int i;
  for (i = 0; i < CLIENT_MAX; i++) {
	if (clients[i] == NULL) {
	  clients[i] = client;
	  return;
	}
  }
}

void remove_client(FILE* client) {
  int i;
  for (i = 0; i < CLIENT_MAX; i++) {
	if (clients[i] == client) {
	  clients[i] = NULL;
	  break;
	}
  }
}

void receive_handler(FILE* client) { /* Message receive handler */
  int i;
  char buf[BUF_MAX];
  fgets(buf, BUF_MAX, client);  /* Read one line */
  for (i = 0; i < CLIENT_MAX; i++) { /* Send message to all clients */
	if (clients[i] == NULL) continue;
	fprintf(clients[i], "%s", buf); /* Send message */
  }
}

void close_handler(FILE* client) {
  /* Remove client from client list */
  remove_client(client);
}

void accept_handler(FILE* client) {
  /* Add to client list */
  add_new_client(client);
}

int main(void) {
  EzSetAcceptHandler(accept_handler); /* New connection event handler */
  EzSetReceiveHandler(receive_handler); /* New message receive event handler */
  EzSetConnectionCloseHandler(close_handler); /* Disconnect event handler */
  EzListen(PORT);               /* Listen at port PORT */
  EzEventLoop();
  return 0;
}
