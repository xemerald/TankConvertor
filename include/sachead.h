#pragma once
/* */
#include <stdint.h>

/*
 *      SAC Header Include File
 *      This file contains structures and definitions for SAC file headers.
 *      If you are looking for SAC putaway routines, look in sacputaway.h
 *
 *      This include file should be used when ever the SAC header
 *      is accessed.
 */

#define SACHEADERSIZE   632          /* # of bytes in SAC header */

#define SACVERSION      6            /* version number of SAC */
#define SAC_I_D         2.0          /* SAC I.D. number */

#define SACWORD         float        /* SAC data is an array of floats */
#define SACUNDEF        -12345       /* undefined value */
#define SACSTRUNDEF     "-12345  "   /* undefined value for strings */

#define SACTEMP4        11

#define SACTEMP6        3
#define SACTEMP7        10

#define KEVNMLEN        16      /* length of event name */
#define K_LEN           8       /* length of all other string fields */

/* header value  constants */
#define SAC_IUNKN       5l
#define SAC_IDISP       6l
#define SAC_IVEL        7l
#define SAC_IACC        8l
#define SAC_IVOLTS      50l
#define SAC_IBEGINTIME  9l
#define SAC_ITIME       1l
#define SAC_IO         11l

#define NUM_FLOAT       70      /* number of floats in header */
#define MAXINT          40      /* number of ints in header */
#define MAXSTRING       24      /* number of strings in header */

/*
 * TYPE DECLARATIONS
 */

/*
 * This is the actual data structure which is used to enter the header
 * variables.
 */

struct SAChead {
/* floating point fields */
	float delta;    /* nominal increment between evenly spaced data (sec) */
	float depmin;   /* min value of trace data (dep var) */
	float depmax;   /* max value of trace data (dep var) */
	float scale;    /* scale factor - NOT USED */
	float odelta;   /* Observed delta if different from delta */
	float b;        /* beginning value of time (indep var)  */
	float e;        /* ending value of time (indep var) */
	float o;        /* event origin time, relative to reference time */
	float a;        /* first arrival time, relative to reference time */
	float internal1;
	float t0;       /* user-defined times */
	float t1;
	float t2;
	float t3;
	float t4;
	float t5;
	float t6;
	float t7;
	float t8;
	float t9;
	float f;        /* End of event time, relative to reference time */
	float resp0;
	float resp1;
	float resp2;
	float resp3;
	float resp4;
	float resp5;
	float resp6;
	float resp7;
	float resp8;
	float resp9;
	float stla;     /* station latitude (deg north) */
	float stlo;     /* station longitude (deg east) */
	float stel;     /* station elevation (meters) */
	float stdp;
	float evla;     /* event location, latitude (deg north) */
	float evlo;     /* event location, longitude (deg east) */
	float evel;
	float evdp;     /* event depth (km) */
	float blank1;
	float user0;    /* user defined values */
	float user1;
	float user2;
	float user3;
	float user4;
	float user5;
	float user6;
	float user7;
	float user8;
	float user9;
	float dist;     /* event-station epicentral distance (km) */
	float az;       /* event to station azimuth (deg) */
	float baz;      /* event to station back-azimuth (deg) */
	float gcarc;    /* event to station arc distance (deg) */
	float internal2;
	float internal3;
	float depmen;
	float cmpaz;    /* component azimuth (deg) */
	float cmpinc;   /* component inclination (deg) */
	float blank4[SACTEMP4];

/* integer fields */
	int32_t nzyear;    /* Reference time = Trace beginning time (year) */
	int32_t nzjday;    /*  (julian day) */
	int32_t nzhour;    /*  (hour) */
	int32_t nzmin;     /*  (minute) */
	int32_t nzsec;     /*  (second) */
	int32_t nzmsec;    /*  (millisecond) */
	int32_t internal4;
	int32_t internal5;
	int32_t internal6;
	int32_t npts;       /* number of points in trace */
	int32_t internal7;
	int32_t internal8;
	int32_t blank6[SACTEMP6];
	int32_t iftype;      /* Type of data:  1 for time series */
	int32_t idep;        /* Type of dependent data =UNKNOWN */
	int32_t iztype;      /* zero time equivalence  =1 for beginning */
	int32_t iblank6a;
	int32_t iinst;
	int32_t istreg;
	int32_t ievreg;
	int32_t ievtyp;      /* event type; IUNKN */
	int32_t iqual;
	int32_t isynth;
	int32_t blank7[SACTEMP7];
	uint32_t leven;    /* =1 for evenly spaced data */
	uint32_t lpspol;   /* =1 for correct polarity, 0 for reversed */
	uint32_t lovrok;
	uint32_t lcalda;
	uint32_t lblank1;

/* character string fields */
	char kstnm[K_LEN];      /* station name (blank padded) */
	char kevnm[KEVNMLEN];   /* event name */
	char khole[K_LEN];      /* for SEED location code, per Peter Goldstein */
	char ko[K_LEN];         /* Origin time label */
	char ka[K_LEN];         /* First arrival time label */
	char kt0[K_LEN];        /* labels for user-defined times */
	char kt1[K_LEN];
	char kt2[K_LEN];
	char kt3[K_LEN];
	char kt4[K_LEN];
	char kt5[K_LEN];
	char kt6[K_LEN];
	char kt7[K_LEN];
	char kt8[K_LEN];
	char kt9[K_LEN];
	char kf[K_LEN];
	char kuser0[K_LEN];     /* labels for user-defined quantities */
	char kuser1[K_LEN];
	char kuser2[K_LEN];
	char kcmpnm[K_LEN];
	char knetwk[K_LEN];
	char kdatrd[K_LEN];
	char kinst[K_LEN];
};

/*
 * This is a structure of the same size as 'SAChead', but in a form
 * that is easier to initialize.
 */

struct SAChead2 {
	float SACfloat[NUM_FLOAT];
	int32_t SACint[MAXINT];
	char SACstring[MAXSTRING][K_LEN];
};

/* eof sachead.h */
