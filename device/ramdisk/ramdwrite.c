#include<xinu.h>

devcall ramdwrite(struct dentry *devptr,char*buffer,uint32 blkNum)
{
	if(ramDisk.nblks <=0 || ramDisk.blkarray == NULL)
	{
		if(DEBUG_1)
		{
			kprintf("RAMDWRITE: RAMDisk is not open");
		}
		return SYSERR;
	}
	if(isBadBlkId(blkNum) || blkNum >= ramDisk.nblks)
	{
		if(DEBUG_1)
		{
			kprintf("RAMDWRITE: toatl number of blocks %u\r\n",ramDisk.nblks);
			kprintf("RAMDWRITE: Invalid block id %u\r\n",blkNum);
		}
		return SYSERR;
	}
	if(!buffer)
	{
		if(DEBUG_1)
		{
			kprintf("RAMDWRITE: Invalid buffer Address  %u\r\n",buffer);
		}
		return SYSERR;
	}
	char *dest = ramDisk.blkarray[blkNum];
	if(!dest)
	{
		if(DEBUG_1)
		{
			kprintf("RAMDWRITE: Invalid block Address  %u\r\n",dest);
		}
		return SYSERR;
	}
	return 	ramdCopyBuffer(buffer,dest);
}

