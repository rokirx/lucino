#include "lcn_util.h"

#define FLOAT_TO_INT_BITS(F) (*((int*)&F ))
#define INT_BITS_TO_FLOAT(I) (*((float*)&I))

BEGIN_C_DECLS

static int float_to_int_bits( float f )
{
    int result;
    float value = f;
    memcpy( (void*)&result, (void*)&value, sizeof( int ) );

    return result;
}

static float int_bits_to_float( int int_bits )
{
    float result;
    int value = int_bits;
    memcpy( (void*)&result, (void*)&value, sizeof( int ) );

    return result;
}

lcn_byte_t
lcn_smallfloat_float_to_byte315( float f )
{
    int smallfloat, bits;
    bits = float_to_int_bits(f);

    smallfloat = bits >> (24-3);
    if ( smallfloat < ( 63 - 15 ) << 3 )
    {
        return ( bits <= 0 ) ?
            (lcn_byte_t)0 :
            (lcn_byte_t)1;
    }
    if ( smallfloat >= ( ( 63 - 15) << 3 ) + 0x100 )
    {
        return -1;
    }
    return (lcn_byte_t)( smallfloat - ( ( 63 - 15 ) << 3 ) );
}

float
lcn_smallfloat_byte315_to_float( lcn_byte_t b )
{
    int bits;

    if ( b == 0 )
    {
        return 0.0f;
    }

    bits = ( b & 0xff ) << ( 24 - 3 );
    bits += ( 63 - 15 ) << 24;

    return int_bits_to_float( bits );
}

END_C_DECLS
