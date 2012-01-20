/* xsh_testsuite.c - xsh_testsuite */

#include <xinu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <testsuite.h>

/*------------------------------------------------------------------------
 * xsh_testsuite - shell command that provides a menu of Xinu component
 *			tests.
 *------------------------------------------------------------------------
 */
shellcmd xsh_testsuite(int nargs, char *args[])
{
	int32	i;
	bool8	verbose = FALSE;		/* verbose mode?	*/
	int32	testnum;			/* number of test to run*/
	pid32	child;				/* ID of child process	*/
	char	*aptr;				/* ptr to arg that is a	*/
						/*	test number	*/
	char	*cptr;				/* ptr to next char in	*/
						/*	arg		*/
	char	ch;				/* next char of arg	*/

	/* For argument '--help', emit help about 'testsuite' */

	if (nargs == 2 && strncmp(args[1], "--help", 7) == 0) {
		printf("Use: %s [-v] <TESTNUM>\n\n", args[0]);
		printf("Description:\n");
		printf("\tSuite of Xinu test programs\n");
		printf("Options:\n");
		printf("\t<TESTNUM>\tif specified, specific test to run\n");
		printf("\t-v\t\tverbose information\n");
		printf("\t--help\t display this help and exit\n");
		return 0;
	}

	/* For no arguments, print a list of available tests */

	if (nargs == 1) {
		printf("Tests available in the test suite:\n");
		for (i = 0; i < ntests; i++) {
			printf("    %2d. %s\n", i+1, testtab[i].name);
		}
		return 0;
	}

	/* Check for valid number of arguments */

	if (nargs > 3) {
		fprintf(stderr, "%s: too many arguments\n", args[0]);
		fprintf(stderr, "Try --help for more information\n");
		return 1;
	}

	/* Parse arguments (at this point there are 2 or 3) */

	aptr = args[1];
	if (nargs == 3) {
		if (strncmp(args[1], "-v", 3) !=0 ) {
			fprintf(stderr, "%s: illegal argument (%s)\n",
				args[0], args[1]);
			return 1;
		}
		aptr = args[2];
		verbose = TRUE;
	}
	testnum = 0;
	cptr = aptr;
	ch = *cptr++;
	while (ch != NULLCH) {
		if ((ch < '0') || (ch > '9') ) {
			fprintf(stderr, "%s: illegal test number (%s)\n",
				args[0], aptr);
			return 1;
		}
		testnum = 10*testnum + (ch - '0');
		ch = *cptr++;
	}

	if (testnum > ntests) {
		fprintf(stderr, "%s: illegal test number (%s)\n",
			args[0], aptr);
		return 1;
	}
	/* run test */

	child = create(testtab[testnum].test, TESTSTK,
				   30, testtab[testnum].name, 1, verbose);
	if (child == SYSERR) {
		fprintf(stderr, "%s: cannot create process for test (%s)\n",
			args[0], aptr);
		return 1;
	}
	printf("Test Suite %2d: %s%s", testnum+1, testtab[testnum].name,
		(verbose ? "\n" : ""));

	/* set file descriptors */

	proctab[child].prdesc[0] = stdin;
	proctab[child].prdesc[1] = stdout;
	proctab[child].prdesc[2] = stderr;

	/* Clear waiting message and resume child */
	recvclr();
	resume(child);

	/* Wait for command process to finish */

	while (receive() != child) {
		;
	}
	return 0;
}
