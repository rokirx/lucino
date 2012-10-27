#include "analysis/stemmer.h"

BEGIN_C_DECLS

void
lcn_german_stemmer_replace_ending ( lcn_stemmer_t *stemmer, char *term )
{
    int l = strlen(term);
    char *end = term + l;
    char *oepos;

    if ( l > 6 )
    {
        char *c = strstr( term, "sss" );

        if ( NULL != c )
        {
            c += 3;

            while( (*(c-1) = *c) )
            {
                c++;
            }

            l--;
            end = term+l;
        }
    }

    if ( l > 5 )
    {
        char *c = strstr( term, "ßs" );

        if ( NULL != c )
        {
            c += 2;

            while( (*(c-1) = *c) )
            {
                c++;
            }

            l--;
            end = term+l;
        }
    }

    if ( l > 6 )
    {
        if ( *(end-1) == 'n' && *(end-2) == 'i' )
        {
            *(end-1) = 'r';
            *(end-2) = 'e';
        }
    }

    if ( l > 8 )
    {
        if ( *(end-1) == 'n' && *(end-2) == 'e' && *(end-3) == 'n' && *(end-4) == 'n' && *(end-5) == 'i' )
        {
            *(end-5) = 'e';
            *(end-4) = 'r';
            *(end-3) = '\0';

            l-=2;
            end = term+l;
        }
    }

    if ( l > 5 && NULL != (oepos = strstr( term, "oe" )))
    {
        char *c = oepos + 2;
        *oepos = 'ö';

        while((*(c-1) = *c))
        {
            c++;
        }

        l--;
        end = term+l;
    }

    if ( l >= 5 && *(end-2) == 'u' && *(end-3) == 'i' && ( *(end-1) == 's' || *(end-1) == 'm' ) )
    {
        *(end-2) = 'e';
        *(end-1) = 'n';
        return;
    }

    if ( l >= 5 && *(end-4) == 'i' && *(end-3) == 'u' && *(end-2) == 'm' && *(end-1) == 's' )
    {
        *(end-3) = 'e';
        *(end-2) = 'n';
        *(end-1) = '\0';
        return;
    }

    if ( l >= 5 )
    {
        /*  isse issen isses  */

        if ( *(end-3) == 's' )
        {
            /* isse */
            if ( *(end-2) == 's' && *(end-4) == 'i' && *(end-1) == 'e' )
            {
                *(end-2) = '\0';
                return;
            }

            /* issen isses */
            if ( *(end-4) == 's' && *(end-5) == 'i' && *(end-2) == 'e' && ( *(end-1) == 's' || *(end-1) == 'n' ))
            {
                *(end-3) = '\0';
                return;
            }
        }
    }

    if ( l > 5 )
    {
        if ((*term == 'a' && ( *(term+1) == 'n' || *(term+1) == 'b' )) ||
            (*term == 'u' && ( *(term+1) == 'm' ))                     ||
            (*term == 'z' && ( *(term+1) == 'u' )))
        {
            if ( *(term+2) == 'g' && *(term+3) == 'e' )
            {
                char *c = term + 4;

                while((*(c-2) = *c))
                {
                    c++;
                }

                return;
            }
        }
    }

    if ( l > 6 )
    {
        if ((*term == 'a' && *(term+1) == 'u' && ( *(term+2) == 'f' || *(term+2) == 's' )) ||
            (*term == 'e' && *(term+1) == 'i' && ( *(term+2) == 'n' || *(term+2) == 's' )))
        {
            if ( *(term+3) == 'g' && *(term+4) == 'e' )
            {
                char *c = term + 5;

                while((*(c-2) = *c))
                {
                    c++;
                }

                return;
            }
        }
    }

    return;
}

/**
 * Do some substitutions for the term to reduce overstemming:
 *
 * - Substitute Umlauts with their corresponding vowel: äöü -> aou,
 *   "ß" is substituted by "ss"
 * - Substitute a second char of a pair of equal characters with
 *   an asterisk: ?? -> ?*
 * - Substitute some common character combinations with a token:
 *   sch/ch/ei/ie/ig/st -> $/§/%/&/#/!
 */
void
lcn_german_stemmer_substitute ( lcn_stemmer_t *stemmer, char *term )
{
    int subst_count = 0;
    char *t;
    char *buf = stemmer->buf;

    for ( t = term; ( *buf = *t ); t++, buf++)
    {
        if      ( *t == 'ä' ) { *buf = 'a'; }
        else if ( *t == 'ö' ) { *buf = 'o'; }
        else if ( *t == 'é' ) { *buf = 'e'; }
        else if ( *t == 'ü' ) { *buf = 'u'; }

        if ( *t == *(t+1) )
        {
            *(++buf) = '*';

            do
            {
                t++;
            }
            while( *t == *(t+1) );

            continue;
        }

        if ( *t == 's' )
        {
            if ( *(t+1) == 'c' && *(t+2) == 'h' )
            {
                *buf = '$';
                subst_count += 2;
                t+=2;
            }
            else if ( *(t+1) == 't' )
            {
                *buf = '!';
                subst_count++;
                t++;
            }
        }
        else if ( *t == 'c' && *(t+1) == 'h' )
        {
            *buf = '§';
            subst_count++;
            t++;
        }
        else if ( *t == 'e' && *(t+1) == 'i' )
        {
            *buf = '%';
            subst_count++;
            t++;
        }
        else if ( *t == 'i' )
        {
            if ( *(t+1) == 'e' )
            {
                *buf = '~';
                subst_count++;
                t++;
            }
            else if ( *(t+1) == 'g' )
            {
                *buf = '#';
                subst_count++;
                t++;
            }
        }
    }

    stemmer->subst_count = subst_count;
}


/**
 * suffix stripping (stemming) on the current term. The stripping is reduced
 * to the seven "base" suffixes "e", "s", "n", "t", "em", "er" and * "nd",
 * from which all regular suffixes are build of. The simplification causes
 * some overstemming, and way more irregular stems, but still provides unique.
 * discriminators in the most of those cases.
 * The algorithm is context free, except of the length restrictions.
 */
void
lcn_german_stemmer_strip( lcn_stemmer_t *stemmer )
{
    int do_more = 1;
    char *buf = stemmer->buf;
    int l = strlen( buf );
    int subst_count = stemmer->subst_count;

    while ( do_more && l > 3 )
    {
        char last = buf[l-1];
        if ( ( l + subst_count > 5 ) && buf[l-2] == 'n' && last == 'd' )
        {
            buf[l-2] = '\0';
            l -= 2;
        }else if ( ( l + subst_count > 4 ) && ( buf[l-2] == 'e' && ( last == 'r' || last == 'm' ) ) )
        {
            buf[l-2] = '\0';
            l -= 2;
        }else if ( last == 'e' || last == 's' || last == 'n' || last == 't' )
        {
            buf[--l] = '\0';
        }else{
            do_more = 0;
        }
    }
}

/**
 * Does some optimizations on the term. This optimisations are
 * contextual.
 */
void
lcn_german_stemmer_optimize( lcn_stemmer_t *stemmer )
{
    char *buf = stemmer->buf;
    int l = strlen( buf );

    /* Additional step for female plurals of professions and inhabitants. */
    if ( l > 5 && buf[l-1] == '*' && buf[l-2] == 'n' && buf[l-3] == 'i' && buf[l-4] == 'r' && buf[l-5] == 'e' )
    {
        buf[--l] = '\0';
        lcn_german_stemmer_strip( stemmer );
        l = strlen( buf );
    }

    /* Additional step for irregular plural nouns like "Matrizen -> Matrix". */
    if ( buf[l-1] == 'z' )
    {
        buf[l-1] = 'x';
    }

    if ( l > 3 && buf[l-1] == 'o' )
    {
        buf[l-1] = '\0';
    }
}

/**
 * Undoes the changes made by substitute(). That are character pairs and
 * character combinations. Umlauts will remain as their corresponding vowel,
 * as "ß" remains as "ss".
 */
void
lcn_german_stemmer_resubstitute( lcn_stemmer_t *stemmer, char *target  )
{
    char *buf = stemmer->buf;
    char c;

    while( ( c = *buf ) )
    {
        switch (c)
        {
        case '*' :
            if ( *(buf-1) == 's' )
            {
                *(--target) = 'ß';
            }else{
                *target = *(buf-1);
            }
            break;

        case '$' :
            *target = 's';
            target++;
            *target = 'c';
            target++;
            *target = 'h';
            break;

        case '§' :
            *target = 'c';
            target++;
            *target = 'h';
            break;

        case '%' :
            *target = 'e';
            target++;
            *target = 'i';
            break;

        case '~' :
            *target = 'i';
            target++;
            *target = 'e';
            break;

        case '#' :
            *target = 'i';
            target++;
            *target = 'g';
            break;

        case '!' :
            *target = 's';
            target++;
            *target = 't';
            break;

        default:
            *target = *buf;
        }

        target++;
        buf++;
    }

    *target = '\0';
}

/**
 * Removes a particle denotion ("ge") from a term.
 */
void
lcn_german_stemmer_remove_particle_denotion( char *buf )
{
    char *itr = buf;
    int l = strlen(buf);

    if ( l > 4 )
    {
        while( *(buf+4) )
        {
            if ( *buf == 'g' && *(buf+1) == 'e' && *(buf+2) == 'g' && *(buf+3) == 'e' )
            {
                itr = buf + 2;
                while( ( *(buf++) = *(itr++) ) );
                return;
            }

            buf++;
        }
    }
}

lcn_bool_t
lcn_german_stemmer_has_vocals( const char *term )
{
    while( *term )
    {
        switch (*term)
        {
        case 'a':
        case 'o':
        case 'u':
        case 'i':
        case 'e':
        case 'ä':
        case 'ö':
        case 'ü':
            return LCN_TRUE;
        }

        term++;
    }

    return LCN_FALSE;
}

static lcn_bool_t
lcn_german_stemmer_is_roman_number( char* str )
{
    lcn_bool_t result = LCN_TRUE;
    char allowed_chars[] = "LDMCXVIldmcxvi";
    char c;
    int i = 0;

    while( ( c = str[i++] ) )
    {
        if( !strchr( allowed_chars, c ) )
        {
            result = LCN_FALSE;
            break;
        }
    }

    return result;
}

static void
lcn_german_stemmer_stem ( lcn_stemmer_t* stemmer, char *term )
{
    if ( strlen( term ) < 3 )
    {
        return;
    }

    if( ! lcn_german_stemmer_has_vocals( term ))
    {
        return;
    }

    if( lcn_german_stemmer_is_roman_number( term ) )
    {
        return;
    }

    lcn_german_stemmer_replace_ending( stemmer, term );
    lcn_german_stemmer_substitute( stemmer, term );
    lcn_german_stemmer_strip( stemmer );
    lcn_german_stemmer_optimize( stemmer );
    lcn_german_stemmer_resubstitute( stemmer, term );
    lcn_german_stemmer_remove_particle_denotion( term );
}

apr_status_t
lcn_german_stemmer_create( lcn_stemmer_t** german_stemmer,
                           apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNPV( (*german_stemmer) = apr_pcalloc( pool, sizeof( lcn_stemmer_t ) ), APR_ENOMEM);
        LCNPV( (*german_stemmer)->buf = apr_palloc( pool, 1024 ), APR_ENOMEM );

        (*german_stemmer)->stem = lcn_german_stemmer_stem;
    }
    while( FALSE );

    return s;
}

END_C_DECLS
