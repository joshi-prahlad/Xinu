#include<xinu.h>

/* Returns number of tokens
 * and initializes tokens array to name of files/directories.
 * e.g. if path points to "/a/b/c" then tokens 
 * would have 'a','b','c' and 3 would be returned.
 * */
int tokenize(char *path,char tokens[][LF_NAME_LEN])
{
	/* If name represents a single '/' return it*/
	int i = 0;
	int tokenCount = 0;
	if(path[i] == PATH_SEPARATOR && path[i+1] == '\0')
	{
		tokens[0][0] = '/';
		tokens[0][1] = '\0';
		return 1;
	}
	int index = 0;
	
	while(tokenCount < LF_PATH_DEPTH)
	{
		if(path[index] == PATH_SEPARATOR)
		{
			++index;
		}
		for(i=0;path[index]&&path[index]!=PATH_SEPARATOR&&i<LF_NAME_LEN-1;++i)
		{
			tokens[tokenCount][i]=path[index];	
			++index;
		}
		if(i)
		{
			if(i >= LF_NAME_LEN-1)
			{
				if(DEBUG_1)
				{
					kprintf("lsOpen: file name %s is larger than limit %d\r\n",path,LF_NAME_LEN);
				}
				return SYSERR;
			}
			tokens[tokenCount][i] = '\0';
			if(DEBUG_1)
			{
				kprintf("Found token %s\r\n",tokens[tokenCount]);
			}
			++tokenCount;
			
		}
		if(path[index] == NULLCH || (path[index] == PATH_SEPARATOR && path[index+1] == NULLCH))
		{
			break;
		}
	}
	return tokenCount;
}
