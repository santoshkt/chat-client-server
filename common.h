/*--------------------------------------------------------------------*/
/* server port info link */
#define PORTLINK ".chatport"

/* buffer limits */
#define MAXNAMELEN 256
#define MAXPKTLEN  2048

/* messsges received/sent by server */
#define LIST_GROUPS    0
#define JOIN_GROUP     1
#define LEAVE_GROUP    2
#define USER_TEXT      3
#define JOIN_REJECTED  4
#define JOIN_ACCEPTED  5

/* structure of a packet */
typedef struct _packet {

  /* packet type */
  char      type;

  /* packet length */
  long      lent;

  /* packet text */
  char *    text;

} Packet;
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
extern int startserver();
extern int locateserver();

extern Packet *recvpkt(int sd);
extern int sendpkt(int sd, char typ, long len, char *buf);
extern void freepkt(Packet *msg);
/*--------------------------------------------------------------------*/
