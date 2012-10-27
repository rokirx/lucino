#include "token.h"

BEGIN_C_DECLS

void
lcn_token_set_position_increment( lcn_token_t* token,
				  unsigned int position_increment )
{
    token->position_increment = position_increment;
}


unsigned int
lcn_token_get_position_increment( lcn_token_t* token )
{
    return token->position_increment;
}

apr_status_t
lcn_token_term_text( lcn_token_t* token, 
		     char** term_text, 
		     apr_pool_t* pool )
{
    if( NULL == ( *term_text = apr_pstrdup( pool, token->term_text ) ) )
    {
	return APR_ENOMEM;
    }
	  
    return APR_SUCCESS;
}

unsigned int
lcn_token_start_offset( lcn_token_t* token )
{
    return token->start_offset;
}

unsigned int
lcn_token_end_offset( lcn_token_t* token )
{
    return token->end_offset;
}

unsigned int
lcn_token_length( lcn_token_t* token )
{
    return ( token->end_offset - token->start_offset );
}

lcn_token_type_t
lcn_token_type( lcn_token_t* token )
{
    return token->type;
}

void
lcn_token_init( lcn_token_t* token, 
		const char* text, 
		unsigned int start, 
		unsigned int end, 
		lcn_token_type_t type )
{
    token->start_offset       = start;
    token->end_offset         = end;
    token->type               = type;
    token->term_text          = text;
    token->position_increment = 1;
}

END_C_DECLS
