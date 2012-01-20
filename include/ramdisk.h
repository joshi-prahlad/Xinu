#ifndef NRAMD
#define NRAMD	1
#endif

#ifndef RAMD_BLKSZ
#define RAMD_BLKSZ	512
#endif

#ifndef RAMD_N_BLK
#define RAMD_N_BLK	(1024*64)
#endif

#ifndef DEBUG_1
#define DEBUG_1 0
#endif
struct ramdcblk{
	uint32 nblks;		/* The number of blocks in disk upper bounded by RAMD_N_BLK. */
	char **blkarray;	/* Array of pointers to the base address of each block. */	
};

extern struct ramdcblk ramDisk;

#define isBadBlkId(x) ((((uint32)(x)) < 0)||(((uint32)(x)) >= RAMD_N_BLK))

syscall ramdCopyBuffer(char * src,char *dest);
	
