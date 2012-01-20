#include<xinu.h>

devcall  ramdread(struct dentry *devptr,char*buffer,uint32 blkNum)
{
	if( ramDisk.nblks <=0 || ramDisk.blkarray == NULL)
	{
		if(DEBUG_1)
		{
			kprintf("RAMDREAD: RAMDisk is not open\r\n");
			kprintf("RAMDREAD: ramdisk.blkarray %d and ramDisk.nblks %d\r\n", ramDisk.blkarray,ramDisk.nblks);
		}
		return SYSERR;
	}
	if(isBadBlkId(blkNum) || blkNum >= ramDisk.nblks)
	{
		if(DEBUG_1)
		{
			kprintf("RAMDREAD: Invalid block id %u\r\n",blkNum);
		}
		return SYSERR;
	}
	if(!buffer)
	{
		if(DEBUG_1)
		{
			kprintf("RAMDREAD: Invalid buffer  %u\r\n",blkNum);
		}
		return SYSERR;
	}
	char * src = ramDisk.blkarray[blkNum];
	if(!src)
	{
		if(DEBUG_1)
		{
			kprintf("RAMDREAD: Invalid block Address  %u\r\n",blkNum);
		}
		return SYSERR;
	}
	return ramdCopyBuffer(src,buffer);	
}

syscall ramdCopyBuffer(char * src,char *dest)
{
	int bytesCopied = 0;
	while(bytesCopied < RAMD_BLKSZ && src && dest)
	{
		*dest++ = *src++;
		++bytesCopied;
	}
	if(bytesCopied != RAMD_BLKSZ)
	{
		return SYSERR;	
	}
	return OK;
}

