/* */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
/* */
#include <sachead.h>
#include <tbconvert.h>
#include <trace_buf.h>

/* */
static void init_header( struct SAChead * );
static void enrich_header_default( struct SAChead * );
static void enrich_sac_starttime( struct SAChead *, const double );
static int  enrich_sac_samps( SACWORD **, SACWORD const *, const TRACE2_HEADER *, void **, void const * );
static int  realloc_outbuf( uint8_t **, uint8_t **, size_t * );
static int  write_sac_file( const uint8_t *, const char * );

/* Macro */
#define REALIGN_SAC_FILE_PARTITION( OUTBUF, HEADER, SAMPS ) \
		__extension__({ \
			const int __OFFSET_IN_MACRO = SAMPS - (SACWORD *)(HEADER + 1); \
			HEADER = (struct SAChead *)OUTBUF; \
			SAMPS = (SACWORD *)(HEADER + 1) + __OFFSET_IN_MACRO; \
		})

/*
 *
 */
char *sacproc_outpath_gen( const char *inputfile, const char *outpath )
{
	static char result[128];
	int         i;
	struct stat fs;

/* Create the output directory */
	if ( !outpath || !strlen(outpath) ) {
		sprintf(result, "%s_SAC", inputfile);
	/* Move the index to the head of real filename, just skip the path */
		if ( strrchr(result, '/') != NULL )
			i = strrchr(result, '/') - result;
		else
			i = 0;
	/* Replace the '.' in the file name to '_' */
		for ( ; i < (int)strlen(result); i++ )
			if ( result[i] == '.' )
				result[i] = '_';
	}
	else {
		strcpy(result, outpath);
	}
/* Check if the directory is existing or not */
	if ( stat(result, &fs) == -1 )
		mkdir(result, 0775);

	return result;
}

/*
 *
 */
int sacproc_trace_output( const char *outpath, void const *tankstart, TRACE_NODE *tnode )
{
	int    i, j;
	int    gapcount  = 0;
	int    nfill_max = 0;
	int    nsamp_trace;
	double starttime, endtime; /* times for current scnl         */
	double samprate = 1.0;

	TRACE2_HEADER  *trh2     = NULL;  /* Tracebuf message read from file      */
	uint8_t        *tankbyte = NULL;
	void           *dataptr  = NULL;
	void           *dataend  = NULL;

	uint8_t        *outbuf       = NULL;
	uint8_t        *outbufend    = NULL;
	size_t          buffersiz    = sizeof(struct SAChead) + tnode->maxtbuf * 100 * sizeof(SACWORD);
	char            sacfile[256] = { 0 };
	struct SAChead *sachead      = NULL;
	SACWORD        *seis         = NULL;
	SACWORD         fill         = (SACWORD)SACUNDEF;

	fprintf(
		stdout, "%s Extracting %s.%s.%s.%s to SAC file...\n",
		progbar_now(), tnode->sta, tnode->chan, tnode->net, tnode->loc
	);

	outbuf    = (uint8_t *)malloc(buffersiz);
	outbufend = outbuf + buffersiz;
	sachead   = (struct SAChead *)outbuf;
	seis      = (SACWORD *)(sachead + 1);

	memset(outbuf, 0, buffersiz);

/* Initialize all the columns in the SAC header */
	init_header( sachead );
/* Set some columns to the default values that every one should be the same */
	enrich_header_default( sachead );
/* Copy the SCNL into the header and blank the trailing chars */
/* Station name */
	strcpy(sachead->kstnm, tnode->sta);
	for ( i = (int)strlen(tnode->sta); i < K_LEN; i++ )
		sachead->kstnm[i] = ' ';
/* Channel code */
	strcpy(sachead->kcmpnm, tnode->chan);
	for ( i = (int)strlen(tnode->chan); i < K_LEN; i++ )
		sachead->kcmpnm[i] = ' ';
/* Network code */
	strcpy(sachead->knetwk, tnode->net);
	for ( i = (int)strlen(tnode->net); i < K_LEN; i++ )
		sachead->knetwk[i] = ' ';
/* Location code */
	strcpy(sachead->khole, tnode->loc);
	for ( i = (int)strlen(tnode->loc); i < K_LEN; i++ )
		sachead->khole[i] = ' ';

/*
 * Orientation of seismometer -	determine the orientation based on the third character
 * of the component name
 */
	switch ( tnode->chan[2] ) {
/* Vertical component */
	case 'Z' :
	case 'z' :
		sachead->cmpaz  = 0.0;
		sachead->cmpinc = 0.0;
		break;
/* North-south component */
	case 'N' :
	case 'n' :
		sachead->cmpaz  = 0.0;
		sachead->cmpinc = 90.0;
		break;
/* East-west component */
	case 'E' :
	case 'e' :
		sachead->cmpaz  = 90.0;
		sachead->cmpinc = 90.0;
		break;
/* Anything else */
	default :
		sachead->cmpaz  = (float)SACUNDEF;
		sachead->cmpinc = (float)SACUNDEF;
		break;
	} /* switch */

	fprintf(
		stdout, "%s SAC header of %s.%s.%s.%s preparation complete!\n",
		progbar_now(), tnode->sta, tnode->chan, tnode->net, tnode->loc
	);
	nsamp_trace = 0;
	endtime     = time(NULL);
/* Go through the tracebuf list of this trace */
	for ( i = 0; i < tnode->ntbuf; i++ ) {
		tankbyte  = (uint8_t *)tankstart + tnode->tlist[i].offset;
		trh2      = (TRACE2_HEADER *)tankbyte;
		dataptr   = trh2 + 1;
		dataend   = tankbyte + tnode->tlist[i].size;
		starttime = trh2->starttime;
		samprate  = trh2->samprate;

		if ( i > 0 ) {
		/* Starttime is set for new packet; endtime is still set for old packet */
			if ( endtime + (2.0 / samprate) < starttime ) {
			/* There's a gap, so fill it */
				int    nfill   = (int)((float)samprate * (float)(starttime - endtime));
				size_t newsize = (nsamp_trace + nfill) * sizeof(SACWORD) + sizeof(struct SAChead);

				if ( newsize > buffersiz ) {
					if ( newsize > buffersiz * 2 ) {
						fprintf(stderr, "%s *** Bogus gap (%d); skipping! ***\n", progbar_now(), nfill);
						continue;
					}
					else {
					/* */
						if ( realloc_outbuf( &outbuf, &outbufend, &buffersiz ) ) {
							fprintf(
								stderr, "%s *** Could not realloc output buffer to %ld bytes ***\n",
								progbar_now(), buffersiz
							);
							return -1;
						}
					/* */
						REALIGN_SAC_FILE_PARTITION( outbuf, sachead, seis );
					}
				}
			/* Do the filling */
				for ( j = 0; j < nfill && seis < (SACWORD *)outbufend; j++, seis++ )
					*seis = fill;
				nsamp_trace += nfill;
			/* Keep track of how many gaps and the largest one */
				gapcount++;
				if ( nfill_max < nfill )
					nfill_max = nfill;
			}
			else if ( endtime + (1.0 / samprate) > trh2->endtime ) {
			/* This is a duplicated tracebuf, we'll just skip it */
				continue;
			}
		}
		else {
		/* gmttime makes juldays starting with 0 */
			enrich_sac_starttime( sachead, starttime );
		}

	/* Enrich data samples to the output buffer */
		if ( enrich_sac_samps( &seis, (SACWORD *)outbufend, trh2, &dataptr, dataend ) < 0 )
			continue;
	/* Allocate more space if necessary */
		if ( seis >= (SACWORD *)outbufend ) {
		/* */
			if ( realloc_outbuf( &outbuf, &outbufend, &buffersiz ) ) {
				fprintf(
					stderr, "%s *** Could not realloc output buffer to %ld bytes ***\n",
					progbar_now(), buffersiz
				);
				return -1;
			}
		/* */
			REALIGN_SAC_FILE_PARTITION( outbuf, sachead, seis );
		/* Enrich the left data samples to the output buffer */
			if ( enrich_sac_samps( &seis, (SACWORD *)outbufend, trh2, &dataptr, dataend ) < 0 )
				continue;
		}
	/* Advance endtime to the new packet; process this packet in the next iteration */
		endtime = trh2->endtime;
		nsamp_trace += trh2->nsamp;
	}
/* All trace data fed into SAC data section.  Now fill in the rest of header */
	sachead->npts  = (int32_t)nsamp_trace;                 /* Samples in trace */
	sachead->delta = (float)(1.0 / samprate);              /* Sample period */
	sachead->e     = (float)nsamp_trace * sachead->delta;  /* End time */
/* Output to the SAC file... */
	sprintf(sacfile, "%s/%s_%s_%s_%s.sac", outpath, tnode->sta, tnode->chan, tnode->net, tnode->loc);
	if ( write_sac_file( outbuf, sacfile ) ) {
		fprintf(stderr, "%s *** Error writing SAC file: %s ***\n", progbar_now(), strerror(errno));
		return -1;
	}
	free(outbuf);

/* If there is any gap in this trace try to notice the user */
	if ( gapcount ) {
		fprintf(
			stderr, "%s *** %d gaps; largest %d for <%s.%s.%s.%s> ***\n",
			progbar_now(), gapcount, nfill_max, tnode->sta, tnode->chan, tnode->net, tnode->loc
		);
	}
/* */
	progbar_inc();
	fprintf(
		stdout, "%s Finish the extraction of %s.%s.%s.%s to SAC file!\n",
		progbar_now(), tnode->sta, tnode->chan, tnode->net, tnode->loc
	);

	return 0;
}

/*
 *
 */
static void init_header( struct SAChead *head )
{
	int i;
/* Use a simple structure here - we don't care what the variables are - set them to 'undefined' */
	struct SAChead2 *head2 = (struct SAChead2 *)head;

/* Set all of the floats to 'undefined' */
	for ( i = 0; i < NUM_FLOAT; i++ )
		head2->SACfloat[i] = SACUNDEF;
/* Set all of the ints to 'undefined' */
	for ( i = 0; i < MAXINT-5; i++ )
		head2->SACint[i] = SACUNDEF;
/* Except for the logical integers - set them to 1 */
	for ( ; i < MAXINT; i++ )
		head2->SACint[i] = 1;
/* Set all of the strings to 'undefined' */
	for ( i = 0; i < MAXSTRING; i++ )
		strncpy(head2->SACstring[i], SACSTRUNDEF, K_LEN);

/* SAC I.D. number */
	head2->SACfloat[9] = SAC_I_D;
/* Header version number */
	head2->SACint[6] = SACVERSION;

	return;
}

/*
 *
 */
static void enrich_header_default( struct SAChead *head )
{
	head->idep   = SAC_IUNKN;      /* Unknown independent data type */
	head->iztype = SAC_IBEGINTIME; /* Reference time is Begin time */
	head->iftype = SAC_ITIME;      /* File type is time series */
	head->leven  = 1;              /* Evenly spaced data */
	head->b      = 0.0;            /* Beginning time relative to reference time */
	strncpy(head->ko, "origin ", K_LEN);

	return;
}

/*
 *
 */
static void enrich_sac_starttime( struct SAChead *head, const double starttime )
{
/* gmttime makes juldays starting with 0 */
	time_t ltime    = (time_t)starttime;
	struct tm *ltm  = gmtime(&ltime);

	head->nzyear = (int32_t)ltm->tm_year + (int32_t)1900; /* Calendar year of reference time */
	head->nzjday = (int32_t)ltm->tm_yday + (int32_t)1;    /* Julian day, 0 - 365 */
	head->nzhour = (int32_t)ltm->tm_hour;
	head->nzmin  = (int32_t)ltm->tm_min;
	head->nzsec  = (int32_t)ltm->tm_sec;
	head->nzmsec = (int32_t)((starttime - (int32_t)starttime) * 1000.0);

	return;
}

/*
 *
 */
static int enrich_sac_samps( SACWORD **samps, SACWORD const *bufend, const TRACE2_HEADER *trh2, void **srcptr, void const *srcend )
{
	int32_t   *idata;
	int16_t   *sdata;
	float     *fdata;
	double    *ddata;
	const char byte_order = trh2->datatype[0];         /* Byte order of this TYPE_TRACEBUF2 msg */
	const int  byte_per_sample = atoi(&trh2->datatype[1]);  /* For TYPE_TRACEBUF2 msg                */

/* */
	if ( byte_order == 'i' || byte_order == 's' ) {
	/* Integer 4 bytes */
		if ( byte_per_sample == 4 ) {
			for ( idata = (int32_t *)(*srcptr); idata < (int32_t *)srcend && *samps < bufend; (*samps)++, idata++ )
				**samps = *idata;
			*srcptr = idata;
		}
		else if ( byte_per_sample == 2 ) {
		/* Short integer 2 bytes */
			for ( sdata = (int16_t *)(*srcptr); sdata < (int16_t *)srcend && *samps < bufend; (*samps)++, sdata++ )
				**samps = *sdata;
			*srcptr = sdata;
		}
		else {
			return -1;
		}
	}
	else if ( byte_order == 'f' || byte_order == 't' ) {
	/* Following would not work now... */
		if ( byte_per_sample == 4 ) {
		/* Float 4 bytes */
			for ( fdata = (float *)(*srcptr); fdata < (float *)srcend && *samps < bufend; (*samps)++, fdata++ )
				**samps = *fdata;
			*srcptr = fdata;
		}
		else if ( byte_per_sample == 8 ) {
		/* Double float 8 bytes */
			for ( ddata = (double *)(*srcptr); ddata < (double *)srcend && *samps < bufend; (*samps)++, ddata++ )
				**samps = *ddata;
			*srcptr = ddata;
		}
		else {
			return -1;
		}
	}
	else {
		return -1;
	}

	return 0;
}

/*
 *
 */
static int realloc_outbuf( uint8_t **outbuf, uint8_t **outbufend, size_t *bufsize )
{
/* */
	*bufsize += (MAX_NUM_TBUF * 0.5) * 100 * sizeof(SACWORD);

	if ( (*outbuf = realloc(*outbuf, *bufsize)) == NULL )
		return -1;
/* */
	*outbufend = *outbuf + *bufsize;

	return 0;
}

/*
 *
 */
static int write_sac_file( const uint8_t *buffer, const char *path )
{
/* Compute the total size of the SAC file */
	size_t totalbyte = sizeof(struct SAChead) + ((struct SAChead *)buffer)->npts * sizeof(SACWORD);
/* Open the file and write all buffer data into the file */
	FILE *ofp = fopen(path, "wb");

	if ( fwrite(buffer, 1, totalbyte, ofp) != totalbyte )
		return -1;

	fclose(ofp);

	return 0;
}
