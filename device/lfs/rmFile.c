#include<xinu.h>

/*
 * Frees all the data/index blocks and marks the directory entry
 * unused for a file identified by the argument path.
 */
status rmFile(char *path)
{
	char pathTokens[LF_PATH_DEPTH][LF_NAME_LEN];  
	int pathDepth = tokenize(path,pathTokens);
	int lfnext = SYSERR;
	if(pathDepth == SYSERR)
	{
		if(DEBUG_1)
		{
			kprintf("rmFile Unable to tokenize the path %s\r\n",path);
		}
		return SYSERR;
	}
	/*
	 * Don't delete the root directory.
	 */
	if(1 == pathDepth && PATH_SEPARATOR==pathTokens[0][0])
	{
		if(DEBUG_1)
		{
			kprintf("rmFile: Can't delete  root directory\r\n");
		}
		return SYSERR;
	}
	wait(lfDirCblkMutex);
	/*
	 * Don't delete an open file.
	 */
	if(isFileOpen(pathTokens,pathDepth,&lfnext))
	{
		if(DEBUG_1)
		{
			kprintf("rmFile: Can't delete  open file\r\n");
		}
		signal(lfDirCblkMutex);
		return SYSERR;
	}
	
	/*
	 * Initialize lfltab[Nlfl+1] and lfltab[Nlfl]
	 * to parent and grandparent of the file to 
	 * be deleted.
	 */
	if(moveToDir(pathTokens,pathDepth-1) == SYSERR)
	{
		if(DEBUG_1)
		{
			kprintf("rmFile: failure in moving to the right directory \r\n");
		}
		signal(lfDirCblkMutex);
		return SYSERR;
	}

	struct lflcblk * dirCblk = &lfltab[Nlfl+1];	
	struct lflcblk* fileCblk = &lfltab[Nlfl];
	struct dentry devPtr;
	struct dentry fileDevPtr;
	struct ldentry tempEntry;
	struct ldentry *dirEntry = &tempEntry;
	char *fileName = pathTokens[pathDepth-1];
	devPtr.dvminor=Nlfl+1;	
	fileDevPtr.dvminor=Nlfl;	
	bool8 found = 0;	
	/*
	 * Search for the file to be deleted in the parent directory.
	 */
	while(lflRead(&devPtr,(char*)dirEntry,sizeof(struct ldentry)) == sizeof(struct ldentry))
	{
		if(DEBUG_1)
		{
			kprintf("lfsOpne: rading name %s comaring with %s dirEntry->isUsed %d dirEntry->type %d\r\n",dirEntry->ld_name,fileName,dirEntry->isUsed,dirEntry->type);
			kprintf("lfsOpen: dirCblk->filePos %u\r\n",dirCblk->lfpos);
			kprintf("llfsOpen: dirCblk->lfbyte %u\r\n",dirCblk->lfbyte - dirCblk->lfdblock);

		}
		/*
		 * We found a match.
		 */
		if(strcmp(dirEntry->ld_name,fileName) && dirEntry->isUsed)
		{
			if( LF_TYPE_DIR == dirEntry->type)
			{	
				/*Trying to delete directory	*/
				if(DEBUG_1)
				{
					kprintf("rmFile: Can't delete a directory");
				}
				dirCblk->lfstate = LF_FREE;
				fileCblk->lfstate = LF_FREE;
				signal(lfDirCblkMutex);
				return SYSERR;
			}
			found = 1;
			//Truncate the file
			resetLflCblk(fileCblk);
			fileCblk->lfstate = LF_USED;
			fileCblk->firstIbId = dirEntry->ld_ilist;
			fileCblk->fileSize = dirEntry->ld_size;
			if(DEBUG_1)
			{
				kprintf("rmFile: Truncating File\r\n");
			}
			wait(Lf_data.lf_mutex);
			lftruncate(fileCblk);
			fileCblk->lfstate = LF_FREE;
			signal(Lf_data.lf_mutex);
			dirEntry->isUsed = (bool8)0;
			dirEntry->ld_ilist = LF_INULL;
			dirEntry->ld_size = 0;
			memset(dirEntry->ld_name,NULLCH,LF_NAME_LEN);
			break;
		}	
	}	
	if(!found)
	{
		if(DEBUG_1)
		{
			kprintf("rmFile: file not found\r\n");
		}
		dirCblk->lfstate = LF_FREE;
		fileCblk->lfstate = LF_FREE;
		signal(lfDirCblkMutex);
		return SYSERR;
	}
	/*The file got deleted so update its entry in its parent 
	 * directory.
	 * lfpos points to the position just after the entry of the
	 * deleted file.
	 */
	if(lflSeek(&devPtr,dirCblk->lfpos - sizeof(struct ldentry)) == SYSERR)
	{
		if(DEBUG_1)
		{
			kprintf("rmFile: SeekFailed\r\n");
		}
		dirCblk->lfstate = LF_FREE;
		fileCblk->lfstate = LF_FREE;
		signal(lfDirCblkMutex);
		return SYSERR;
	}
	/*
	* Overwrite the entry for the deleted file .
	*/
	if(lflWrite(&devPtr,(char*)dirEntry,sizeof(struct ldentry)) == SYSERR)
	{
		if(DEBUG_1)
		{
			kprintf("rmFile: Write failed\r\n");
		}
		dirCblk->lfstate = LF_FREE;
		fileCblk->lfstate = LF_FREE;
		signal(lfDirCblkMutex);
		return SYSERR;
	}
	/*
	 * Write to disk.
	 */
	if(lfflush(dirCblk) == SYSERR)
	{
		if(DEBUG_1)
		{
			kprintf("rmFile: flusing failed \r\n");
		}
		dirCblk->lfstate = LF_FREE;
		fileCblk->lfstate = LF_FREE;
		signal(lfDirCblkMutex);
		return SYSERR;
	}
	dirCblk->lfstate = LF_FREE;
	fileCblk->lfstate = LF_FREE;
	signal(lfDirCblkMutex);
	return OK;
}
