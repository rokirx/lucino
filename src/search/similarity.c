#include "similarity.h"
#include "lucene.h"
#include <math.h>

BEGIN_C_DECLS

static float
lcn_default_similarity_norm_table[] =
{
    0.0,
    5.820766E-10,
    6.9849193E-10,
    8.1490725E-10,
    9.313226E-10,
    1.1641532E-9,
    1.3969839E-9,
    1.6298145E-9,
    1.8626451E-9,
    2.3283064E-9,
    2.7939677E-9,
    3.259629E-9,
    3.7252903E-9,
    4.656613E-9,
    5.5879354E-9,
    6.519258E-9,
    7.4505806E-9,
    9.313226E-9,
    1.1175871E-8,
    1.3038516E-8,
    1.4901161E-8,
    1.8626451E-8,
    2.2351742E-8,
    2.6077032E-8,
    2.9802322E-8,
    3.7252903E-8,
    4.4703484E-8,
    5.2154064E-8,
    5.9604645E-8,
    7.4505806E-8,
    8.940697E-8,
    1.0430813E-7,
    1.1920929E-7,
    1.4901161E-7,
    1.7881393E-7,
    2.0861626E-7,
    2.3841858E-7,
    2.9802322E-7,
    3.5762787E-7,
    4.172325E-7,
    4.7683716E-7,
    5.9604645E-7,
    7.1525574E-7,
    8.34465E-7,
    9.536743E-7,
    1.1920929E-6,
    1.4305115E-6,
    1.66893E-6,
    1.9073486E-6,
    2.3841858E-6,
    2.861023E-6,
    3.33786E-6,
    3.8146973E-6,
    4.7683716E-6,
    5.722046E-6,
    6.67572E-6,
    7.6293945E-6,
    9.536743E-6,
    1.1444092E-5,
    1.335144E-5,
    1.5258789E-5,
    1.9073486E-5,
    2.2888184E-5,
    2.670288E-5,
    3.0517578E-5,
    3.8146973E-5,
    4.5776367E-5,
    5.340576E-5,
    6.1035156E-5,
    7.6293945E-5,
    9.1552734E-5,
    1.0681152E-4,
    1.2207031E-4,
    1.5258789E-4,
    1.8310547E-4,
    2.1362305E-4,
    2.4414062E-4,
    3.0517578E-4,
    3.6621094E-4,
    4.272461E-4,
    4.8828125E-4,
    6.1035156E-4,
    7.324219E-4,
    8.544922E-4,
    9.765625E-4,
    0.0012207031,
    0.0014648438,
    0.0017089844,
    0.001953125,
    0.0024414062,
    0.0029296875,
    0.0034179688,
    0.00390625,
    0.0048828125,
    0.005859375,
    0.0068359375,
    0.0078125,
    0.009765625,
    0.01171875,
    0.013671875,
    0.015625,
    0.01953125,
    0.0234375,
    0.02734375,
    0.03125,
    0.0390625,
    0.046875,
    0.0546875,
    0.0625,
    0.078125,
    0.09375,
    0.109375,
    0.125,
    0.15625,
    0.1875,
    0.21875,
    0.25,
    0.3125,
    0.375,
    0.4375,
    0.5,
    0.625,
    0.75,
    0.875,
    1.0,
    1.25,
    1.5,
    1.75,
    2.0,
    2.5,
    3.0,
    3.5,
    4.0,
    5.0,
    6.0,
    7.0,
    8.0,
    10.0,
    12.0,
    14.0,
    16.0,
    20.0,
    24.0,
    28.0,
    32.0,
    40.0,
    48.0,
    56.0,
    64.0,
    80.0,
    96.0,
    112.0,
    128.0,
    160.0,
    192.0,
    224.0,
    256.0,
    320.0,
    384.0,
    448.0,
    512.0,
    640.0,
    768.0,
    896.0,
    1024.0,
    1280.0,
    1536.0,
    1792.0,
    2048.0,
    2560.0,
    3072.0,
    3584.0,
    4096.0,
    5120.0,
    6144.0,
    7168.0,
    8192.0,
    10240.0,
    12288.0,
    14336.0,
    16384.0,
    20480.0,
    24576.0,
    28672.0,
    32768.0,
    40960.0,
    49152.0,
    57344.0,
    65536.0,
    81920.0,
    98304.0,
    114688.0,
    131072.0,
    163840.0,
    196608.0,
    229376.0,
    262144.0,
    327680.0,
    393216.0,
    458752.0,
    524288.0,
    655360.0,
    786432.0,
    917504.0,
    1048576.0,
    1310720.0,
    1572864.0,
    1835008.0,
    2097152.0,
    2621440.0,
    3145728.0,
    3670016.0,
    4194304.0,
    5242880.0,
    6291456.0,
    7340032.0,
    8388608.0,
    1.048576E7,
    1.2582912E7,
    1.4680064E7,
    1.6777216E7,
    2.097152E7,
    2.5165824E7,
    2.9360128E7,
    3.3554432E7,
    4.194304E7,
    5.0331648E7,
    5.8720256E7,
    6.7108864E7,
    8.388608E7,
    1.00663296E8,
    1.17440512E8,
    1.34217728E8,
    1.6777216E8,
    2.01326592E8,
    2.34881024E8,
    2.68435456E8,
    3.3554432E8,
    4.02653184E8,
    4.69762048E8,
    5.3687091E8,
    6.7108864E8,
    8.0530637E8,
    9.395241E8,
    1.07374182E9,
    1.34217728E9,
    1.61061274E9,
    1.87904819E9,
    2.14748365E9,
    2.68435456E9,
    3.22122547E9,
    3.75809638E9,
    4.2949673E9,
    5.3687091E9,
    6.4424509E9,
    7.5161928E9,
};

/**
 * Decodes a normalization factor stored in an index.
 */
static float
lcn_default_similarity_decode_norm( lcn_byte_t b)
{
    return lcn_default_similarity_norm_table[ b & 0xFF ];
}


static lcn_byte_t
lcn_default_similarity_encode_norm(float f)
{
    return lcn_smallfloat_float_to_byte315( f );
}

static float
lcn_default_similarity_idf( unsigned int doc_freq, unsigned int num_docs )
{
    return ((float)log( num_docs / (float)( doc_freq + 1 ) )) + 1.0f;
}

static float
lcn_default_similarity_tf( float freq )
{
    return (float)sqrt( freq );
}

static float
lcn_default_similarity_sloppy_freq( unsigned int distance )
{
    return ( 1.0f / ( (float) ( distance + 1 ) ) );
}

static float
lcn_default_similarity_coord( unsigned int overlap, unsigned int max_overlap )
{
    return (float) ( overlap / (float) max_overlap );
}

static float
lcn_default_similarity_length_norm( const char* field, unsigned int num_tokens )
{
    return (float)( 1.0f / sqrt( num_tokens ) );
}

static float
lcn_default_similarity_query_norm( float sum_of_squared_weights )
{
    float result;

    result = (float)( 1.0f / sqrt( sum_of_squared_weights ) );

    return result;
}

float
lcn_similarity_decode_norm( const lcn_similarity_t* similarity, lcn_byte_t b )
{
    return similarity->decode_norm( b );
}


lcn_byte_t
lcn_similarity_encode_norm( const lcn_similarity_t* similarity, float f)
{
    return similarity->encode_norm( f );
}

float
lcn_similarity_length_norm( const lcn_similarity_t* similarity,
                            const char* field_name,
                            unsigned int num_tokens )
{
    return similarity->length_norm( field_name, num_tokens );
}

float
lcn_similarity_query_norm( const lcn_similarity_t* similarity,
                           float sum_of_squared_weight )
{
    float result = similarity->query_norm( sum_of_squared_weight );

    return result;
}

float
lcn_similarity_sloppy_freq( const lcn_similarity_t* similarity,
                            unsigned int distance )
{
    float result =  similarity->sloppy_freq( distance );

    return result;
}

float
lcn_similarity_idf( const lcn_similarity_t* similarity,
                    unsigned int doc_freq,
                    unsigned int num_docs )
{

    return similarity->idf( doc_freq, num_docs );
}

float
lcn_similarity_coord( const lcn_similarity_t* similarity,
                      unsigned int overlap,
                      unsigned int max_overlap )
{
    return similarity->coord( overlap, max_overlap );
}

float
lcn_similarity_tf( const lcn_similarity_t* similarity, float freq )
{
    return similarity->tf( freq );
}

apr_status_t
lcn_default_similarity_create( lcn_similarity_t** similarity,
                               apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *similarity = apr_pcalloc( pool,
                                          sizeof( lcn_similarity_t ) ),
               APR_ENOMEM );

        (*similarity)->encode_norm  = lcn_default_similarity_encode_norm;
        (*similarity)->decode_norm  = lcn_default_similarity_decode_norm;
        (*similarity)->idf          = lcn_default_similarity_idf;
        (*similarity)->tf           = lcn_default_similarity_tf;
        (*similarity)->sloppy_freq  = lcn_default_similarity_sloppy_freq;
        (*similarity)->coord        = lcn_default_similarity_coord;
        (*similarity)->length_norm  = lcn_default_similarity_length_norm;
        (*similarity)->query_norm   = lcn_default_similarity_query_norm;
        (*similarity)->norm_decoder = (float*)lcn_default_similarity_norm_table;
    }
    while( FALSE );

    return s;
}


float*
lcn_similarity_norm_decoder( const lcn_similarity_t* similarity )
{
    return similarity->norm_decoder;
}

apr_status_t
lcn_similarity_clone( const lcn_similarity_t* similarity,
                      lcn_similarity_t** new_similarity,
                      apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *new_similarity = apr_pcalloc( pool,
                                              sizeof( lcn_similarity_t ) ),
               APR_ENOMEM );
        memcpy( *new_similarity, similarity, sizeof( lcn_similarity_t ) );
    }
    while( FALSE );

    return s;
}

END_C_DECLS
