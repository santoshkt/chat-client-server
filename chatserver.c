///////////////////////////////////////////////////////////////////////////////
//// 
////  Filename: chatserver.c
////  Author : Santosh K Tadikonda (stadikon@gmu.edu)
////  Date: Oct 25, 2013
////  Version: 1.0
////
///////////////////////////////////////////////////////////////////////////////

/*--------------------------------------------------------------------*/
/* chat server */

#include <stdlib.h>
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
#include <signal.h>

#include "common.h"
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/* info about a client */
typedef struct _member {

  /* member name */
  char *            name;

  /* member socket */
  int               sock;

  /* member of group */
  int               grid;

  /* next member */
  struct _member *  next;

  /* prev member */
  struct _member *  prev;

} Member;

/* info about a group */
typedef struct _group {

  /* group name */
  char *            name;

  /* maximum capacity */
  int               capa;

  /* current occupancy */
  int               occu;

  /* member list */
  struct _member *  mems;

} Group;

/* list of all groups */
Group *  group;
int      ngroups;
/*--------------------------------------------------------------------*/


/*--------------------------------------------------------------------*/
/* find the group with given name */
int findgroup(char *name)
{
  int grid;

  for (grid=0; grid < ngroups; grid++) {
    if (strcmp(group[grid].name, name) == 0)
      return(grid);
  }
  return(-1);
}

/* find the member with given name */
Member *findmemberbyname(char *name)
{
  int grid;

  /* go thru each group */
  for (grid=0; grid < ngroups; grid++) {
    Member *memb;

    /* go thru all members */
    for (memb = group[grid].mems; memb ; memb = memb->next) {
      if (strcmp(memb->name, name) == 0)
	return(memb);
    }
  }
  return(NULL);
}

/* find the member with given sock */
Member *findmemberbysock(int sock)
{
  int grid;

  /* go thru each group */
  for (grid=0; grid < ngroups; grid++) {
    Member *memb;

    /* go thru all members */
    for (memb = group[grid].mems; memb ; memb = memb->next) {
      if (memb->sock == sock)
	return(memb);
    }
  }
  return(NULL);
}
/*--------------------------------------------------------------------*/


/*--------------------------------------------------------------------*/
/* clean up before exit */
void cleanup()
{
  char linkname[MAXNAMELEN];

  /* unlink the link */
  sprintf(linkname, "%s/%s", getenv("HOME"), PORTLINK);
  unlink(linkname);

  exit(0);
}

/* main routine */
main(int argc, char *argv[])
{
  int    servsock;   /* server socket descriptor */
  int    maxsd;	     /* largest client socket descriptor */
  fd_set livesdset, tempset; /* set of live client sockets */


  /* check usage */
  if (argc != 2) {
    fprintf(stderr, "usage : %s <groups-file>\n", argv[0]);
    exit(1);
  }

  /* init groups */
  if (!initgroups(argv[1])) {
    exit(1);
  }

  /* setup signal handlers to clean up */
  signal(SIGTERM, cleanup);
  signal(SIGINT, cleanup);

  //printf("SANT: Before starting server..\n");
  /* get ready to receive requests */
  servsock = startserver();
  if (servsock == -1) {
    exit(1);
  }

  //printf("SANT: server started: %d\n", servsock);

  // startserver also binds and listens.

  /* init maxsd */
  maxsd = servsock;

  FD_ZERO(&livesdset);
  FD_ZERO(&tempset);
  FD_SET(servsock, &livesdset);

  /* receive requests and process them */
  while (1) {
    int    sock;    /* loop variable */

	tempset = livesdset;

    /*
      FILL HERE
      wait using select() for
        packets on existing clients and
	connect requests from new clients
    */

    select(maxsd + 1, &tempset, NULL, NULL, NULL);

    /* look for requests from existing clients */
    for (sock=3; sock <= maxsd; sock++) {

      if (FD_ISSET(sock, &tempset)) {
            // printf("Some one is set:  %d\n", sock);
      }

      /* skip the server socket */
      if (sock == servsock) {

		if (FD_ISSET(sock, &tempset)) {
			// printf("Its a server fd %d\n", frsock);
		}
		continue;

	  }

      /* FILL HERE: message from client 'sock'? */
      if(FD_ISSET(sock, &tempset)){
	Packet *pkt;

	/* read the message */
	pkt = recvpkt(sock);
	if (!pkt) {
	  /* client socket disconnected */
	  char *clientname;  /* host name of the client */

	  /*
	    FILL HERE:
	    figure out client's host name
	    using getpeername() and gethostbyaddr()
	  */

	  socklen_t len;
	  struct sockaddr_in addr;
	  len = sizeof(addr);
	  if (getpeername(sock, (struct sockaddr*) &addr, &len) == 0) {
	    struct sockaddr_in *s = (struct sockaddr_in *) &addr;
	    struct hostent *he;
	    he = gethostbyaddr(&s->sin_addr, sizeof(struct in_addr), AF_INET);
	    clientname = he->h_name;

	  }
	  else
	  {
	  	printf("Cannot get peer name");
	  }

	  printf("admin: disconnect from '%s' at '%d'\n",
		 clientname, sock);

	  /* revoke its membership */
	  leavegroup(sock);

	  /* close the socket */
	  close(sock);

	  /*
	    FILL HERE:
	    stop looking for packets from this 'sock' further
	  */

	  FD_CLR(sock, &livesdset);

	} else {
	  char *gname, *mname;

	  /* take action based on messge type */
	  switch (pkt->type) {
	    case LIST_GROUPS :
	      listgroups(sock);
	      break;
	    case JOIN_GROUP :
	      gname = pkt->text;
	      mname = gname + strlen(gname) + 1;
	      joingroup(sock, gname, mname);
	      break;
	    case LEAVE_GROUP :
	      leavegroup(sock);
	      break;
  	    case USER_TEXT :
	      relaymsg(sock, pkt->text);
	      break;
	  }

	  /* free the message */
	  freepkt(pkt);
	}
      }
    }

    struct sockaddr_in remoteaddr; // client address
	socklen_t addrlen;

    /* FILL HERE: connect request from a new client? */
    if(FD_ISSET(servsock, &tempset)){
      int  csd; /* new client socket descriptor */

      /*
	FILL HERE:
	accept a new connection request
      */

      addrlen = sizeof remoteaddr;
	  csd = accept(servsock, (struct sockaddr *) &remoteaddr, &addrlen);

      /* if accept is fine? */
      if (csd != -1) {
	char *clientname;

	/*
	  FILL HERE:
	  figure out client's host name using gethostbyaddr()
	*/

	struct hostent *h;
	h = gethostbyaddr((char *) &remoteaddr.sin_addr.s_addr,
					sizeof(struct in_addr), AF_INET);
	if (h != (struct hostent *) 0) {
		clientname = h->h_name;
	} else
		printf("gethostbyaddr failed\n");

	/* dispaly the client's hostname and the socket */
	printf("admin: connect from '%s' at '%d'\n",
	       clientname, csd);

	/*
	  FILL HERE
	  look for packets from 'csd' now on
	*/

	FD_SET(csd, &livesdset);

	/* keep track of the max */
	if (csd > maxsd)
	  maxsd = csd;
      } else {
	perror("accept");
	exit(0);
      }
    }
  }
}
/*--------------------------------------------------------------------*/


/*--------------------------------------------------------------------*/
/* init group list */
int initgroups(char *groupsfile)
{
  FILE *   fp;
  char     name[MAXNAMELEN];
  int      capa;
  int      grid;

  /* open the file */
  fp = fopen(groupsfile, "r");
  if (!fp) {
    fprintf(stderr, "error : unable to open file '%s'\n", groupsfile);
    return(0);
  }

  /* get the number of groups */
  fscanf(fp, "%d", &ngroups);

  /* allocate space for all groups */
  group = (Group *) calloc(ngroups, sizeof(Group));
  if (!group) {
    fprintf(stderr, "error : unable to calloc\n");
    return(0);
  }

  /* read info on all groups */
  for (grid =0; grid < ngroups; grid++) {
    /* get name and capa */
    if (fscanf(fp, "%s %d", name, &capa) != 2) {
      fprintf(stderr, "error : no info on group %d\n", grid+1);
      return(0);
    }

    /* fill in details */
    group[grid].name = strdup(name);
    group[grid].capa = capa;
    group[grid].occu = 0;
    group[grid].mems = NULL;
  }

  //printf("Groups initalized.");
  return(1);
}


/* list groups and their occupancies */
int listgroups(int sock)
{
  int      grid;
  char     pktbufr[MAXPKTLEN];
  char *   bufrptr;
  long     bufrlen;

  /* each piece of info is separated by null char */
  bufrptr = pktbufr;
  for (grid=0; grid < ngroups; grid++) {
    /* out group name */
    sprintf(bufrptr, "%s", group[grid].name);
    bufrptr += strlen(bufrptr) + 1;

    /* out group capacity */
    sprintf(bufrptr, "%d", group[grid].capa);
    bufrptr += strlen(bufrptr) + 1;

    /* out group occupancy */
    sprintf(bufrptr, "%d", group[grid].occu);
    bufrptr += strlen(bufrptr) + 1;
  }
  bufrlen = bufrptr - pktbufr;

  /* send the message */
  sendpkt(sock, LIST_GROUPS, bufrlen, pktbufr);
  return(1);
}

/* join the group as a member */
int joingroup(int sock, char *gname, char *mname)
{
  int       grid;
  Member *  memb;

  /* get hold of the group */
  grid = findgroup(gname);
  if (grid == -1) {
    char *errmsg = "no such group";
    sendpkt(sock, JOIN_REJECTED, strlen(errmsg), errmsg);
    return(0);
  }

  /* check if member name already exists */
  memb = findmemberbyname(mname);
  if (memb) {
    char *errmsg = "member name already exists";
    sendpkt(sock, JOIN_REJECTED, strlen(errmsg), errmsg);
    return(0);
  }

  /* check if room is full */
  if (group[grid].capa == group[grid].occu) {
    char *errmsg = "room is full";
    sendpkt(sock, JOIN_REJECTED, strlen(errmsg), errmsg);
    return(0);
  }

  /* make this a member of the group */
  memb = (Member *) calloc(1, sizeof(Member));
  if (!memb) {
    fprintf(stderr, "error : unable to calloc\n");
    cleanup();
  }
  memb->name = strdup(mname);
  memb->sock = sock;
  memb->grid = grid;
  memb->prev = NULL;
  memb->next = group[grid].mems;
  if (group[grid].mems) {
    group[grid].mems->prev = memb;
  }
  group[grid].mems = memb;
  printf("admin: '%s' joined '%s'\n", mname, gname);

  /* one more member in this group */
  group[grid].occu++;

  /* send acceptance message */
  sendpkt(sock, JOIN_ACCEPTED, 0, NULL);
  return(1);
}

/* leave the group */
int leavegroup(int sock)
{
  Member *  memb;

  /* get hold of the member */
  memb = findmemberbysock(sock);
  if (!memb) {
    return(0);
  }

  /* exclude from the group */
  if (memb->next) {
    memb->next->prev = memb->prev;
  }
  /* remove from ... */
  if (group[memb->grid].mems == memb) {
    /* the head of list */
    group[memb->grid].mems = memb->next;
  } else {
    /* the middle of list */
    memb->prev->next = memb->next;
  }
  printf("admin: '%s' left '%s'\n",
	 memb->name, group[memb->grid].name);

  /* one more left the group */
  group[memb->grid].occu--;

  /* free up member */
  free(memb->name);
  free(memb);
  return(1);
}

/* leave the group */
int relaymsg(int sock, char *text)
{
  Member *   memb;
  Member *   sender;
  char       pktbufr[MAXPKTLEN];
  char *     bufrptr;
  long       bufrlen;

  /* get hold of the member */
  sender = findmemberbysock(sock);
  if (!sender) {
    fprintf(stderr, "strange: no member at %d\n", sock);
    return(0);
  }

  /* prepend the member name to text */
  bufrptr = pktbufr;
  strcpy(bufrptr,sender->name);
  bufrptr += strlen(bufrptr) + 1;
  strcpy(bufrptr, text);
  bufrptr += strlen(bufrptr) + 1;
  bufrlen = bufrptr - pktbufr;

  /* send msg to all members of the group */
  for (memb = group[sender->grid].mems;
       memb; memb = memb->next) {
    /* skip the sender */
    if (memb->sock == sock) continue;

    /* send the pkt */
    sendpkt(memb->sock, USER_TEXT, bufrlen, pktbufr);
  }
  printf("%s: %s", sender->name, text);
  return(1);
}
/*--------------------------------------------------------------------*/
