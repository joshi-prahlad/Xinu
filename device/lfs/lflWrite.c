/* lflWrite.c  -  lfWrite */

#include <xinu.h>

/*------------------------------------------------------------------------
 * lflWrite  --  write data to a previously opened local disk file
 *------------------------------------------------------------------------
 */
devcall	lflWrite (
	  struct dentry *devptr,	/* entry in device switch table */
	  char	*buff,			/* buffer holding data to write	*/
	  int32	count			/* number of bytes to write	*/
	)
{
	int32		i;		/* number of bytes written	*/

	if (count < 0) {
		return SYSERR;
	}
	struct	lflcblk	*lfptr;		/* ptr to open file table entry	*/

	/* Obtain exclusive use of the file */

	lfptr = &lfltab[devptr->dvminor];
	wait(lfptr->lfmutex);

	/* If file is not open, return an error */

	if (lfptr->lfstate != LF_USED) {
		signal(lfptr->lfmutex);
		return SYSERR;
	}

	/* Return SYSERR for an attempt to skip bytes beyond the */
	/* 	current end of the file				 */

	if (lfptr->lfpos > lfptr->fileSize) {
		signal(lfptr->lfmutex);
		return SYSERR;
	}
	i =0 ;
	do
	{
		if (lfptr->lfbyte >= &lfptr->lfdblock[LF_BLKSIZ]) 
		{
		/* Set up block for current file position */
			lfsetup(lfptr);
		}
		/* If appending a byte to the file, increment the file size.	*/
		/* Note: comparison might be equal, but should not be greater.	*/

		if (lfptr->lfpos >= lfptr->fileSize ) {
			lfptr->fileSize++;
		}

		/* Place byte in buffer and mark buffer "dirty" */

		*lfptr->lfbyte++ = *buff++;
		lfptr->lfpos++;
		lfptr->lfdbdirty = TRUE;

	}while(++i < count);
	
	signal(lfptr->lfmutex);
	return count;
}
