/*  main.c  - main */


#include <xinu.h>
#include <stdio.h>
#include<stdlib.h>

/************************************************************************/
/*									*/
/* main - main program for testing Xinu					*/
/*									*/
/************************************************************************/

char *itoa(int i,char*name);
void listfiles(void);
void createFiles(void);
int fileCount =6;
void getFilePaths(char filePathList[fileCount][160] );
void strcat(char *reslt,char*one,char*two);
int diskId =  -1;
int main(int argc, char **argv)
{
/*	umsg32 retval;


	resume(create(shell, 4096, 1, "shell", 1, CONSOLE));

	retval = recvclr();
	while (TRUE) {
		retval = receive();
		kprintf("\n\n\rMain process recreating shell\n\n\r");
		resume(create(shell, 4096, 1, "shell", 1, CONSOLE));
	}

	return OK;
	*/
	uint32 nBlocks = 8192;
	char *a = "8192";
	kprintf("Total blocks in the disk %s\r\n",a);
	kprintf("Total size of the disk %d\r\n",nBlocks*RAMD_BLKSZ);
	kprintf("opening the ramd disk drive\r\n");
	if((diskId = open(RDISK,a,NULL)) == SYSERR)
	{
		kprintf("Failed to open ramdisk \r\n");
		return SYSERR;
	}
	int iblockCount = 1024*4;
	kprintf("Creating initially empty file sytem on disk with %d iblocks\r\n",iblockCount);
	if(SYSERR == control(LFILESYS,LF_CTL_FORMAT,iblockCount,nBlocks* RAMD_BLKSZ))
	{
		kprintf("File system creation failed\r\n");
		return SYSERR;
	}
	lfsckfmt(RDISK);
	kprintf("Creating Direcotry \r\n");
	char *dirPath ="/dir1";
	if(control(LFILESYS,LF_CTL_MKDIR,(int)dirPath,0) == SYSERR)
	{
		kprintf("Direcotry creation failed\r\n");
		return SYSERR;
	}
	kprintf("Creating Direcotry  /dir1/dir2\r\n");
	dirPath ="/dir1/dir2";
	if(control(LFILESYS,LF_CTL_MKDIR,(int)dirPath,0) == SYSERR)
	{
		kprintf("Direcotry creation failed\r\n");
		return SYSERR;
	}
	kprintf("Creating Direcotry  /dir1/dir2/dir3\r\n");
	dirPath ="/dir1/dir2/dir3";
	if(control(LFILESYS,LF_CTL_MKDIR,(int)dirPath,0) == SYSERR)
	{
		kprintf("Direcotry creation failed\r\n");
		return SYSERR;
	}
	
	createFiles();
/*	int i =0;
	for(i=1;i<25l;++i)
	{
		kprintf("Opening  file named %d\r\n",i);
		char name[20];
		itoa(i,name);
		int fd = open(LFILESYS,name,"n");
		if(fd  == SYSERR)
		{
			kprintf("file open failed for %s\r\n",name);
		}
		kprintf("closing  file named %d\r\n",i);
		close(fd);
		kprintf("Total files %d\r\n",Lf_data.lf_dir.lfd_nfiles);

	}
	kprintf("Calling list files\r\n");
	listfiles();*/
	return OK;
}
char *itoa(int i,char*name)
{
	int j =0;
	while(i)
	{
		name[j++] = i%10;
		i/=10;
	}
	name[j]='\0';
	return name;
}

void listfiles(void)
{

}
void getFilePaths(char filePathList[fileCount][160] )
{
	int i =0;
	char fileName [] = {'/','f','i','l','e','_','0','0','0','\0'};
	for(i=0;i<fileCount-2;++i)
	{
		int k = i%100;
		fileName[6] = '0'+i/100;
		fileName[7] = '0'+k/10;
		fileName[8] = '0'+k%10;
	
		strcat(filePathList[i],"/dir1",fileName);
		strcat(filePathList[++i],"/dir1/dir2",fileName);
		strcat(filePathList[++i],"/dir1/dir2/dir3",fileName);
	
	}
}
void createData(int *data,int count)
{
	while(count>0)
	{
		*data++ = count--;
	}
}
bool8 matchData(int *data,int count)
{
	while(count>0)
	{
		if(*data++ != count--)
		{
			kprintf("Looking for %d but found %d\r\n",count+1,*(data-1));
		 	return 0;
		}
	}
	return 1;
}
void createFiles(void)
{
	int i =0;
	char filePathList[fileCount][160];
	getFilePaths(filePathList);
	int fid = -1;
	int dataToWrite[500];
	createData(dataToWrite,500);
	//char arr[500*4];
	for(i=0;i<fileCount;++i)
	{
		kprintf("Creating file %s\r\n",filePathList[i]);
		if((fid = open(LFILESYS,filePathList[i],"n"))== SYSERR)
		{
			kprintf("Failure\r\n");
			return ;
		}
		
		kprintf("Writting to the file\r\n");
		if(write(fid,(char*)dataToWrite,4*500)== SYSERR)
		{
			kprintf("Write failed\r\n");
		}
		kprintf("File size %u\r\n",lfltab[fid-5].fileSize);
	
		kprintf("closing the file\r\n");
		if(close(fid)== SYSERR)
		{
			kprintf("close failed\r\n");
		}
	}
	kprintf("Done Writting\r\n");	
		
	kprintf("Deleting Directory /dir1/dir2/dir3\r\n");
	char *path ="/dir1/dir2/dir3";
	if(control(LFILESYS,LF_CTL_RMDIR,(int)path,0) == SYSERR)
	{
		kprintf("Deleting directory /dir1/dir2/dir3 failed\r\n");
	}
	kprintf("Deleting Directory /dir1/dir2\r\n");
	path ="/dir1/dir2";
	if(control(LFILESYS,LF_CTL_RMDIR,(int)path,0) == SYSERR)
	{
		kprintf("Deleting directory /dir1/dir2 failed\r\n");
	}
	kprintf("Deleting Directory /dir1\r\n");
	path ="/dir1";
	if(control(LFILESYS,LF_CTL_RMDIR,(int)path,0) == SYSERR)
	{
		kprintf("Deleting directory /dir1 failed\r\n");
	}
	
	lfsckfmt(RDISK);
	/*kprintf("Removing file /dir1/file_000/ \r\n");
	char *path ="/dir1/file_000";
	if(control(LFILESYS,LF_CTL_DEL,(int)path,0) == SYSERR)
	{
		kprintf("Removing file /dir1/file_000/ failed\r\n");

	}
	kprintf("Removing file /dir1/dir2/dir3/file_003/ \r\n");

	path ="/dir1/dir2/dir3/file_003/";
	if(control(LFILESYS,LF_CTL_DEL,(int)path,0) == SYSERR)
	{
		kprintf("Removing file /dir1/dir2/dir3/file_003 failed\r\n");

	}
	for(i=0;i<fileCount;++i)
	{
		kprintf("Opening file %s\r\n",filePathList[i]);
		if((fid = open(LFILESYS,filePathList[i],"o"))== SYSERR)
		{
			kprintf("Failure\r\n");
			//return ;
			continue;
		}
		
		kprintf("File depth %u\r\n",lfltab[fid-5].depth);
		kprintf("File size %u\r\n",lfltab[fid-5].fileSize);
		kprintf("firstIbId %u\r\n",lfltab[fid-5].firstIbId);
		int k =0;
		kprintf("filePath ");
		for(k=0;k<lfltab[fid-5].depth;++k)
		{	
			kprintf("/%s",lfltab[fid-5].path[k]);
		}
		kprintf("\r\n");
		kprintf("Reading from the file\r\n");
		int bytesRead = 0;
		if((bytesRead =read(fid,arr,500*4)) == SYSERR)
		{
			kprintf("Read falied\n");
		}
		kprintf("Bytes Read %d\r\n",bytesRead);
		if(matchData((int*)arr,500))
		{
			kprintf("MATCH \r\n");
		}
		else
		{
			kprintf("MISMATCH\r\n");
		}
	//	int j=0;
	//	for(j=0;j<bytesRead;++j)
	//	{
	//		kprintf("%c\r\n",arr[j]);
	//	}
		kprintf("closing the file\r\n");
		if(close(fid)== SYSERR)
		{
			kprintf("close failed\r\n");
		}
	}
	kprintf("Done Opening\r\n");	
	kprintf("Creating file /dir1/file_000/ \r\n");

	if((fid = open(LFILESYS,"/dir1/file_000","n")) == SYSERR)
	{
		kprintf("Creating file /dir1/file_000/ failed\r\n");

	}
	if(write(fid,(char*)dataToWrite,4*500)== SYSERR)
	{
		kprintf("Write failed\r\n");
	}
	kprintf("closing the file\r\n");
	if(close(fid)== SYSERR)
	{
		kprintf("close failed\r\n");
	}
	kprintf("opening file /dir1/file_000/ \r\n");

	if((fid = open(LFILESYS,"/dir1/file_000","o")) == SYSERR)
	{
		kprintf("opening file /dir1/file_000/ failed\r\n");

	}	
	kprintf("Reading from the file\r\n");
	int bytesRead = 0;
	if((bytesRead =read(fid,arr,500*4)) == SYSERR)
	{
		kprintf("Read falied\n");
	}
	kprintf("Bytes Read %d\r\n",bytesRead);

	kprintf("Creating file /dir1/dir2/dir3/file_003/ \r\n");

	if((fid = open(LFILESYS,"/dir1/dir2/dir3/file_003/","n")) == SYSERR)
	{
		kprintf("Creating file /dir1/dir2/dir3/file_003/  failed\r\n");

	}
	if(write(fid,(char*)dataToWrite,4*500)== SYSERR)
	{
		kprintf("Write failed\r\n");
	}
	kprintf("closing the file\r\n");
	if(close(fid)== SYSERR)
	{
		kprintf("close failed\r\n");
	}
	kprintf("opening file /dir1/dir2/dir3/file_003/ \r\n");

	if((fid = open(LFILESYS,"/dir1/dir2/dir3/file_003","o")) == SYSERR)
	{
		kprintf("opening file /dir1/dir2/dir3/file_003 failed\r\n");

	}	
	kprintf("Reading from the file\r\n");
	if((bytesRead =read(fid,arr,500*4)) == SYSERR)
	{
		kprintf("Read falied\n");
	}
	kprintf("Bytes Read %d\r\n",bytesRead);*/


}

void strcat(char *reslt,char*one,char*two)
{
	char *temp = reslt;
	while(*one)
	{
		*temp ++ = *one++;	
	}
	while((*temp++ = *two++) != '\0');
}
