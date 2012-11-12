#ifndef TOKEN_H
#define TOKEN_H

#include "lcn_analysis.h"


void
lcn_token_init( lcn_token_t* token, 
		const char* text, 
		unsigned int start, 
		unsigned int end, 
		lcn_token_type_t type );

#endif /* TOKEN_H */
