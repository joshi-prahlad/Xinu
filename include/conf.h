/* conf.h (GENERATED FILE; DO NOT EDIT) */

/* Device switch table declarations */

/* Device table entry */
struct	dentry	{
	int32   dvnum;
	int32   dvminor;
	char    *dvname;
	devcall (*dvinit) (struct dentry *);
	devcall (*dvopen) (struct dentry *, char *, char *);
	devcall (*dvclose)(struct dentry *);
	devcall (*dvread) (struct dentry *, void *, uint32);
	devcall (*dvwrite)(struct dentry *, void *, uint32);
	devcall (*dvseek) (struct dentry *, int32);
	devcall (*dvgetc) (struct dentry *);
	devcall (*dvputc) (struct dentry *, char);
	devcall (*dvcntl) (struct dentry *, int32, int32, int32);
	void    *dvcsr;
	void    (*dvintr)(void);
	byte    dvirq;
};

extern	struct	dentry	devtab[]; /* one entry per device */

/* Device name definitions */

#define CONSOLE     0       /* type tty      */
#define NOTADEV     1       /* type null     */
#define ETHER0      2       /* type eth      */
#define RDISK       3       /* type rds      */
#define LFILESYS    4       /* type lfs      */
#define LFILE0      5       /* type lfl      */
#define LFILE1      6       /* type lfl      */
#define LFILE2      7       /* type lfl      */
#define LFILE3      8       /* type lfl      */
#define LFILE4      9       /* type lfl      */
#define LFILE5      10       /* type lfl      */

/* Control block sizes */

#define	Nnull	1
#define	Ntty	1
#define	Neth	1
#define	Nrds	1
#define	Nlfs	1
#define	Nlfl	6

#define DEVMAXNAME 24
#define NDEVS 11


/* Configuration and Size Constants */

#define	NPROC	     100	/* number of user processes		*/
#define	NSEM	     100	/* number of semaphores			*/
#define	IRQ_TIMER    IRQ_HW5	/* timer IRQ is wired to hardware 5	*/
#define	IRQ_ATH_MISC IRQ_HW4	/* Misc. IRQ is wired to hardware 4	*/
#define MAXADDR      0x02000000	/* 32 MB of RAM				*/
#define CLKFREQ      200000000	/* 200 MHz clock			*/
#define FLASH_BASE   0xBD000000	/* Flash ROM device			*/

#ifdef RD_SERVER_PORT
#undef RD_SERVER_PORT
#endif
#define RD_SERVER_PORT 	30139
#define LF_DISK_DEV     RDISK
