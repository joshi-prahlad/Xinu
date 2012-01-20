/* rdsClose.c  -  rdsClose */

#include <xinu.h>

/*------------------------------------------------------------------------
 * rdsClose - Close a remote disk device
 *------------------------------------------------------------------------
 */
devcall	rdsClose (
	  struct dentry	*devptr		/* entry in device switch table	*/
	)
{
	struct	rdscblk	*rdptr;		/* ptr to control block entry	*/
	struct	rdbuff	*bptr;		/* ptr to buffer on a list	*/
	struct	rdbuff	*nptr;		/* ptr to next buff on the list	*/
	int32	nmoved;			/* number of buffers moved	*/
	struct	rd_msg_oreq msg;	/* message to be sent		*/
	struct	rd_msg_ores resp;	/* buffer to hold response	*/
	int32	retval;			/* return value from rdscomm	*/
	char	*idto;			/* ptr to ID string copy	*/
	char	*idfrom;		/* pointer into ID string	*/

	/* Device must be open */

	rdptr = &rdstab[devptr->dvminor];
	if (rdptr->rd_state != RD_OPEN) {
		return SYSERR;
	}

	/* Request queue must be empty */

	if (rdptr->rd_rhnext != (struct rdbuff *)&rdptr->rd_rtnext) {
		return SYSERR;
	}

	/* Move all buffers from the cache to the free list */

	bptr = rdptr->rd_chnext;
	nmoved = 0;
	while (bptr != (struct rdbuff *)&rdptr->rd_ctnext) {
		nmoved++;

		/* Unlink buffer from cache */

		nptr = bptr->rd_next;
		(bptr->rd_prev)->rd_next = nptr;
		nptr->rd_prev = bptr->rd_prev;

		/* Insert buffer into free list */

		bptr->rd_next = rdptr->rd_free;
	
		rdptr->rd_free = bptr;
		bptr->rd_status = RD_INVALID;

		/* Move to next buffer in the cache */

		bptr = nptr;
	}

	/* Hand-craft an close request message to be sent to the server */

	msg.rd_type = htons(RD_MSG_CREQ);/* Request an open		*/
	msg.rd_status = htons(0);
	msg.rd_seq = 0;			/* rdscomm fills in an entry	*/
	idto = msg.rd_id;
	memset(idto, NULLCH, RD_IDLEN);/* initialize ID to zero bytes	*/
	idfrom = rdptr->rd_id;
	while ( (*idto++ = *idfrom++) != NULLCH ) { /* copy ID to req.	*/
		;
	}

	/* Send message and receive response */

	retval = rdscomm((struct rd_msg_hdr *)&msg,
					sizeof(struct rd_msg_oreq),
			 (struct rd_msg_hdr *)&resp,
					sizeof(struct rd_msg_ores),
				rdptr );

	/* Check response */

	if (retval == SYSERR) {
		return SYSERR;
	} else if (retval == TIMEOUT) {
		kprintf("Timeout during remote file open\n\r");
		return SYSERR;
	} else if (ntohs(resp.rd_status) != 0) {
		return SYSERR;
	}

	/* Set the state to indicate the device is closed */

	rdptr->rd_state = RD_FREE;
	return OK;
}
