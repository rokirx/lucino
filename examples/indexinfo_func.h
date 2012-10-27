#ifndef INDEXINFO_FUNC_H
#define INDEXINFO_FUNC_H

#include "lucene.h"
#include "lcn_index.h"


/**shows count of all docs within the index_reader
 *
 *@param index_reader
 *@return s
 */

apr_status_t
lcn_index_reader_show_doc_count( lcn_index_reader_t* index_reader );


/**
 *shows all fields (with its settings) of the index_reader
 *
 *@param index_reader
 *@param pool
 *@return s
 */

apr_status_t
lcn_index_reader_show_fields_overview( lcn_index_reader_t* index_reader, 
		      apr_pool_t* pool );


/**shows <number> of <term-text> from <field>
 *
 * "with it's <freq>" NOT STABLE JET shows -1 for now (at line 181)
 *
 *@param index_reader
 *@field_name
 *@term_text
 *@number
 *@param pool @return s
 */
apr_status_t
lcn_index_reader_show_field_overview( lcn_index_reader_t* index_reader, 
                                      const char* field_name, 
                                      const char* term_text, 
                                      const int  number,
                                      unsigned int term_freq_lo,
                                      unsigned int term_freq_hi,
                                      apr_pool_t* pool );

apr_status_t
lcn_do_query_search( lcn_index_reader_t *index_reader,
                     const char *query_string,
                     const char *output_fields,
                     unsigned int limit,
                     apr_pool_t *pool );


#endif /* INDEXINFO_FUNC_H */

