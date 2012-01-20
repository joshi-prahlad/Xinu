/* lfsOpen.c  -  lfsOpen */

#include <xinu.h>

status lfsOpenHelper(char *fileName,struct ldentry *dirEntry,int32 modeBits);

/*------------------------------------------------------------------------
 * lfsOpen - open a file and allocate a local file pseudo-device
 *------------------------------------------------------------------------
 */
devcall	lfsOpen (
	 struct	dentry	*devptr,	/* entry in device switch table	*/
	 char	*path,			/* complete path of file to open		*/
	 char	*mode			/* mode chars: 'r' 'w' 'o' 'n'	*/
	)
{
	did32		lfnext;		/* minor number of an unused	*/
					/*    file pseudo-device	*/
	struct	lflcblk	*lfptr;		/* ptr to open file table entry	*/
	int32	mbits;			/* mode bits			*/


	char pathTokens[LF_PATH_DEPTH][LF_NAME_LEN];  
	int pathDepth = tokenize(path,pathTokens);
	if(pathDepth == SYSERR)
	{
		if(DEBUG_1)
		{
			kprintf("lfsOpen Unable to tokenize the path %s\r\n",path);
		}
		return SYSERR;
	}
	if(1 == pathDepth && PATH_SEPARATOR==pathTokens[0][0])
	{
		if(DEBUG_1)
		{
			kprintf("lfsOpne: Can't open  root directory\r\n");
		}
		return SYSERR;
	}
	/* Parse mode argument and convert to binary */

	mbits = lfgetmode(mode);
	if (mbits == SYSERR) {
		return SYSERR;
	}

	/* If named file is already open, return SYSERR */

	lfnext = SYSERR;
	wait(lfDirCblkMutex);
	if(isFileOpen(pathTokens,pathDepth,&lfnext))
	{
		signal(lfDirCblkMutex);
		return SYSERR;
	}
	if (lfnext == SYSERR) {	/* no slave file devices are available	*/
		if(DEBUG_1)
		{
			kprintf("lfsOpen no slave devices are available\r\n");
		}
		signal(lfDirCblkMutex);
		return SYSERR;
	}

	struct ldentry fileInfo;
	/*
	 * Initialize lfltab[Nlfl+1] and lfltab[Nlfl]
	 * to parent and grandparent of the file we
	 * want to open.
	 * e.g. to /a/b and to /a/ if we want to open
	 * /a/b/c.
	 */
 	if(moveToDir(pathTokens,pathDepth-1) == SYSERR)
	{
		if(DEBUG_1)
		{
			kprintf("LfsOpen: failure in moving to the right directory \r\n");
		}
		signal(lfDirCblkMutex);
		return SYSERR;
	}	
	/*
	 * Either create a  new file or open an already existing file.
	 */
	if(lfsOpenHelper(pathTokens[pathDepth-1],&fileInfo,mbits) == SYSERR)
	{
		if(DEBUG_1)
		{
			kprintf("LfsOpen: failure in reading/crating file \r\n");
		}
		signal(lfDirCblkMutex);
		return SYSERR;
	}
	

	/* Initialize the local file pseudo-device */
	lfptr = &lfltab[lfnext];
	lfptr->lfstate = LF_USED;
	lfptr->lfmode = mbits & LF_MODE_RW;

	/* File starts at position 0 */

	lfptr->lfpos     = 0;

	/* Neither index block nor data block are initially valid	*/

	lfptr->lfinum    = LF_INULL;
	lfptr->lfdnum    = LF_DNULL;

	/* Initialize byte pointer to address beyond the end of the	*/
	/*	buffer (i.e., invalid pointer triggers setup)		*/

	lfptr->lfbyte = &lfptr->lfdblock[LF_BLKSIZ];
	lfptr->lfibdirty = FALSE;
	lfptr->lfdbdirty = FALSE;

	lfptr->fileSize = fileInfo.ld_size;
	lfptr->firstIbId = fileInfo.ld_ilist;
	memcpy(lfptr->path,pathTokens,LF_NAME_LEN * LF_PATH_DEPTH);
	lfptr->depth = pathDepth;

	signal(lfDirCblkMutex);
	return lfptr->lfdev;
}
/*
 * Assumes that parent and grandparent are already initialized
 * and fileName points to the name of the file.
 * dirEntry is initialized ot the entry of the file in the 
 * parent directory.
 */
status lfsOpenHelper(char *fileName,struct ldentry *dirEntry,int32 mbits)
{
	struct lflcblk * dirCblk = &lfltab[Nlfl+1];	/*last entry is used for modifying the directory in which file is getting created.*/
	struct lflcblk* parentDirCblk = &lfltab[Nlfl];	/*second last entry is used for parent of the directory in which file is getting created*/
	struct dentry devPtr;
	struct dentry parentDevPtr;
	devPtr.dvminor=Nlfl+1;	
	parentDevPtr.dvminor=Nlfl;	
	uint32 replacePos = 0;
	bool8 isRPosInitialized = 0;
	if(DEBUG_1)
	{
		kprintf("lfsOpen: dirCblk->filePos %u\r\n",dirCblk->lfpos);
		kprintf("llfsOpen: dirCblk->lfbyte %u\r\n",dirCblk->lfbyte - dirCblk->lfdblock);
	}
	/*
	 * Keep on reding the entries in the parent directory unless you find a match
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
		 * If we find a deleted entry we can reuse it 
		 * in case we create a new file.
		 */
		if(!dirEntry->isUsed)
		{
			if(!isRPosInitialized)
			{
				replacePos = dirCblk->lfpos - sizeof(struct ldentry);
				isRPosInitialized = 1;
			}
			continue;
		}
		/*
		 * We found a match.
		 */
		if(strcmp(dirEntry->ld_name,fileName) && dirEntry->isUsed)
		{
			if(DEBUG_1)
			{
				kprintf("TRUE\r\n");
			}
			if( LF_TYPE_DIR == dirEntry->type)
			{	
				/*Trying to open a directory	*/
				dirCblk->lfstate = LF_FREE;
				parentDirCblk->lfstate = LF_FREE;
				return SYSERR;
			}
			if (mbits & LF_MODE_N) 
			{	/* file must not exist	*/
				dirCblk->lfstate = LF_FREE;
				parentDirCblk->lfstate = LF_FREE;
				return SYSERR;
				if(DEBUG_1)
				{
					kprintf("lfsOpen File not found\r\n");	
				}
			}
			dirCblk->lfstate = LF_FREE;
			parentDirCblk->lfstate = LF_FREE;
			return OK;
		}
	}
	//File Not Found
	/*
	 * If we are opening a file and file doesn't exist
	 * then return error.
	 */
	if(mbits & LF_MODE_O)
	{
		dirCblk->lfstate = LF_FREE;
		parentDirCblk->lfstate = LF_FREE;
		if(DEBUG_1)
		{
			kprintf("lfsOpen File not found\r\n");	
		}
		return SYSERR;
	}
	/*
	 * If the file doesn't exist and mode bits have
	 * LF_MODE_N set then create the file.
	 */
	if(isRPosInitialized)
	{
		/*
		 * We can reuse an existing directory entry to create
		 * new file.
		 */
		if(DEBUG_1)
		{
			kprintf("lfsOpen: Moving to position replacePos\r\n");
		}
		if(lflSeek(&devPtr,replacePos) == SYSERR)
		{
			if(DEBUG_1)
			{
				kprintf("lfsOpne: Seek Failed");
			}
		}
	}
	/*
	 * Create the file
	 */
	if(SYSERR == createDirEntry(fileName,LF_TYPE_FILE,dirEntry,isRPosInitialized))
	{
		if(DEBUG_1)
		{
			kprintf("lfsOpen : call to createdirEntry failed\r\n");
		}
		dirCblk->lfstate = LF_FREE;
		parentDirCblk->lfstate = LF_FREE;
		return SYSERR;
	}
	return OK;
}






