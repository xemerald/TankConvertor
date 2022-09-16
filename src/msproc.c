/* */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
/* */
#include <libmseed.h>
#include <tbconvert.h>
#include <trace_buf.h>

/*
 *
 */
char *msproc_outpath_gen( const char *inputfile, const char *outpath )
{
	static char result[MAX_PATH_LENGTH] = { 0 };
	char       *cptr = NULL;
	struct stat fs;

/* Create the output directory */
	if ( !outpath || !strlen(outpath) ) {
	/* Move the index to the head of real filename, just skip the path */
		if ( (cptr = strrchr(inputfile, '/')) == NULL )
			cptr = (char *)inputfile;
		else
			cptr++;
	/* */
		sprintf(result, "./%s", cptr);
	/* Replace the '.' in the file name to '_' */
		for ( cptr = result + 2; *cptr; cptr++ )
			if ( *cptr == '.' )
				*cptr = '_';
		strcat(result, ".mseed");
	}
	else {
		strcpy(result, outpath);
	}

	return result;
}

/*
 *
 */
int msproc_tlist_add( MS3TraceList *msl, void const *tankstart, TRACE_NODE *tnode )
{
	static int32_t *int32_buf = NULL;
	static int      int32_num = 0;
	int16_t        *int16_ptr = NULL;

	int             i, j;
	int             gapcount  = 0;
	int             nfill_max = 0;
	int             nfill     = 0;
	double          lendtime  = 0.0;
	TRACE2_HEADER  *trh2      = NULL;  /* Tracebuf message read from file      */
	uint8_t        *tankbyte  = NULL;

	char            byte_order;       /* Byte order of this TYPE_TRACEBUF2 msg */
	int             byte_per_sample;  /* For TYPE_TRACEBUF2 msg                */

	MS3Record      *msr = NULL;

/* */
	fprintf(
		stdout, "%s Extracting %s.%s.%s.%s to miniSEED record...\n",
		progbar_now(), tnode->sta, tnode->chan, tnode->net, tnode->loc
	);
/* Initialize the miniSEED header */
	if ( !(msr = msr3_init(msr)) ) {
		fprintf(stderr, "%s *** Could not allocate memory; skipping! ***\n", progbar_now());
		return -1;
	}
/* Turn the SCNL into the header sid and NULL the "--" location */
	ms_nslc2sid(
		msr->sid, LM_SIDLEN, 0,
		tnode->net,
		tnode->sta,
		strcmp(tnode->loc, LOC_NULL_STRING) ? tnode->loc : NULL,
		tnode->chan
	);
/* */
	msr->pubversion = 1;
	fprintf(
		stdout, "%s miniSEED record header of %s preparation complete!\n",
		progbar_now(), msr->sid
	);
/* */
	lendtime = (double)time(NULL);
/* Go through the tracebuf list of this trace */
	for ( i = 0; i < tnode->ntbuf; i++ ) {
		tankbyte = (uint8_t *)tankstart + tnode->tlist[i].offset;
		trh2     = (TRACE2_HEADER *)tankbyte;
	/* */
		msr->starttime   = (nstime_t)(MS_EPOCH2NSTIME(trh2->starttime) + 0.5);
		msr->samprate    = trh2->samprate;
		msr->numsamples  = trh2->nsamp;
		msr->samplecnt   = trh2->nsamp;
		msr->datasamples = trh2 + 1;
	/* */
		byte_order       = trh2->datatype[0];
		byte_per_sample  = atoi(&trh2->datatype[1]);
	/* Starttime is set for new packet; endtime is still set for old packet */
		if ( lendtime + (2.0 / trh2->samprate) < trh2->starttime ) {
			nfill = (int)((float)trh2->samprate * (float)(trh2->starttime - lendtime));
		/* Keep track of how many gaps and the largest one */
			gapcount++;
			if ( nfill_max < nfill )
				nfill_max = nfill;
		}
	/* Depends on the data type chose the pointer */
		if ( byte_order == 'i' || byte_order == 's' ) {
			if ( byte_per_sample == 2 ) {
				if ( trh2->nsamp > int32_num ) {
					int32_buf = (int32_t *)realloc(int32_buf, trh2->nsamp * sizeof(int32_t));
					int32_num = trh2->nsamp;
				}
			/* Convert 16-bit integers to 32-bit integers */
				int16_ptr = (int16_t *)msr->datasamples;
				for ( j = 0; j < trh2->nsamp; j++, int16_ptr++ )
					int32_buf[j] = *int16_ptr;
				msr->datasamples = int32_buf;
			}
			else if ( byte_per_sample == 4 ) {
			/* Do nothing! Just a placeholder! */
			}
			else {
				continue;
			}
			msr->sampletype = 'i';
		}
		else if ( byte_order == 'f' || byte_order == 't' ) {
		/* Following would not work now... */
			if ( byte_per_sample == 4 ) {
			/* Float 4 bytes */
				msr->sampletype = 'f';
			}
			else if ( byte_per_sample == 8 ) {
			/* Double float 8 bytes */
				msr->sampletype = 'd';
			}
			else {
				continue;
			}
		}
		else {
			continue;
		}
	/* Add the record to the trace list; and detach the sample buffer from the miniSEED record */
		if ( mstl3_addmsr(msl, msr, 0, 1, 1, NULL) == NULL ) {
			fprintf(stderr, "%s *** Error add record(%s) to trace list ***\n", progbar_now(), msr->sid);
		}
		msr->datasamples = NULL;
	/* Advance endtime to the new packet; process this packet in the next iteration */
		lendtime = trh2->endtime;
	}
/* If there is any gap in this trace try to notice the user */
	if ( gapcount ) {
		fprintf(
			stderr, "%s *** %d gap(s); largest %d for <%s.%s.%s.%s> ***\n",
			progbar_now(), gapcount, nfill_max, tnode->sta, tnode->chan, tnode->net, tnode->loc
		);
	}

/* */
	progbar_inc();
	fprintf(
		stdout, "%s Finish the extraction of %s.%s.%s.%s to miniSEED record!\n",
		progbar_now(), tnode->sta, tnode->chan, tnode->net, tnode->loc
	);
/* */
	msr3_free(&msr);

	return 0;
}
