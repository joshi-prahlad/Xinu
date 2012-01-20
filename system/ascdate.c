/* ascdate.c - ascdate */

#include <xinu.h>
#include <stdio.h>

struct dateinfo Date = {0, FALSE, FALSE,
		       {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
		       {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
		        "Aug", "Sep","Oct","Nov","Dec"},
		       {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"}};

/*------------------------------------------------------------------------
 *  ascdate  -  format a date in ascii including day, year hours:mins:secs
 *------------------------------------------------------------------------
 */
status	ascdate (
	  uint32	now,		/* date and time in xinu format	*/
	  char		*str		/* string ( >= 20 chars long)	*/
	)
{
	uint32	tmp;			/* counts remaining seconds	*/
					/*   during computations	*/
	int32	year, month, day,	/* values for various parts of	*/
		hour, minute, second;	/*   the date			*/
	long	days;			/* number of days in a year	*/
	uint32	leapyrs;		/* number of leap years between	*/
					/*   1970 and now		*/
	uint32	shift;			/* number of times the calendar	*/
					/*    shifted by a day		*/
	uint32	dayofweek;		/* day of the week (0 - 6)	*/

	char	*zones[]  = {"EST", "CST", "MST", "PST"};
	char	*dzones[] = {"EDT", "CDT", "MDT", "PDT"};

	/* compute the year (1970-2099) */

	for (year=1970 ; TRUE ; year++) {
		days = isleap(year) ? 366 : 365;
		tmp = days * SECPERDY;
		if (tmp > now)
			break;
		now -= tmp;
	}

	/* compute the number of whole days that have already passed	*/
	/*   during the current year (0 through 365)			*/

	days = now / SECPERDY;

	/* compute the month (0-11) */

	for (month=0 ; month<12 ; month++) {
		tmp = Date.dt_msize[month] * SECPERDY;
		if ((month == 1) && isleap(year)) {
			tmp += SECPERDY;
		}
		if (tmp > now)
			break;
		now -= tmp;
	}

	/* Compute the day of month (1-31) */

	day = (int32)( now/SECPERDY ) + 1;
	now %= SECPERDY;

	/* Compute the hour (0-23) */

	hour = (int32) ( now/SECPERHR );
	now %= SECPERHR;

	/* Compute the minutes (0-59) */

	minute = now / SECPERMN;
	now %= SECPERMN;

	/* Compute the seconds (0-59) */
	second = (int32) now;

	/* Compute the day of the week (0-6)				*/
	/*								*/
	/*   Note: Jan 1, 1970 was a Thursday (4 on a 0 to 6 range)	*/
	/* Add one day of the week each	full year (365%7 == 1) plus	*/
	/* one extra day for each leap year				*/

	/* Compute number of leap years prior to current year */

	leapyrs = (year-1969)/4;

	/* Compute shift for all previous years (i.e., day of week of	*/
	/* Jan 1 of the current year)					*/

	shift = 4 + (year - 1970) + leapyrs;

	/* Add the number of days so far this year */

	shift += days;

	/* convert to integer from 0 through 6 */

	dayofweek = shift % 7;

	sprintf(str, "%3s  %3s %2d  %2d:%02d:%02d %s %d",
		Date.dt_dnam[dayofweek], Date.dt_mnam[month],
		day, hour, minute, second, Date.dt_daylight? dzones[TIMEZONE-5]:
			zones[TIMEZONE-5], year);
	return OK;
}
