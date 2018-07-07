/*
 * EzGraph: Easy Graphic Library
 * Copyright(C) 2016 Y. Doi
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>

#include <assert.h>

#include "EzGraph.h"

#define PENDINGS 32             /* Max pending queue size */

struct fp_list_t {
  struct fp_list_t *next;
  FILE* fp;
};

static struct fp_list_t dummy = {};
static struct fp_list_t* head = &dummy;

/* -1 : listen error or unconnected */
static int listen_socket = -1;
static unsigned int current_port;

static FILE* searchfp(int fd) {
    struct fp_list_t* current;

    for (current = head->next; current != NULL; current = current->next) {
	FILE* fp;

	if (fileno(fp = current->fp) == fd) {
	    return fp;
	}
    }
    return NULL;
}

static FILE* removefp(int fd) {
    struct fp_list_t* prev;

    for (prev = head; prev && prev->next != NULL; prev = prev->next) {
	struct fp_list_t* current = prev->next;
	FILE* fp;

	if (fileno(fp = current->fp) == fd) {
	    prev->next = current->next;
	    free(current);
	    return fp;
	}
    }
    return NULL;
}

static FILE* addfp(FILE* fp) {
    struct fp_list_t* tail = head;
    removefp(fileno(fp)); /* to prevent multiple registration */

    while (tail->next != NULL) tail = tail->next;
    tail->next = malloc(sizeof(struct fp_list_t));
    tail->next->next = NULL;
    tail->next->fp = fp;
    return fp;
}

static int sock_check(int socket) {
    if (socket == -1) {
	perror("[EzConnect]");
	return 0;
    }
    return 1;
}
static int file_check(FILE *f) {
    if (f == NULL) {
	perror("[EzConnect]");
	return 0;
    }
    return 1;
}

void EzMonitor(FILE* fp) {
    addfp(fp);
}

FILE *EzConnect(const char *node, unsigned int port) {
    struct addrinfo hints = {};
    struct addrinfo *result, *rp;
    int sfd, ret;
    char service[16];

    sprintf(service, "%d", port);
    hints.ai_family = AF_UNSPEC; /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;
    hints.ai_protocol = 0; /* Any protocol */
    ret = getaddrinfo(node, service, &hints, &result);
    if (ret != 0) {
	fprintf(stderr, "[EzConnect]: getaddrinfo: %s\n", gai_strerror(ret));
	return NULL;
    }
    for (rp = result; rp != NULL; rp = rp->ai_next) {
	sfd = socket(rp->ai_family, rp->ai_socktype,
		     rp->ai_protocol);
	if (sfd == -1) {
	    continue;
	}
	if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
	    FILE *fp = fdopen(sfd, "r+");
	    
	    file_check(fp);
	    setbuf(fp, NULL); /* Disable buffering */
	    freeaddrinfo(result);
	    return addfp(fp);
	}
	switch (errno) {
	  case ECONNREFUSED:
	    /* No-one listening on the remote address. */
	    /* perror("[EzConnect]"); */
	    break;
	  default:
	    perror("[EzConnect]");
	}
	close(sfd);
    }
    freeaddrinfo(result); /* No longer needed */
    return NULL;
}

int EzListen(unsigned int port) { /* bind & listen */
    int option = 1;
    if (listen_socket != -1) {
	return current_port == port;
    }
    listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    /* For case of address already in use */
    setsockopt(listen_socket, SOL_SOCKET,
	       SO_REUSEADDR, &option, sizeof(option));
    if (!sock_check(listen_socket)) {
	return 0;
    }
    struct sockaddr_in addr = {
	.sin_family = AF_INET,
	.sin_port = htons(port),
	.sin_addr.s_addr = INADDR_ANY,
    };
    if (sock_check(bind(listen_socket,
			(struct sockaddr *)&addr,
			sizeof(addr)))) {
	listen(listen_socket, PENDINGS);
	current_port = port;
	return 1;
    }
    return 0;
}

void EzListenClose(void) {
    if (listen_socket == -1) {
	return;
    }
    if (close(listen_socket) == -1) {
	perror("[EzConnect]");
    }
    listen_socket = -1;
}

FILE *EzAccept(void) {
    struct sockaddr_in client;

    socklen_t len = sizeof(struct sockaddr_in);;
    int nfds = listen_socket+1, ret;
    struct timeval utimeout = {};
    fd_set readfds, writefds, exceptfds;
    if (listen_socket == -1) {
	return NULL;
    }
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);
    FD_SET(listen_socket, &readfds);
    ret = select(nfds, &readfds, &writefds, &exceptfds,
		 &utimeout);
    assert(ret != -1);
    if (!FD_ISSET(listen_socket, &readfds)) {
	return NULL;
    }
    
    /* Accept connection request from a client */
    int sock = accept(listen_socket, (struct sockaddr *)&client, &len);
    if (sock_check(sock)) {
	FILE *fp = fdopen(sock, "w+");

	file_check(fp);
	setbuf(fp, NULL);
	return addfp(fp);
    }
    return NULL;
}

static int check(EzConnectHandler closed,
		 EzConnectHandler receive) {
    int nfds = -1, ret, fired = 0;
    struct fp_list_t *current;
    struct timeval utimeout = {};
    fd_set readfds, writefds, exceptfds;

    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);

    for (current = head->next; current != NULL; current = current->next) {
	int fd = fileno(current->fp);

	nfds = (fd > nfds)? fd: nfds;
	FD_SET(fd, &readfds);
	FD_SET(fd, &writefds);
    }
    
    nfds += 1;
    ret = select(nfds, &readfds, &writefds, &exceptfds, &utimeout);
    assert(ret != -1);

    if (ret > 0) {
	for (current = head->next; current != NULL; current = current->next) {
	    int fd = fileno(current->fp);

	    if (FD_ISSET(fd, &readfds)) {
		char buf[16];
		FILE* fp;
		
		FD_CLR(fd, &readfds);
		ret = recv(fd, buf, 16, MSG_PEEK);
		if (ret == 0) { /* < 0; error: man recv */
		    recv(fd, buf, 16, MSG_DONTWAIT);
		    fp = removefp(fd);
		    if (closed != NULL) {
			closed(fp);
		    }
		    current = head;
		    continue;
		} else { /* readable */
		    fp = searchfp(fd);
		    if (receive != NULL) {
			receive(fp);
			fired++;
		    }
		}
	    }
	}
    }
    return fired;
}

void EzCheckConnection(EzConnectHandler closed, EzConnectHandler receive) {
    while (check(closed, receive));
}
