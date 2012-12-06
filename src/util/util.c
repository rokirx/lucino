#include "lucene.h"
#include "lcn_util.h"

/**
 * An array to map all upper-case characters into their corresponding
 * lower-case character.
 */

unsigned char lcn_upper_to_lower_map[] = {
      0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
     10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
     20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
     30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
     40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
     50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
     60, 61, 62, 63, 64, 97, 98, 99,100,101,
    102,103,104,105,106,107,108,109,110,111, /*  70 */
    112,113,114,115,116,117,118,119,120,121, /*  80 */
    122, 91, 92, 93, 94, 95, 96, 97, 98, 99, /*  90 */
    100,101,102,103,104,105,106,107,108,109, /* 100 */
    110,111,112,113,114,115,116,117,118,119, /* 110 */
    120,121,122,123,124,125,126,127,128,129, /* 120 */
    130,131,132,133,134,135,136,137,138,139, /* 130 */
    140,141,142,143,144,145,146,147,148,149, /* 140 */
    150,151,152,153,154,155,156,157,158,159, /* 150 */
    160,161,162,163,164,165,166,167,168,169, /* 160 */
    170,171,172,173,174,175,176,177,178,179, /* 170 */
    180,181,182,183,184,185,186,187,188,189, /* 180 */
    190,191,224,225,226,227,228,229,230,231, /* 190 */
    232,233,234,235,236,237,238,239,240,241, /* 200 */
    242,243,244,245,246,215,248,249,250,251, /* 210 */
    252,253,254,223,224,225,226,227,228,229, /* 220 */
    230,231,232,233,234,235,236,237,238,239, /* 230 */
    240,241,242,243,244,245,246,247,248,249, /* 240 */
    250,251,252,253,254,255
};

char *
lcn_string_to_lower( char *s )
{
    char *ls = s;

    while( *ls )
    {
        *ls = (char) lcn_upper_to_lower_map[ (unsigned char) *ls ];
        ls++;
    }

    return s;
}


static char digits[] = {
    '0' , '1' , '2' , '3' , '4' , '5' , '6' , '7' ,
    '8' , '9' , 'a' , 'b' , 'c' , 'd' , 'e' , 'f' ,
    'g' , 'h' , 'i' , 'j' , 'k' , 'l' , 'm' , 'n' ,
    'o' , 'p' , 'q' , 'r' , 's' , 't' , 'u' , 'v' ,
    'w' , 'x' , 'y' , 'z',
};

#define DIGIT_1 (0x0000001f)
#define DIGIT_2 (0x000003e0)
#define DIGIT_3 (0x00007c00)
#define DIGIT_4 (0x000f8000)
#define DIGIT_5 (0x01f00000)
#define DIGIT_6 (0x3e000000)
#define DIGIT_7 (0xc0000000)

LUCENE_EXTERN void
lcn_itoa32 ( unsigned int i, char *buf )
{
    int index;
    int j;
    int pos = 0;

    /* calculate the length of the number representation */
    if      (( index = ( i & DIGIT_7 ) >> 30 )) { j = 7; }
    else if (( index = ( i & DIGIT_6 ) >> 25 )) { j = 6; }
    else if (( index = ( i & DIGIT_5 ) >> 20 )) { j = 5; }
    else if (( index = ( i & DIGIT_4 ) >> 15 )) { j = 4; }
    else if (( index = ( i & DIGIT_3 ) >> 10 )) { j = 3; }
    else if (( index = ( i & DIGIT_2 ) >>  5 )) { j = 2; }
    else
    {
        index =   i & DIGIT_1;
        j = 1;
    }

    /* set the leftmost digit */
    buf[ pos++ ] = digits[ index ];

    /* set remaining digits */
    switch (j)
    {
    case 7: buf[ pos++ ] = digits[ (i & DIGIT_6) >> 25 ];
    case 6: buf[ pos++ ] = digits[ (i & DIGIT_5) >> 20 ];
    case 5: buf[ pos++ ] = digits[ (i & DIGIT_4) >> 15 ];
    case 4: buf[ pos++ ] = digits[ (i & DIGIT_3) >> 10 ];
    case 3: buf[ pos++ ] = digits[ (i & DIGIT_2) >>  5 ];
    case 2: buf[ pos++ ] = digits[ (i & DIGIT_1)       ];
    case 1: buf[ pos   ] = 0; /* null terminated string */
    }
}

void
lcn_itoa36( apr_int64_t i, char *buf )
{
    char b[10];
    int index = 0;
    int j = 0;
    unsigned int r = i;

    if ( 0 == i )
    {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    while( r > 0 )
    {
        b[ index++ ] = digits[ r % 36 ];
        r /= 36;
    }

    buf[ index ] = '\0';

    while( --index >= 0 )
    {
        buf[ index ] = b[ j++ ];
    }
}

static int char2int( char c )
{
    switch(c)
    {
    case '0': return  0;
    case '1': return  1;
    case '2': return  2;
    case '3': return  3;
    case '4': return  4;
    case '5': return  5;
    case '6': return  6;
    case '7': return  7;
    case '8': return  8;
    case '9': return  9;
    case 'a': return 10;
    case 'b': return 11;
    case 'c': return 12;
    case 'd': return 13;
    case 'e': return 14;
    case 'f': return 15;
    case 'g': return 16;
    case 'h': return 17;
    case 'i': return 18;
    case 'j': return 19;
    case 'k': return 20;
    case 'l': return 21;
    case 'm': return 22;
    case 'n': return 23;
    case 'o': return 24;
    case 'p': return 25;
    case 'q': return 26;
    case 'r': return 27;
    case 's': return 28;
    case 't': return 29;
    case 'u': return 30;
    case 'v': return 31;
    case 'w': return 32;
    case 'x': return 33;
    case 'y': return 34;
    case 'z': return 35;
    }
    return 0;
}

LUCENE_EXTERN unsigned int
lcn_atoi32 ( const char *buf )
{
    unsigned int result = 0;
    if ( buf[0] != 0 ){ result |= char2int(buf[0]); } else { return 0; }

    if ( buf[1] != 0 ){ result = (result << 5) | char2int(buf[1]); } else { return result; }
    if ( buf[2] != 0 ){ result = (result << 5) | char2int(buf[2]); } else { return result; }
    if ( buf[3] != 0 ){ result = (result << 5) | char2int(buf[3]); } else { return result; }
    if ( buf[4] != 0 ){ result = (result << 5) | char2int(buf[4]); } else { return result; }
    if ( buf[5] != 0 ){ result = (result << 5) | char2int(buf[5]); } else { return result; }
    if ( buf[6] != 0 ){ result = (result << 5) | char2int(buf[6]); }

    return result;
}

LUCENE_EXTERN unsigned int
lcn_atoi36 ( const char *buf )
{
    unsigned int result = 0;

    if ( buf[0] != 0 ){ result = char2int(buf[0]); } else { return 0; }

    if ( buf[1] != 0 ){ result = result * 36 + char2int(buf[1]); } else { return result; }
    if ( buf[2] != 0 ){ result = result * 36 + char2int(buf[2]); } else { return result; }
    if ( buf[3] != 0 ){ result = result * 36 + char2int(buf[3]); } else { return result; }
    if ( buf[4] != 0 ){ result = result * 36 + char2int(buf[4]); } else { return result; }
    if ( buf[5] != 0 ){ result = result * 36 + char2int(buf[5]); } else { return result; }
    if ( buf[6] != 0 ){ result = result * 36 + char2int(buf[6]); }

    return result;
}

int
lcn_string_difference ( const char *s1, const char *s2 )
{
    int i = 0;

    while( *s1 && *s2 && *s1++ == *s2++ )
    {
        i++;
    }

    return i;
}

lcn_bool_t
lcn_string_starts_with( const char* haystack, const char* needle )
{
    while( *needle && *haystack )
    {
        if( *needle != *haystack )
        {
            return LCN_FALSE;
        }

        needle++; haystack++;

        if( '\0' == *needle )
        {
            return LCN_TRUE;
        }
    }
    return LCN_FALSE;
}

apr_status_t
lcn_string_escape( char** result,
                   const char* input,
                   const char* chars_to_escape,
                   char escape_char,
                   apr_pool_t* pool )
{
    apr_status_t s;
    apr_pool_t* cp = NULL;
    char* estr;

    do
    {
        char c;
        lcn_string_buffer_t* sb;

        LCNCE( apr_pool_create( &cp, pool ) );
        LCNCE( lcn_string_buffer_create( &sb, cp ) );

        while( ( c = *(input++) ) )
        {
            const char* found;

            if( ( c == escape_char ) )
            {
                LCNCE( lcn_string_buffer_append_format( sb, "%c%c",
                                                        escape_char,
                                                        escape_char ) );
                continue;
            }
            if( NULL != ( found = strchr( chars_to_escape, c ) ) )
            {
                char rep_char;
                if( *found == '\n' ) {
                    rep_char = 'n';
                } else if( *found == '\r' ) {
                    rep_char = '\r';
                } else {
                    rep_char = *found;
                }
                LCNCE( lcn_string_buffer_append_format( sb, "%c%c",
                                                        escape_char,
                                                        rep_char ) );
                continue;
            }
            LCNCE( lcn_string_buffer_append_format( sb, "%c", c ) );
        }
        LCNCE( lcn_string_buffer_to_string( sb, &estr, pool ) );

        *result = estr;
    }
    while( 0 );

    if( NULL != cp )
    {
        apr_pool_destroy( cp );
    }
    return s;
}

apr_status_t
lcn_string_unescape( char** result,
                     const char* input,
                     const char* chars_to_escape,
                     char escape_char,
                     apr_pool_t* pool )
{
    apr_status_t s;
    char c;
    char* ustr;

    lcn_bool_t last_was_escape = LCN_FALSE;

    *result = NULL;

    LCNPR( ustr = apr_palloc( pool, strlen( input ) * sizeof(char) + 1 ),
           APR_ENOMEM );

    *result = ustr;

    while( ( c = *(input++) ) )
    {
        if( c != escape_char  )
        {
            last_was_escape = LCN_FALSE;
            *(ustr++) = c;
            continue;
        }

        if( last_was_escape )
        {
            last_was_escape = LCN_FALSE;
            *(ustr++) = c;
            continue;
        }

        last_was_escape = LCN_TRUE;
    }

    *ustr = '\0';

    return s;

}

const char*
lcn_string_next_unescaped( const char* input, char c, char escape_char )
{
    int counter = 1;
    char last;

    if( *input == c )
    {
        return input;
    }

    while( ( last = *(input++) ) && *input )
    {
        if( *input == c )
        {
            /* Check, if escape-char was escaped */

            if( last == escape_char )
            {
                if( ( counter > 1 ) && *( input - 2 ) == escape_char   )
                {
                    return input;
                }
            }
            else
            {
                return input;
            }
        }
        counter++;
    }

    return NULL;
}

char*
lcn_string_purge_whitespaces( char* str )
{
    char c, *result, *loop;
    loop = result = str;

    while( ( c = *(str++) ) )
    {
        if( LCN_IS_WHITESPACE( c ) )
        {
            continue;
        }
        *(loop++) = c;
    }
    *loop = 0;
    return result;
}



apr_status_t
lcn_string_from_file( char** result,
                      const char* path,
                      unsigned int* length,
                      apr_pool_t* pool )
{
    apr_status_t s;
    apr_file_t *file = NULL;
    apr_size_t bytes_read;
    apr_finfo_t finfo;
    char *new_buffer = NULL;

    do
    {
        *result = NULL;
        LCNCM( apr_file_open( &file,
                              path,
                              APR_READ,
                              APR_OS_DEFAULT,
                              pool ), path );
        LCNCM( apr_file_info_get( &finfo, APR_FINFO_SIZE, file ), path );

        new_buffer = (char* ) apr_palloc(
            pool,
            (unsigned int)( finfo.size + 1 ) * sizeof( char )
            );

        LCNPV( new_buffer, APR_ENOMEM );
        LCNCE( apr_file_read_full( file,
                                   new_buffer,
                                   (unsigned int)finfo.size, &bytes_read ) );
    }
    while(0);

    if ( NULL != file )
    {
        apr_status_t stat = apr_file_close( file );
        s = ( s ? s : stat );
    }

    if ( APR_SUCCESS == s )
    {
        new_buffer[ bytes_read ] = '\0';
        *length = (unsigned int) finfo.size;
        *result = new_buffer;
    }

    return s;
}

lcn_bool_t
lcn_string_is_digit( char *str )
{
    int len;
    int i = 0;

    if( !str )
    {
        return 0;
    }

    len = strlen(str);
    while( i < len )
    {
        if( !isdigit(str[i]) )
            return 0;
        i++;
    }

    return 1;
}
