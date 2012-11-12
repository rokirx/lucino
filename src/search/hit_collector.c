#include "hit_collector.h"

void
lcn_hit_collector_set_filter_bits( lcn_hit_collector_t* hit_collector,
                                   lcn_bitvector_t* bitvector )
{
    hit_collector->bitvector = bitvector;
}


apr_status_t
lcn_hit_collector_collect( lcn_hit_collector_t* hit_collector,
                           unsigned int doc,
                           lcn_score_t score )
{
    return hit_collector->collect( hit_collector, doc, score );
}

unsigned int
lcn_hit_collector_total_hits( lcn_hit_collector_t *hit_collector )
{
    return hit_collector->total_hits;
}
