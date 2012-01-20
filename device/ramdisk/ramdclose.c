#include<xinu.h>

devcall ramdclose(struct dentry *devptr)
{
	if(ramDisk.nblks == 0 && ramDisk.blkarray == NULL)
	{
		if(DEBUG_1)
		{
			kprintf("RAMDISK already closed\r\n");
		}
		return SYSERR;
	}
	int index =0;
	syscall status = 0;
	for(;index<ramDisk.nblks;++index)
	{
		if(freemem(ramDisk.blkarray[index],RAMD_BLKSZ)==SYSERR)
		{
			if(DEBUG_1)
			{
				kprintf("Failure in freeing block number %d\r\n",index);
			}
			status = -1;
		}
		ramDisk.blkarray[index] = NULL;
	}
	if(freemem((char*)(ramDisk.blkarray),sizeof(char*)*(ramDisk.nblks)) == SYSERR)
	{
		if(DEBUG_1)		
		{
			kprintf("Failure in freeing array of blocks%d\r\n",index);
		}
		status = -1;
	}
	if(status == -1)
	{
		return SYSERR;
	}
	ramDisk.blkarray = NULL;
	ramDisk.nblks = 0;
	return OK;	
}

