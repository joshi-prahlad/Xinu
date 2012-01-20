#include<xinu.h>

/*
* Given a full path of  a directory ,creates that directory
* assumes all the parent directories already exist 
* e.g. if call is like mkDir("/a/b/c)" then /a/b should
* already exist. If it doesn't exist or 'c' already exists
* then return SYSERR.
*/
status mkDir(char *path)
{
	char pathTokens[LF_PATH_DEPTH][LF_NAME_LEN];  
	/*Find out how deeply the directory is nested
	 * e.g. /a/b/c is at depht 3.
	 */
	int pathDepth = tokenize(path,pathTokens);
	if(pathDepth == SYSERR)
	{
		if(DEBUG_1)
		{
			kprintf("mkDir Unable to tokenize the path %s\r\n",path);
		}
		return SYSERR;
	}
	if(1 == pathDepth && PATH_SEPARATOR==pathTokens[0][0])
	{
		if(DEBUG_1)
		{
			kprintf("mkdir: Can't create  root directory\r\n");
		}
		return SYSERR;
	}
	wait(lfDirCblkMutex);
	/*
	 * initialize lfltabl[Nlfl+1] and lfltab[Nlfl] to 
	 * parent directory and to parent's parent directory
	 */
	if(moveToDir(pathTokens,pathDepth-1) == SYSERR)
	{
		if(DEBUG_1)
		{
			kprintf("mkdir: failure in moving to the right directory \r\n");
		}
		signal(lfDirCblkMutex);
		return SYSERR;
	}	
	struct lflcblk * dirCblk = &lfltab[Nlfl+1];	/*last entry is used for modifying the directory in which file is getting created.*/
	struct lflcblk* parentDirCblk = &lfltab[Nlfl];	/*second last entry is used for parent of the directory in which file is getting created*/
	struct dentry devPtr;
	struct dentry parentDevPtr;
	struct ldentry tempDirEntry;
	struct ldentry*dirEntry = &tempDirEntry;
	char*dirName = pathTokens[pathDepth-1];
	uint32 replacePos = 0;
	bool8 isRPosInitialized = 0;
	devPtr.dvminor=Nlfl+1;	
	parentDevPtr.dvminor=Nlfl;	

	/* Go throuh all the entries in the parent directory to find out whethere any
	 * of them is free. If none found then add an entry at the end.
	 */
	while(lflRead(&devPtr,(char*)dirEntry,sizeof(struct ldentry)) == sizeof(struct ldentry))
	{
		if(DEBUG_1)
		{
			kprintf("mvDir :ld_name %s\r\n",dirEntry->ld_name);
		}
		/* 
		 * An existing unused entry will be replaced.
		 * Record its posion.
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
		if(strcmp(dirEntry->ld_name,dirName) && dirEntry->isUsed)
		{
			if( LF_TYPE_DIR == dirEntry->type)
			{	
				/*directory already exists	*/
				if(DEBUG_1)
				{
					kprintf("MKDIR:directory already exists\r\n");
				}
			}
			else
			{
				/*A file with the same path name as directory already exists*/
				if(DEBUG_1)
				{
					kprintf("MKDIR:file of name as directory already exists\r\n");
				}

			}
			dirCblk->lfstate = LF_FREE;
			parentDirCblk->lfstate = LF_FREE;
			signal(lfDirCblkMutex);
			return SYSERR;
		}
	}
	/*
	 * Replace an existing unused  directory entry
	 */
	if(isRPosInitialized)
	{
		if(DEBUG_1)
		{
			kprintf("mkdir: Moving to position replacePos\r\n");
		}
		if(lflSeek(&devPtr,replacePos) == SYSERR)
		{
			if(DEBUG_1)
			{
				kprintf("mkDir: Seek Failed");
			}
		}
	}
	/*
	 * Create a new dir entry
	 */
	if(SYSERR == createDirEntry(dirName,LF_TYPE_DIR,dirEntry,isRPosInitialized))
	{
		if(DEBUG_1)
		{
			kprintf("mkDir : call to createdirEntry failed\r\n");
		}
		signal(lfDirCblkMutex);
		return SYSERR;
	}

	signal(lfDirCblkMutex);
	return OK;
}
