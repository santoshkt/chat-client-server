///////////////////////////////////////////////////////////////////////////////
//
//  Filename: chatlinker.c
//  Author : Santosh K Tadikonda (stadikon@gmu.edu)
//  Date: Oct 25, 2013
//  Version: 1.0
//
///////////////////////////////////////////////////////////////////////////////

/*--------------------------------------------------------------------*/
/* functions to connect clients and server */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>

#include "common.h"
/*--------------------------------------------------------------------*/


/*----------------------------------------------------------------*/
/*
  prepares server to accept requests
  returns file descriptor of socket
  returns -1 on error
*/
int startserver()
{
  int     sd;      /* socket descriptor */
  int     myport;  /* server's port */
  const char *  myname;  /* full name of local host */

  char 	  linktrgt[MAXNAMELEN];
  char 	  linkname[MAXNAMELEN];

  /*
    FILL HERE
    create a TCP socket using socket()
  */

  sd = socket(PF_INET, SOCK_STREAM, 0);
  //printf("SANT: Created TCP Socked for server: %d\n", sd);

  /*
    FILL HERE
    bind the socket to some port using bind()
    let the system choose a port
  */

  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = htonl(INADDR_ANY);
  server_address.sin_port = htons(0);

  bind(sd, (struct sockaddr *) &server_address, sizeof(server_address));

  /* specify the accept queue length */
  // Queue length must be increased so that more people
  // can join.
  listen(sd, 20);

  /*
    FILL HERE
    figure out the full local host name and server's port
    use getsockname(), gethostname() and gethostbyname()
  */

  char hostname[MAXNAMELEN];

  if (gethostname(hostname, sizeof hostname) != 0)
  	perror("gethostname");

  struct hostent* h;
	h = gethostbyname(hostname);

  int len = sizeof(struct sockaddr);

  getsockname(sd, (struct sockaddr *) &server_address, &len);

  myname = h->h_name;
  myport = ntohs(server_address.sin_port);

  /* create the '.chatport' link in the home directory */
  sprintf(linktrgt, "%s:%d", myname, myport);
  sprintf(linkname, "%s/%s", getenv("HOME"), PORTLINK);
  if (symlink(linktrgt, linkname) != 0) {
    fprintf(stderr, "error : server already exists\n");
    return(-1);
  }

  /* ready to accept requests */
  printf("admin: started server on '%s' at '%d'\n",
	 myname, myport);
  return(sd);
}
/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
/*
  establishes connection with the server
  returns file descriptor of socket
  returns -1 on error
*/
int hooktoserver()
{
  int     sd;                    /* socket descriptor */

  char    linkname[MAXNAMELEN];
  char    linktrgt[MAXNAMELEN];
  char *  servhost;
  char *  servport;
  int     bytecnt;

  // printf("SANT: Hooking started..\n");

  /* locate server */
  //  printf("%s/%s\n", getenv("HOME"), PORTLINK);
  sprintf(linkname, "%s/%s", getenv("HOME"), PORTLINK);
  bytecnt = readlink(linkname, linktrgt, MAXNAMELEN);
  if (bytecnt == -1) {
    fprintf(stderr, "error : no active chat server\n");
    return(-1);
  }
  
  // printf("SANT: linktrgt: %s\n", linktrgt);

  linktrgt[bytecnt] = '\0';

  /* split addr into host and port */
  servport = index(linktrgt, ':');
  *servport = '\0';
  servport++;
  servhost = linktrgt;

  unsigned short number = (unsigned short) strtoul(servport, NULL, 0);
  // printf("SANT: Unsigned: %d", number);

  // printf("servport: %s, servhost: %s\n", servport, servhost);

  /*
    FILL HERE
    create a TCP socket using socket()
  */

  sd = socket(AF_INET, SOCK_STREAM, 0);
  //printf("SANT: Socket created: %d\n", sd);

  /*
    FILL HERE
    connect to the server on 'servhost' at 'servport'
    need to use gethostbyname() and connect()
  */

  struct hostent *hostinfo;
  struct sockaddr_in address;

  hostinfo = gethostbyname(servhost); /* look for host's name */
  address.sin_addr = *(struct in_addr *) *hostinfo->h_addr_list;
  address.sin_family = AF_INET;
  address.sin_port = htons(number);

  //printf("SANT: Connecting to server %s on port %s with fd %d\n", servhost, servport, sd );

  if (connect(sd, (struct sockaddr *) &address, sizeof(address)) < 0) {
  	perror("connecting");
  	exit(1);
  }

  /* succesful. return socket descriptor */
  printf("admin: connected to server on '%s' at '%s'\n",
	 servhost, servport);
  return(sd);
}
/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
int readn(int sd, char *buf, int n)
{
  int     toberead;
  char *  ptr;

  toberead = n;
  ptr = buf;
  while (toberead > 0) {
    int byteread;

    byteread = read(sd, ptr, toberead);
    if (byteread <= 0) {
      if (byteread == -1)
	perror("read");
      return(0);
    }

    toberead -= byteread;
    ptr += byteread;
  }
  return(1);
}

Packet *recvpkt(int sd)
{
  Packet *pkt;

  /* allocate space for the pkt */
  pkt = (Packet *) calloc(1, sizeof(Packet));
  if (!pkt) {
    fprintf(stderr, "error : unable to calloc\n");
    return(NULL);
  }

  /* read the message type */
  if (!readn(sd, (char *) &pkt->type, sizeof(pkt->type))) {
    free(pkt);
    return(NULL);
  }

  /* read the message length */
  if (!readn(sd, (char *) &pkt->lent, sizeof(pkt->lent))) {
    free(pkt);
    return(NULL);
  }
  pkt->lent = ntohl(pkt->lent);

  /* allocate space for message text */
  if (pkt->lent > 0) {
    pkt->text = (char *) malloc(pkt->lent);
    if (!pkt) {
      fprintf(stderr, "error : unable to malloc\n");
      return(NULL);
    }

    /* read the message text */
    if (!readn(sd, pkt->text, pkt->lent)) {
      freepkt(pkt);
      return(NULL);
    }
  }

  /* done reading */
  return(pkt);
}

int sendpkt(int sd, char typ, long len, char *buf)
{
  char tmp[8];
  long siz;

  /* write type and lent */
  bcopy(&typ, tmp, sizeof(typ));
  siz = htonl(len);
  bcopy((char *) &siz, tmp+sizeof(typ), sizeof(len));
  write(sd, tmp, sizeof(typ) + sizeof(len));

  /* write message text */
  if (len > 0)
    write(sd, buf, len);
  return(1);
}

void freepkt(Packet *pkt)
{
  free(pkt->text);
  free(pkt);
}
/*----------------------------------------------------------------*/
