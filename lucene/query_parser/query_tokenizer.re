#include "lucene.h"
#include "lcn_search.h"
#include "query_parser.h"
#include "query_tokenizer.h"


static void
init_token ( lcn_query_token_t *token,
             int token_id,
             char* start,
             char* end,
             apr_pool_t *pool )
{
    token->token_id = token_id;
    token->text = apr_pstrndup( pool, start, end-start );
}


static int
lcn_next_token_impl ( lcn_query_tokenizer_t *query_tokenizer,
                      lcn_query_token_t *token )
{
    char *q;
    char *ctx;
    char *token_start = (char*) query_tokenizer->text;
    token->token_id = -1;

    if ( token_start == query_tokenizer->eofbuf )
    {
        return LCN_ERR_ITERATOR_NO_NEXT;
    }

#define YYCTYPE         unsigned char
#define YYCURSOR        query_tokenizer->text
#define YYLIMIT         query_tokenizer->eofbuf
#define YYMARKER        q
#define YYCTXMARKER     ctx
#define YYFILL(n)


#define INIT_TOKEN(X) { init_token( token, X, token_start, query_tokenizer->text, query_tokenizer->pool );return APR_SUCCESS; }

/*!re2c

  fieldname = [_a-z0-9];
  termtext  = [a-zA-ZÄÖÜöäüß0-9];
  digit  = [0-9];
  sep    = [:];
  any    = [\000-\377];

  fieldname+ /[:]  { INIT_TOKEN( LCNQ_FIELD_PREFIX );}
  sep              { INIT_TOKEN( LCNQ_SEP);          }
  termtext+        { INIT_TOKEN( LCNQ_TERM_TEXT );   }
  [ ]+             { INIT_TOKEN( LCNQ_SPACE );       }
  [-+]             { INIT_TOKEN( LCNQ_CLAUSE );      }
  [*]              { INIT_TOKEN( LCNQ_ASTERISK );    }
  [""]             { INIT_TOKEN( LCNQ_QUOT_MARK );   }
  [(]              { INIT_TOKEN( LCNQ_OPEN_BR );     }
  [)]              { INIT_TOKEN( LCNQ_CLOSE_BR );    }
  any              { INIT_TOKEN( LCNQ_EOS );         }


*/

    return LCN_ERR_ITERATOR_NO_NEXT;
}



apr_status_t
lcn_query_tokenizer_next_token ( lcn_query_tokenizer_t *tokenizer,
                                 int *token_id,
                                 lcn_query_token_t *token )
{
    apr_status_t s = lcn_next_token_impl( tokenizer, token );
    *token_id = token->token_id;
    return s;
}


apr_status_t
lcn_query_tokenizer_create( lcn_query_tokenizer_t **tokenizer,
                            const char *text,
                            apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *tokenizer = (lcn_query_tokenizer_t*) apr_pcalloc( pool, sizeof(lcn_query_tokenizer_t)),
               APR_ENOMEM );

        LCNPV( (*tokenizer)->text = apr_pstrdup( pool, text ),
               APR_ENOMEM );

        (*tokenizer)->pool = pool;
        (*tokenizer)->eofbuf = (*tokenizer)->text + strlen(text);

    }
    while(0);

    return s;
}

