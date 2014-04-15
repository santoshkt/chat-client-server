///////////////////////////////////////////////////////////////////////////////
//// 
////  Filename: chatclient.c
////  Author : Santosh K Tadikonda (stadikon@gmu.edu)
////  Date: Oct 25, 2013
////  Version: 1.0
////
///////////////////////////////////////////////////////////////////////////////

/*--------------------------------------------------------------------*/
/* chat client */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>

#include "common.h"

#define QUIT_STRING "/end"
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/* display group list */
void showgroups(long lent, char *text) {
	char *tptr;

	tptr = text;
	printf("%15s %15s %15s\n", "group", "capacity", "occupancy");
	while (tptr < text + lent) {
		char *name, *capa, *occu;

		name = tptr;
		tptr = name + strlen(name) + 1;
		capa = tptr;
		tptr = capa + strlen(capa) + 1;
		occu = tptr;
		tptr = occu + strlen(occu) + 1;

		printf("%15s %15s %15s\n", name, capa, occu);
	}
}

/* try to join a group */
int joinagroup(int sock) {
	//printf("SANT: Join a Group ?\n");
	Packet * pkt;
	char bufr[MAXPKTLEN];
	char * bufrptr;
	int bufrlen;
	char * gname;
	char * mname;

	/* send a list group request */
	sendpkt(sock, LIST_GROUPS, 0, NULL);

	/* recv a list group reply */
	pkt = recvpkt(sock);
	if (!pkt) {
		fprintf(stderr, "error: server died\n");
		exit(1);
	}

	if (pkt->type != LIST_GROUPS) {
		fprintf(stderr, "error: unexpected reply from server\n");
		exit(1);
	}

	/* display groups */
	showgroups(pkt->lent, pkt->text);

	/* read in group name */
	printf("which group? ");
	fgets(bufr, MAXPKTLEN, stdin);
	bufr[strlen(bufr) - 1] = '\0';

	/* may want to quit */
	if (strcmp(bufr, "") == 0
			|| strncmp(bufr, QUIT_STRING, strlen(QUIT_STRING)) == 0) {
		//printf("SANT: Quitting..\n");
		close(sock);
		exit(0);
	}
	gname = strdup(bufr);

	/* read in member name */
	printf("what nickname? ");
	fgets(bufr, MAXPKTLEN, stdin);
	bufr[strlen(bufr) - 1] = '\0';

	/* may want to quit */
	if (strcmp(bufr, "") == 0
			|| strncmp(bufr, QUIT_STRING, strlen(QUIT_STRING)) == 0) {
		close(sock);
		exit(0);
	}
	mname = strdup(bufr);

	/* send a join group message */
	bufrptr = bufr;
	strcpy(bufrptr, gname);
	bufrptr += strlen(bufrptr) + 1;
	strcpy(bufrptr, mname);
	bufrptr += strlen(bufrptr) + 1;
	bufrlen = bufrptr - bufr;
	sendpkt(sock, JOIN_GROUP, bufrlen, bufr);

	/* read the reply */
	pkt = recvpkt(sock);
	if (!pkt) {
		fprintf(stderr, "error: server died\n");
		exit(1);
	}
	if (pkt->type != JOIN_ACCEPTED && pkt->type != JOIN_REJECTED) {
		fprintf(stderr, "error: unexpected reply from server\n");
		exit(1);
	}

	/* if rejected display the reason */
	if (pkt->type == JOIN_REJECTED) {
		printf("admin: %s\n", pkt->text);
		free(gname);
		free(mname);
		return (0);
	} else {
		printf("admin: joined '%s' as '%s'\n", gname, mname);
		free(gname);
		free(mname);
		return (1);
	}
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
main(int argc, char *argv[]) {
	int sock;
	//printf("SANT: Program started.. \n");

	/* check usage */
	if (argc != 1) {
		fprintf(stderr, "usage : %s\n", argv[0]);
		exit(1);
	}

	//printf("SANT: Trying to hook to server\n");
	/* get hooked on to the server */
	sock = hooktoserver();
	if (sock == -1)
		exit(1);
	//printf("SANT: YAY! Hooked with socket: %d.", sock);

	fflush(stdout);

	// Initialize FDs to zero.
	// Assign Input FD to client Fds.
	// Assign socked FD to client Fds.

	fd_set clientfds, tempfds;
	FD_ZERO(&clientfds);
	FD_ZERO(&tempfds);
	FD_SET(sock, &clientfds);
	FD_SET(0, &clientfds);

	/* keep mingling */
	while (1) {
		/* try to join a group */
		if (!joinagroup(sock))
			continue;

		/* joined some group. keep chatting */
		while (1) {
			/*
			 FILL HERE
			 use select() to watch simulataneously for
			 input from keyboard and messages from server
			 */

			// Use tempfds as it will be overwritten on select call
			// We want clientfds to keep track of all connected fds.
			tempfds = clientfds;

			if (select(FD_SETSIZE, &tempfds, NULL, NULL, NULL) == -1) {
				perror("select");
				exit(4);
			}

			// For every fd in the list, if the fd is set, check if
			// that is socket fd. If so, then it means we
			// received some message from server. It can be a message
			// from different client or server death. Also if the fd
			// is 0, it means there is some input from the user.
			// Read that input and send it to the server.

			/* FILL HERE: message from server? */
			if (FD_ISSET(sock,&tempfds)) {
				Packet *pkt;
				pkt = recvpkt(sock);
				if (!pkt) {
					/* server killed, exit */
					fprintf(stderr, "error: server died\n");
					exit(1);
				}

				/* display the text */
				if (pkt->type != USER_TEXT) {
					fprintf(stderr, "error: unexpected reply from server\n");
					exit(1);
				}

				printf("%s: %s", pkt->text, pkt->text + strlen(pkt->text) + 1);
				freepkt(pkt);
			}

			/* FILL HERE: input from keyboard? */
			if (FD_ISSET(0,&tempfds)) {
				char bufr[MAXPKTLEN];

				fgets(bufr, MAXPKTLEN, stdin);
				if (strncmp(bufr, QUIT_STRING, strlen(QUIT_STRING)) == 0) {
					/* leaving group */
					//printf("SANT: Got %s string. Leaving group..\n", bufr);
					sendpkt(sock, LEAVE_GROUP, 0, NULL);
					break;
				}

				/* send the text to the server */
				sendpkt(sock, USER_TEXT, strlen(bufr) + 1, bufr);
			}

		}

	}
}

/*--------------------------------------------------------------------*/
