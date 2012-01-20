/* lflRead.c  -  lfRead */

#include <xinu.h>

/*------------------------------------------------------------------------
 * lflRead  --  read from a previously opened local file
 *------------------------------------------------------------------------
 */
devcall	lflRead (
	  struct dentry *devptr,	/* entry in device switch table */
	  char	*buff,			/* buffer to hold bytes		*/
	  int32	count			/* max bytes to read		*/
	)
{
	int32	numread;		/* number of bytes read		*/

	if (count < 0) {
		return SYSERR;
	}

	struct	lflcblk	*lfptr;		/* ptr to open file table entry	*/
	lfptr = &lfltab[devptr->dvminor];
	wait(lfptr->lfmutex);

	/* If file is not open, return an error */

	if (lfptr->lfstate != LF_USED) {
		signal(lfptr->lfmutex);
		return SYSERR;
	}

	/* Return EOF for any attempt to read beyond the end-of-file */

	if (lfptr->lfpos >= lfptr->fileSize) {
		signal(lfptr->lfmutex);
		return EOF;
	}

	numread = 0;
	do
	{
		/* If byte pointer is beyond the current data block, */
		/*	set up a new data block			     */
		if (lfptr->lfbyte >= &lfptr->lfdblock[LF_BLKSIZ]) 
		{
			lfsetup(lfptr);
		}
		*buff++ = (char)(0XFF & *lfptr->lfbyte++);
		/*if(DEBUG_1)
		{
			kprintf("Read %c\r\n",*(buff-1));
		}*/
		lfptr->lfpos++;
	}while(++numread < count && lfptr->lfpos < lfptr->fileSize);

	signal(lfptr->lfmutex);
	/*if(DEBUG_1)
	{
		kprintf("Returning numread %d\r\n",numread);
	}*/
	return (numread == 0 ?EOF:numread);
}
