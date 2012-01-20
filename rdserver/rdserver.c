/* rdserver.c - main */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <netdb.h>
#include <signal.h>

#include "xinudefs.h"
#include "rdisksys.h"
#include "rdserver.h"

struct	fentry ofiles[MAXDISKS];	/* open file table (one file	*/
					/*  per open disk)		*/
int	findex;				/* index of file in ofiles	*/
struct	fentry	*fptr;			/* pointer to entry in ofiles	*/

int	fnext = 0;			/* next entry in ofiles to test	*/

int	sock;				/* UDP socket used to receive	*/
					/*   requests & send responses	*/

unsigned short int port;	/* UDP port used to receive 	*/
					/*   requests & send responses  */

struct	sockaddr_in senderip;		/* the sender's address taken	*/
					/*   the current UDP packet	*/
int	addrlen;			/* size of the above address	*/


extern	void	*memset(void *, int, size_t);

void sigint_handler(int signum) {

	/* close all open files before exit */

	for (findex=0; findex<MAXDISKS; findex++) {
		fptr = &ofiles[findex];
		if (fptr->desc < 0) {
			continue;
		}
		close(fptr->desc);
	}
	printf("\n");
	exit(0);
}

/*------------------------------------------------------------------------
 * main - implement a Xinu remote file server running on a Unix machine
 *------------------------------------------------------------------------
 */
int	main (int argc, char *argv[]) 
{
	char	inbuf[MAXMSG];		/* space big enough for largest	*/
					/*   message that can arrive	*/ 
	char	outbuf[MAXMSG];		/* space big enough for largest	*/
					/*   response that can be sent	*/
	struct	rd_msg_hdr *mptr;	/* ptr to incoming request	*/
	struct	rd_msg_hdr *rptr;	/* ptr to outgoing response	*/
	struct	protoent *pptr;		/* ptr to protocol information	*/



	int	seq = 0;		/* sequence number in previous	*/
					/*   message			*/
	int	thisseq;		/* sequence in current packet	*/
	int	i, j;			/* general loop index variables	*/
	int	n;			/* number of bytes in UDP read	*/
	int	msgtyp;			/* message type in packet	*/
	int	err;			/* non-zero when error detected	*/
	int	retval;			/* return value			*/
	char	*to, *from;		/* used during name copy	*/
	int	len;			/* length of name		*/

	if (argc != 2) {
		printf("Usage: rdserver port\n");
		exit(1);
	}
	
	signal(SIGINT, sigint_handler);
	
	port = atoi(argv[1]);

	/* initialize table that records open files */

	for (i=0; i<MAXDISKS; i++) {
		ofiles[i].desc = -1;
		for (j=0; j<RD_IDLEN; j++) {
			ofiles[i].name[j] = NULLCH;
		}
	}

	memset((void *)&senderip, 0, (size_t) sizeof(senderip));
	senderip.sin_family = AF_INET;
	senderip.sin_addr.s_addr = INADDR_ANY;
	senderip.sin_port = htons(port);
	pptr = getprotobyname("udp");
	sock = socket(PF_INET, SOCK_DGRAM, pptr->p_proto);
	if (sock < 0) {
		printf("Error opening socket");
		exit(1);
	}

	/* Bind the socket */

	retval = bind( sock, (struct sockaddr *)&senderip,
			sizeof(senderip) );
	if (retval < 0) {
		printf("Error: cannot bind to UDP port %d\n\n",
				port);
		exit(1);
	} else {
		printf("Server is using UDP port %d\n\n",
				port);
}

	mptr = (struct rd_msg_hdr *)inbuf;
	rptr = (struct rd_msg_hdr *)outbuf;
	addrlen = sizeof(senderip);

	while (1) {

		n = recvfrom(sock, &inbuf, MAXMSG, 0,
			(struct sockaddr *) &senderip, &addrlen);

		/* ignore if message is too small or an error occurrred	*/

		if ( n < sizeof(struct rd_msg_hdr) ) {
			continue;
		}

		/* ignore if message type is invalid */

		msgtyp = ntohs(mptr->rd_type & 0xffff);
		if ( (msgtyp < RD_MIN_REQ) || (msgtyp > RD_MAX_REQ) ) {
			continue;
		}

		/* if incoming sequence is 1, reset local seq */

		thisseq = ntohl(mptr->rd_seq);
		if (thisseq == 1) {
			seq = 0;
		}

		/* ignore if sequence in packet is non-zero and is less	*/
		/*	than the server's sequence number		*/

		if ( (thisseq != 0) && (thisseq < seq) ) {
			continue;
		}

		seq = thisseq;		/* record sequence		*/

		/* Ignore if ID is too long */


		for (len=0; len<RD_IDLEN; len++) {
			if (mptr->rd_id[len] == NULLCH) {
				break;
			}
		}
		if (len >= RD_IDLEN) {
			snderr(mptr, rptr, sizeof(struct rd_msg_hdr) );
			continue;
		}

		/* Ignore if ID contains '/' */

		err = 0;
		for (i=0; i<len; i++) {
			if (mptr->rd_id[i] == NULLCH) {
				break;
			} else if (mptr->rd_id[i] == '/') {
				err = 1;
				break;
			}
		}
		if (err > 0) {
			snderr(mptr, rptr, sizeof(struct rd_msg_hdr) );
			continue;
		}

		/* see if we already have the file open for this ID */

		for (findex=0; findex<MAXDISKS; findex++) {
			fptr = &ofiles[findex];
			if (fptr->desc < 0) {
				continue;
			}
			if (strncmp(mptr->rd_id,fptr->name,RD_IDLEN)
				== 0) {
				break;
			}
		}
		if ( (findex >= MAXDISKS) ) {
			findex = -1;	/* file not found */
		}

		/* process message */

		switch (msgtyp) {

		case RD_MSG_OREQ:
			rdopen (  (struct rd_msg_oreq *)mptr,
				  (struct rd_msg_ores *)rptr );
			break;

		case RD_MSG_CREQ:
			rdclose(  (struct rd_msg_creq *)mptr,
				  (struct rd_msg_cres *)rptr );
			break;

		case RD_MSG_RREQ:
			rdread(	  (struct rd_msg_rreq *)mptr,
				  (struct rd_msg_rres *)rptr );
			break;

		case RD_MSG_WREQ:
			rdwrite(  (struct rd_msg_wreq *)mptr,
				  (struct rd_msg_wres *)rptr );
			break;

		case RD_MSG_DREQ:
			rddelete( (struct rd_msg_dreq *)mptr,
				  (struct rd_msg_dres *)rptr );
			break;

		}

	}
}
