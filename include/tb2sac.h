#pragma once

#include <trace_buf.h>
#include <sachead.h>

#define MAX_BUF_SIZ_KB  4096
#define MAX_NUM_TBUF    4096

typedef struct {
    int    offset;    /* Offset in bytes from beginning of input file */
    int    size;      /* Length in bytes of this TRACEBUF2 message    */
    double time;      /* A time from the header of this TRACEBUF2 msg */
} TBUF;

typedef struct {
	char  sta[TRACE2_STA_LEN];
	char  chan[TRACE2_CHAN_LEN];
	char  net[TRACE2_NET_LEN];
	char  loc[TRACE2_LOC_LEN];

	int   ntbuf;
	int   maxtbuf;
	TBUF *tlist;
} TRACE_NODE;

char *nowprog( void );

int compare_SCNL( const void *, const void * );
int compare_time( const void *, const void * );

void sacinit( struct SAChead * );
void sacdefault( struct SAChead * );
