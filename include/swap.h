#pragma once

#include <trace_buf.h>

#ifdef WIN64  /* use keyword to address alignment issues with 64-bit Windows */
#define _UNALIGNED __unaligned
#else
#define _UNALIGNED
#endif

/* include file for swap.c: handy routines for swapping earthwormy things */
void swap_uint16( void * );
#define swap_int16( data ) swap_uint16( data )
#define swap_short( data ) swap_uint16( data )
void swap_uint32( void * );
#define swap_int32( data ) swap_uint32( data )
#define swap_int( data ) swap_uint32( data )
#define swap_float( data ) swap_uint32( data )
void swap_uint64( _UNALIGNED void * );
#define swap_int64( data ) swap_uint64( data )
#define swap_double( data ) swap_uint64( data )

/* fixes wave message into local byte order, based on globals _SPARC and _INTEL */
int swap_wavemsg_makelocal( TRACE_HEADER* );
int swap_wavemsg2_makelocal( TRACE2_HEADER* );
int swap_wavemsg2x_makelocal( TRACE2X_HEADER* );
