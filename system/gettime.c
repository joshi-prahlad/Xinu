/* gettime.c - gettime */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  gettime  -  get local time in seconds past Jan 1, 1970
 *------------------------------------------------------------------------
 */
status	gettime(
	  uint32  *timvar		/* location to store the result	*/
	)
{
	uint32	now;			/* current time (UCT)		*/
	int32	retval;			/* return value from call	*/

	/* Get current time in UCT representation (GMT) */

	retval = getutime(&now);
	if (retval == SYSERR) {
		return SYSERR;
	}

	/* Adjust to local time and store result */

	*timvar = utim2ltim(now) + (Date.dt_daylight ? SECPERHR: 0);
	return OK;
}
