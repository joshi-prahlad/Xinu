/* lflClose.c  -  lflClose.c */

#include <xinu.h>

static status lflCloseHelper(char *fileName,struct lflcblk* lfptr);
/*------------------------------------------------------------------------
 * lflClose  --  close a file by flushing output and freeing device entry
 *------------------------------------------------------------------------
 */
devcall	lflClose (
	  struct dentry *devptr		/* entry in device switch table */
	)
{
	struct	lflcblk	*lfptr;		/* ptr to open file table entry	*/

	/* Obtain exclusive use of the file */

	lfptr = &lfltab[devptr->dvminor];
	wait(lfptr->lfmutex);

	/* If file is not open, return an error */

	if (lfptr->lfstate != LF_USED) {
		signal(lfptr->lfmutex);
		if(DEBUG_1)
		{
			kprintf("lflClose fiel is not open\r\n");
		}
		return SYSERR;
	}

	/* Write index or data blocks to disk if they have changed */

	if (lfptr->lfdbdirty || lfptr->lfibdirty) {
		lfflush(lfptr);
	}

	/*Update the file entry in parent directory*/
	wait(lfDirCblkMutex);

	if(moveToDir(lfptr->path,lfptr->depth-1) == SYSERR)
	{
		if(DEBUG_1)
		{
			kprintf("lflClose: Unable to move to the directory\r\n");
		}
		signal(lfDirCblkMutex);
		signal(lfptr->lfmutex);
		return SYSERR;
	}
	if(lflCloseHelper(lfptr->path[lfptr->depth-1],lfptr) == SYSERR)
	{
		if(DEBUG_1)
		{
			kprintf("lflClose: Unable to update the parent directory\r\n");
		}
		signal(lfDirCblkMutex);
		signal(lfptr->lfmutex);
		return SYSERR;
	}
	/* Set device state to FREE and return to caller */

	lfptr->lfstate = LF_FREE;
	signal(lfDirCblkMutex);
	signal(lfptr->lfmutex);
	return OK;
}
static status lflCloseHelper(char *fileName,struct lflcblk* lfptr)
{
	struct lflcblk * dirCblk = &lfltab[Nlfl+1];	/*last entry is used for modifying the directory in which file is getting created.*/
	struct lflcblk* parentDirCblk = &lfltab[Nlfl];	/*second last entry is used for parent of the directory in which file is getting created*/
	struct dentry devPtr;
	struct dentry parentDevPtr;
	struct ldentry tempEntry;
	struct ldentry*dirEntry = &tempEntry;
	devPtr.dvminor=Nlfl+1;	
	parentDevPtr.dvminor=Nlfl;	
	bool8 found = 0;
	while(lflRead(&devPtr,(char*)dirEntry,sizeof(struct ldentry)) == sizeof(struct ldentry))
	{
		if(strcmp(dirEntry->ld_name,fileName) && dirEntry->isUsed)
		{
			found = 1;
			break;
		}
	}
	if(!found)
	{
		if(DEBUG_1)
		{
			kprintf("lflCloseHleper :file %s not found\r\n",fileName); 
		}

		dirCblk->lfstate = LF_FREE;
		parentDirCblk->lfstate = LF_FREE;
		return SYSERR;
	}
	
	dirEntry->ld_ilist = lfptr->firstIbId;
	dirEntry->ld_size = lfptr->fileSize;
	
	uint32 writePos = dirCblk->lfpos - sizeof(struct ldentry);
	/*if(DEBUG_1)
	{
		kprintf("lflCloseHelper :ld_ilist %d\r\n",dirEntry->ld_ilist);	
		kprintf("lflCloseHelper: fileSize %u\r\n",dirEntry->ld_size);	
		kprintf("lflcloseHelper: writePos %u\r\n",writePos);
		kprintf("lflcloseHelper: dirCblk->firstIbid %u\r\n",dirCblk->firstIbId);
		kprintf("lflcloseHelper: dirCblk->fileSize %u\r\n",dirCblk->fileSize);
	}*/
	if(lflSeek(&devPtr,writePos) == SYSERR)
	{
		if(DEBUG_1)
		{
			kprintf("LflCloseHelper: lflSeekfalied\r\n");
		}
	}
	/*if(DEBUG_1)
	{
		kprintf("After Seeking\r\n");
		kprintf("lflcloseHelper: dirCblk->filePos %u\r\n",dirCblk->lfpos);
		kprintf("lflcloseHelper: dirCblk->lfbyte %u\r\n",dirCblk->lfbyte - dirCblk->lfdblock);
	}*/
	if(lflWrite(&devPtr,(char*)dirEntry,sizeof(struct ldentry)) == SYSERR)
	{
		if(DEBUG_1)
		{
			kprintf("LFSCLOSE: Error in updating  directory's entry");
		}
		dirCblk->lfstate = LF_FREE;
		parentDirCblk->lfstate = LF_FREE;
		return SYSERR;
	}
	/*if(DEBUG_1)
	{
		kprintf("After Writing\r\n");
		kprintf("lflcloseHelper: dirCblk->filePos %u\r\n",dirCblk->lfpos);
		kprintf("lflcloseHelper: dirCblk->lfbyte %u\r\n",dirCblk->lfbyte - dirCblk->lfdblock);
		kprintf("lflcloseHelper: dirCblk->ibdirty %u\r\n",dirCblk->lfibdirty);
		kprintf("lflcloseHelper: dirCblk->dbdirty %u\r\n",dirCblk->lfdbdirty);
	}*/
	/*Close the directory*/
	if(lfflush(dirCblk) == SYSERR)
	{
		if(DEBUG_1)
		{
			kprintf("lfsClose: flushing the directory after file closing failed\r\n");
		}
		dirCblk->lfstate = LF_FREE;
		parentDirCblk->lfstate = LF_FREE;
		
		return SYSERR;
	}
	dirCblk->lfstate = LF_FREE;
	parentDirCblk->lfstate = LF_FREE;
	
	return OK;

}
