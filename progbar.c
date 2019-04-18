#include <stdio.h>
#include <string.h>

/**/
volatile unsigned int TotalDuty = 0;
volatile unsigned int CompDuty  = 0;

/*
*/
char *nowprog( void )
{
	static char progstr[16] = { 0 };

	sprintf(progstr, "[%6.2f%%]", TotalDuty > 0 ? ((float)CompDuty/TotalDuty)*100.0 : 0.0);

	return progstr;
}
