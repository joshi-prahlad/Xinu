#include<xinu.h>

/*
 * When a file/directory gets created we need to modify
 * not only its immediate parent directory but also its
 * grandparent.
 * e.g. if we are creating 'c' with full path /a/b/c
 * then we need to add/replace an entry in /a/b  and
 * might need to update 'b's entry in /a.
 * So moveToDir initializes lfltab[Nlfl+1] and lfltab
 * [Nlfl] to /a/b/ and to /a/ respectively.
 * It accepts the depth and tokenized
 * path upto immediate parent.
 * e.g. it should be passed 'a','b' and 2 in case we are creating
 * /a/b/c
 */
status moveToDir(char pathTokens[][LF_NAME_LEN],int fileDepth)
{
	struct lflcblk * dirCblk = &lfltab[Nlfl+1];	/*last entry is used for modifying the directory in which file is getting created.*/
	struct lflcblk* parentDirCblk = &lfltab[Nlfl];	/*second last entry is used for parent of the directory in which file is getting created*/
	struct dentry devPtr;
	struct dentry parentDevPtr;
	/*first read the  0th data block to find out 
	 * size of the root directory and first index block*/
	wait(Lf_data.lf_mutex);
	if(Lf_data.lf_dirpresent == FALSE)
	{
		/*Cache the root directory*/
		/*This should get executed only once*/
		struct  lfdir rootInfo;
		if(DEBUG_1)
		{
			kprintf("moveToDir : loading root directory\r\n");
		}
		if(read(Lf_data.lf_dskdev,(char*)&rootInfo,LF_AREA_ROOT) == SYSERR)
		{
			if(DEBUG_1)
			{
				kprintf("moveToDir:Unable to read root block id %d\r\n",LF_AREA_ROOT);
			}
			signal(Lf_data.lf_mutex);
			return SYSERR;
		}
		Lf_data.lf_dir = rootInfo;
		Lf_data.lf_dirpresent = TRUE;
		Lf_data.lf_dirdirty = FALSE;
	}
	signal(Lf_data.lf_mutex);
	
	
	devPtr.dvminor=Nlfl+1;	
	parentDevPtr.dvminor=Nlfl;	
	/*Point to the root directory*/
	resetLflCblk(dirCblk);
	resetLflCblk(parentDirCblk);
	dirCblk->lfstate = LF_USED;
	dirCblk->fileSize = Lf_data.lf_dir.lfd_size;
	dirCblk->firstIbId = Lf_data.lf_dir.lfd_ifirst;
	
	int currentDepth =0;		/*Root directory has a depth of zero. */
	struct ldentry currentDirEntry;
	struct ldentry*dirEntry = &currentDirEntry;
	/*
	 * Start from the root and keep on going down unless dirCblk points
	 * to last directory in the tokenized path and parentDirCblk points
	 * to its parent.
	 */
	while(currentDepth < fileDepth && lflRead(&devPtr,(char*)dirEntry,sizeof(struct ldentry)) == sizeof(struct ldentry))
	{
		if(strcmp(dirEntry->ld_name,pathTokens[currentDepth])&& dirEntry->isUsed)
		{
			/*
			 * Return error if something in the path
			 * is directory instead of a file.
			 */
			if(dirEntry->type != LF_TYPE_DIR)
			{
				if(DEBUG_1)
				{
					kprintf("moveToDir: looking for dir %s but it is a file\r\n",pathTokens[currentDepth-1]);
				}
				return SYSERR;
			}
			/*
			 * Save the parent of the current directory.
			 */
			memcpy(parentDirCblk,dirCblk,sizeof(struct lflcblk));
			
			/*Read this directory*/
			resetLflCblk(dirCblk);
			dirCblk->lfstate = LF_USED;
			dirCblk->fileSize = dirEntry->ld_size;
			dirCblk->firstIbId = dirEntry->ld_ilist;
			++currentDepth;
		}
	}
	if(fileDepth != currentDepth)
	{
		/*One of the required parent directory doesn;t exist*/
		if(DEBUG_1)
		{
			kprintf("moveToDir: One of the required parent directory doesn;t exist fileDepth is %d and currentDepth is %d\r\n",fileDepth,currentDepth);
		}	
		return SYSERR;
	}
	if(DEBUG_1)
	{
		kprintf("moveToDir: returning from depth  %d with dir name %s and dirCblk->IbId %d and fileSize %d %u %s\r\n",currentDepth,dirEntry->ld_name,dirCblk->firstIbId,dirCblk->fileSize,dirEntry->ld_size,dirEntry->ld_name);
	}

	return OK;
}
void resetLflCblk(struct lflcblk*dirCblk )
{
	dirCblk->lfstate = LF_FREE;
	dirCblk->lfpos     = 0;
	dirCblk->lfinum    = LF_INULL;
	dirCblk->lfdnum    = LF_DNULL;
	dirCblk->lfbyte =   &dirCblk->lfdblock[LF_BLKSIZ];
	dirCblk->lfibdirty = FALSE;
	dirCblk->lfdbdirty = FALSE;
	dirCblk->fileSize = -1;
	dirCblk->firstIbId = LF_INULL;
	memset((char*)dirCblk->path,NULLCH,LF_PATH_DEPTH*LF_NAME_LEN);
	dirCblk->depth = -1;
}
