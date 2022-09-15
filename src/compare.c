#include <string.h>
#include <tbconvert.h>

/*
 *
 */
int compare_SCNL( const void *a, const void *b )
{
	int ret;
	TRACE_NODE *trace_a = (TRACE_NODE *)a;
	TRACE_NODE *trace_b = (TRACE_NODE *)b;

	if ( (ret = strcmp(trace_a->sta, trace_b->sta)) )
		return ret;
	if ( (ret = strcmp(trace_a->chan, trace_b->chan)) )
		return ret;
	if ( (ret = strcmp(trace_a->net, trace_b->net)) )
		return ret;
	return strcmp(trace_a->loc, trace_b->loc);
}

/*
 *
 */
int compare_time( const void *a, const void *b )
{
	TBUF *ta = (TBUF *)a;
	TBUF *tb = (TBUF *)b;

	if ( ta->time > tb->time )
		return 1;
	else if ( ta->time < tb->time )
		return -1;
	else
		return 0;
}
