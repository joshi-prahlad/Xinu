/* lfflush.c  -  lfflush */

#include <xinu.h>

/*------------------------------------------------------------------------
 * lfflush  -  flush data block and index blocks for an open file
 *			(assumes file mutex is held)
 *------------------------------------------------------------------------
 */
status	lfflush (
	  struct lflcblk  *lfptr	/* ptr to file pseudo device	*/
	)
{
	if (lfptr->lfstate == LF_FREE) {
		return SYSERR;
	}

	/* Write data block if it has changed */

	if (lfptr->lfdbdirty) {
		if(DEBUG_1)
		{
			/*kprintf("lfflush on DataBlock wriintg block num %d\r\n",lfptr->lfdnum);
			int i =0;
			char arr[28];
			for(i=0;i<28;++i)
			{
				arr[i]= lfptr->lfdblock[i];	
			}
			struct ldentry *temp = (struct ldentry*)arr;
			kprintf("name %s and ibid %d size %u\r\n",temp->ld_name,temp->ld_ilist,temp->ld_size);*/
		}
		write(Lf_data.lf_dskdev, lfptr->lfdblock, lfptr->lfdnum);
		lfptr->lfdbdirty = FALSE;
	}

	/* Write i-block if it has changed */

	if (lfptr->lfibdirty) {
		if(DEBUG_1)
		{
			//kprintf("lfflush on IndexBlock wriintg block num %d\r\n",lfptr->lfinum);
		}
		lfibput(Lf_data.lf_dskdev, lfptr->lfinum, &lfptr->lfiblock);
		lfptr->lfibdirty = FALSE;
	}
	
	return OK;
}
