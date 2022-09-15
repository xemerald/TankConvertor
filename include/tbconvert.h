#pragma once

#include <trace_buf.h>
#include <libmseed.h>
#include <sachead.h>
/* */
#define PROG_NAME       "tbconvert"
#define VERSION         "1.0.0 - 2022-09-15"
#define AUTHOR          "Benjamin Ming Yang"
/* */
#define MAX_NUM_TBUF    4096
/* */
#define CONV_FORMAT_SAC     0
#define CONV_FORMAT_MSEED   1
#define CONV_FORMAT_MSEED3  2

/* */
typedef struct {
	size_t offset;    /* Offset in bytes from beginning of input file */
	size_t size;      /* Length in bytes of this TRACEBUF2 message    */
	double time;      /* A time from the header of this TRACEBUF2 msg */
} TBUF;
/* */
typedef struct {
	char  sta[TRACE2_STA_LEN];
	char  chan[TRACE2_CHAN_LEN];
	char  net[TRACE2_NET_LEN];
	char  loc[TRACE2_LOC_LEN];

	unsigned int ntbuf;
	unsigned int maxtbuf;
	TBUF *tlist;
} TRACE_NODE;

/* Progression bar function */
char *progbar_now( void );
int   progbar_init( const int );
int   progbar_inc( void );
/* Compare functions */
int compare_SCNL( const void *, const void * );
int compare_time( const void *, const void * );
/* SAC-related functions */
char *sacproc_outpath_gen( const char *, const char * );
int   sacproc_trace_output( const char *, void const *, TRACE_NODE * );
/* miniSEED-related functions */
char *msproc_outpath_gen( const char *, const char * );
int   msproc_tlist_add( MS3TraceList *, void const *, TRACE_NODE * );
