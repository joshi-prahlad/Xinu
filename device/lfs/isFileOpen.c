#include<xinu.h>

static bool8 isEqualFilePath(char one[][LF_NAME_LEN],int len1,char two[][LF_NAME_LEN],int len2);

/*
 * Checks whether the file identified by pathTokens is open or
 * not. Also initializes firstFreeSlot to a free entry in lfltab.
 */
bool8 isFileOpen(char pathTokens[][LF_NAME_LEN],int pathDepth,int*firstFreeSlot)
{
	struct lflcblk*lfptr;
	int i=0;
	for (i=0; i<Nlfl; i++)
	{	/* search file pseudo-devices	*/
		lfptr = &lfltab[i];
		if (lfptr->lfstate == LF_FREE)
		{
			if(SYSERR == *firstFreeSlot)
			{
				*firstFreeSlot = i;
			}
		}
		else
		{
			/* Compare requested name to name of open file */
			if(isEqualFilePath(lfptr->path,lfptr->depth,pathTokens,pathDepth))
			{
				if(DEBUG_1)
				{
					kprintf("isFileOpen:file is already opnen\r\n");
				}
				signal(lfDirCblkMutex);
				return 1;
			}
		}
	}
	return 0;
}
/*
 * Checks whether two file paths are exactly same or not
 */
static bool8 isEqualFilePath(char one[][LF_NAME_LEN],int len1,char two[][LF_NAME_LEN],int len2)
{
	int i =0;
	if(len1!=len2)
		return 0;
	for(i=0;i<len1;++i)
	{
		if(!strcmp(one[i],two[i]))
		{
			return 0;
		}
	}
	return 1;
}
