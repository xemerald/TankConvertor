#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <search.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
/* */
#include <trace_buf.h>
#include <libmseed.h>
#include <swap.h>
#include <tb2sac.h>
/* */
#define VERSION         "0.0.2 - 2019-12-19"
#define AUTHOR          "Benjamin Ming Yang"
/* */
static int  scan_tracebuf( const void *, const void * );
static void sort_tlist( const void *, const VISIT, const int );
static void tree2list( const void *, const VISIT, const int );

static char         OutDir[128]   = { 0 };
static char         OutFormat[16] = { 0 };
static void        *Root      = NULL;         /* Root of binary tree */
static TRACE_NODE **TraceList = NULL;         /* List for remaping */

/*
 *
 */
int main( int argc, char *argv[] )
{
	int    i;
	int    ifd;         /* file of waveform data to read from   */
	struct stat fs;

	void *tankstart;
	void *tankend;
	int   totaltrace = 0;

	struct timespec tt1, tt2;  /* Nanosecond Timer */

	if ( argc != 2 ) {
		fprintf(stdout, "Usage: tb2sac <tankfile>\n***************************\n" );
		fprintf(stdout, "version: %s\n", VERSION );
		fprintf(stdout, "author:  %s\n", AUTHOR );
		return 0;
	}

/* Nanosecond Timer */
	clock_gettime(CLOCK_MONOTONIC, &tt1);

/* Open a waveform files
***********************/
	ifd = open(argv[1], O_RDONLY, 0);
	if ( ifd < 0 ) {
		fprintf(stderr, "%s Cannot open tankfile <%s>\n", progbar_now(), argv[1] );
		return -1;
	}

/* For future function... */
	sprintf(OutFormat, "SAC");

/* Create the output directory
******************************/
	sprintf(OutDir, "%s_%s", argv[1], OutFormat);
/* Move the index to the head of real filename, just skip the path */
	if ( strrchr(OutDir, '/') != NULL )
		i = strrchr(OutDir, '/') - OutDir;
	else
		i = 0;
/* Replace the '.' in the file name to '_' */
	for ( ; i < (int)strlen(OutDir); i++ )
		if ( OutDir[i] == '.' )
			OutDir[i] = '_';
/* Check if the directory is existing or not */
	if ( stat(OutDir, &fs) == -1 )
		mkdir(OutDir, 0775);

	fstat(ifd, &fs);
	fprintf(stdout, "%s Open the tankfile %s, size is %ld bytes.\n", progbar_now(), argv[1], (size_t)fs.st_size);
	fprintf(stdout, "%s Mapping the tankfile %s into memory...\n", progbar_now(), argv[1]);
	tankstart = mmap(NULL, (size_t)fs.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, ifd, 0);
	tankend   = (uint8_t *)tankstart + (size_t)fs.st_size;

	if ( (totaltrace = scan_tracebuf( tankstart, tankend )) <= 0 ) {
		fprintf(stderr, "%s Cannot mark all the tracebuf from <%s>\n", progbar_now(), argv[1] );
		return -1;
	}

	progbar_init( totaltrace );
	fprintf(stdout, "%s Estimation complete, total %d traces.\n", progbar_now(), totaltrace );

	for ( i = 0; i < totaltrace; i++ ) {
		sacproc_output( OutDir, tankstart, TraceList[i] );
	}

	munmap(tankstart, (size_t)fs.st_size);
	free(TraceList);
	close(ifd);

/* Nanosecond Timer */
	clock_gettime(CLOCK_MONOTONIC, &tt2);
	fprintf(
		stdout, "%s Convertion complete! Total processing time: %.3f sec.\n", progbar_now(),
		(float)(tt2.tv_sec - tt1.tv_sec) + (float)(tt2.tv_nsec - tt1.tv_nsec)* 1e-9
	);

	return 0;
}

/*
 *
 */
static int scan_tracebuf( void const *tankstart, void const *tankend )
{
	TRACE2_HEADER *trh2 = NULL;              /* tracebuf message read from file      */
	TRACE_NODE    *tnode, tkey;
	uint8_t       *tankbyte = NULL;

	char    byte_order;       /* byte order of this TYPE_TRACEBUF2 msg */
	int     byte_per_sample;  /* for TYPE_TRACEBUF2 msg                */
	int     totaltrace = 0;   /* total # trace read from file so far   */
	int     ntbuf      = 0;   /* total # msgs read of one trace        */
	size_t  skipbyte   = 0;   /* total # bytes skipped from last successed fetching */
	size_t  datalen;
	int     i, rc;
	double 	hdtime;           /* time read from TRACEBUF2 header       */
	int32_t nsamp;            /* #samples in this message              */


/* Point to the mapping memory
******************************/
	tankbyte = (uint8_t *)tankstart;

/* Initialize the key node */
	memset(&tkey, 0, sizeof(TRACE_NODE));

/* Read thru mapping memory reading headers; gather info about all tracebuf messages
**************************************************************************/
	do {
		trh2 = (TRACE2_HEADER *)tankbyte;
	/* Swap the byte order into local order, and check the validity of this tracebuf */
		if ( (rc = swap_wavemsg2_makelocal( trh2 )) < 0 ) {
			if ( rc == -1 ) {
				if ( ++tankbyte < (uint8_t *)tankend ) {
					skipbyte++;
					continue;
				}
				else {
					break;
				}
			}
			else {
				fprintf(stderr, "%s *** swap_wavemsg2_makelocal() failed! Skip this tracebuf! ***\n", progbar_now());
			}
		}
		else {
			if ( skipbyte ) {
				fprintf(
					stderr, "%s Shift total %ld bytes, found the next correct tracebuf for <%s.%s.%s.%s> %13.2f+%4.2f!\n",
					progbar_now(), skipbyte, trh2->sta, trh2->chan, trh2->net,
					trh2->loc, trh2->starttime, trh2->endtime-trh2->starttime
				);
			}
			skipbyte = 0;
		}

		hdtime          = trh2->endtime;
		nsamp           = trh2->nsamp;
		byte_order      = trh2->datatype[0];
		byte_per_sample = atoi(&trh2->datatype[1]);

	/* Store pertinent info about this tracebuf message
	**************************************************/
		strcpy(tkey.sta,  trh2->sta);
		strcpy(tkey.chan, trh2->chan);
		strcpy(tkey.net,  trh2->net);
		strcpy(tkey.loc,  trh2->loc);

	/* Check there are any space in the location code */
		for ( i = 0; i < (int32_t)strlen(tkey.loc); i++ )
			if ( tkey.loc[i] == ' ' )
				tkey.loc[i] = '-';

		tkey.tlist = NULL;

	/* Find which trace */
		if ( (tnode = tfind(&tkey, &Root, compare_SCNL)) == NULL ) {
		/* Not found in trace table, the whole new trace */
		/* Allocate the trace information memory */
			if ( (tnode = (TRACE_NODE *)calloc(1, sizeof(TRACE_NODE))) == NULL ) {
				fprintf(stderr, "%s Error allocate the memory for trace information!\n", progbar_now());
				return -1;
			}

		/* Copy the data into the new node */
			*tnode = tkey;
			tnode->ntbuf   = 0;
			tnode->maxtbuf = MAX_NUM_TBUF;
			tnode->tlist   = (TBUF *)calloc(MAX_NUM_TBUF, sizeof(TBUF));

		/* Insert the trace information into binary tree */
			if ( tsearch(tnode, &Root, compare_SCNL) == NULL ) {
				fprintf(stderr, "%s Error insert trace into binary tree!\n", progbar_now());
				return -1;
			}

		/* Keep track the total traces */
			totaltrace++;
		}
		else {
		/* Found in trace table, try to fetch the pointer */
			tnode = *(TRACE_NODE **)tnode;
		}

	/* Derive the data length of this tracebuf */
		datalen = byte_per_sample * nsamp;
	/* Fetch the # tracebuf to local memory, it would simplify the code */
		ntbuf = tnode->ntbuf;
	/* Fill in the pertinent info */
		tnode->tlist[ntbuf].offset = tankbyte - (uint8_t *)tankstart;
		tnode->tlist[ntbuf].size   = datalen + sizeof(TRACE2_HEADER);
		tnode->tlist[ntbuf].time   = hdtime;

	 /* Skip over data samples
	 ************************/
		if( tnode->tlist[ntbuf].size > MAX_TRACEBUF_SIZ ) {
			fprintf(stderr, "%s *** msg[%ld] overflows internal buffer[%d] ***\n",
				progbar_now(), tnode->tlist[ntbuf].size, MAX_TRACEBUF_SIZ );
			return -1;
		}
	/* Keep track the total bytes we have read, and move the pointer to the next header */
		tankbyte += tnode->tlist[ntbuf].size;

	/* Now check to see if we store this packet at all */
	/* Only track this tracebuf if swap_wavemsg2_makelocal() SUCCEEDED */
		if ( rc == 0 ) tnode->ntbuf++;

	/* Allocate more space if necessary
	**********************************/
		if ( tnode->ntbuf >= tnode->maxtbuf ) {
			tnode->maxtbuf <<= 1;
			tnode->tlist = realloc(tnode->tlist, tnode->maxtbuf*sizeof(TBUF));
			if ( tnode->tlist == NULL ) {
				fprintf(stderr, "%s *** Could not realloc list to %ld bytes ***\n",
					progbar_now(), tnode->maxtbuf*sizeof(TBUF));
				return -1;
			}
		}
	} while ( tankbyte < (uint8_t *)tankend );

/* Allocate the linear list for mapping the whole tree
******************************************************/
	TraceList = (TRACE_NODE **)calloc(totaltrace + 1, sizeof(TRACE_NODE *));
	twalk(Root, tree2list);

/* Sort the every tracebuf list of each trace by time */
	twalk(Root, sort_tlist);

	return totaltrace;
}

/*
 *
 */
static void sort_tlist( const void *nodep, const VISIT which, const int depth )
{
	TRACE_NODE *tnode = *(TRACE_NODE **)nodep;

/* Sort the TBUF structure on time */
	switch ( which ) {
	case postorder:
	case leaf:
		qsort( tnode->tlist, tnode->ntbuf, sizeof(TBUF), compare_time );
		break;
	case preorder:
	case endorder:
		break;
	}

	return;
}

/*
 *
 */
static void tree2list( const void *nodep, const VISIT which, const int depth )
{
	static int  nlist = 0;
	TRACE_NODE *tnode = *(TRACE_NODE **)nodep;

/* Remap the tree structure to line list */
	switch ( which ) {
	case postorder:
	case leaf:
		TraceList[nlist] = tnode;
		nlist++;
		break;
	case preorder:
	case endorder:
		break;
	}

	return;
}
