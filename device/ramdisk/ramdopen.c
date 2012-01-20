#include<xinu.h>
#include<stdlib.h>
#include<ctype.h>

struct ramdcblk ramDisk = {0,NULL};
static uint32 strToUint32(char*);

devcall ramdopen(struct dentry *devptr,char*maxBlocks,char*mode)
{
	if(!(ramDisk.nblks == 0 &&  ramDisk.blkarray == NULL))
	{
		if(DEBUG_1)
		{
			kprintf("RAMDISK already in use\r\n");
		}
		return SYSERR;
	}
	int index = 0;

	uint32 nblks = strToUint32(maxBlocks);
	if(nblks == 0)
	{
		if(DEBUG_1)
		{
			kprintf("RAMDISKOPEN number of blocks not specified\r\n");
		}
		return SYSERR;
	}
	if(nblks < 1 || nblks > RAMD_N_BLK)
	{
		if(DEBUG_1)
		{
			kprintf("RAMDISKOPEN number of blocks not specified\r\n");
		}
		return SYSERR;
	}
	/*Allocate space for storing base address of memory blocks.*/
	char **blkarray = (char**)(getmem(nblks*sizeof(char*)));
	if((status)blkarray == -1)
	{
		if(DEBUG_1)
		{
			kprintf("Unable to allocated memory for block array\r\n");
		}
		return SYSERR;
	}

	for(;index<nblks;++index)
	{
		blkarray[index] = getmem(RAMD_BLKSZ);
		if((status)(blkarray[index]) == -1)
		{
			if(DEBUG_1)
			{
				kprintf("Unable to allocate memory for %d th block\r\n",index);
			}
			/*Free  already allocated blocks*/
			{
				int i = 0;
				for(;i<index;++i)
				{
					freemem(blkarray[i],RAMD_BLKSZ);		
				}
			}
			return SYSERR;
		}
	}
	ramDisk.blkarray = blkarray;
	ramDisk.nblks = nblks;

	return OK;	
}

static uint32 strToUint32(char *str)
{
	
	int index = 0;
	uint32 value = 0;
	if(str == NULL)
	{
		return 0;
	}
	while(str[index]&&isdigit(str[index]))
	{
		value = value*10 + (str[index] - '0');
		if(value > RAMD_N_BLK)
		{
			return 0;
		}
		++index;
	}
	if(str[index] != NULL)
	{
		return 0;
	}
	return value;
}
