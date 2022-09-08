#include <stdio.h>
#include <string.h>

/*
 *
 */
volatile unsigned int TotalDuty;
volatile unsigned int CompDuty;

/*
 *
 */
char *progbar_now( void )
{
	static char progstr[16] = { 0 };

	if ( TotalDuty )
		sprintf(progstr, "[%6.2f%%]", ((float)CompDuty/TotalDuty)*100.0);
	else
		sprintf(progstr, "[-------]");

	return progstr;
}

/*
 *
 */
int progbar_init( const int total )
{
	CompDuty = 0;

	return (TotalDuty = total);
}

/*
 *
 */
int progbar_inc( void )
{
	return ++CompDuty;
}
