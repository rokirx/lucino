#include "lcn_index.h"
#include "field.h"


#define LCN_FIELD_TYPE_SETTER( MASK, NAME )             \
apr_status_t                                            \
lcn_field_type_set_##NAME( lcn_field_type_t *ft,        \
                           lcn_bool_t val )             \
{                                                       \
    unsigned int bm = (unsigned int) *ft;               \
                                                        \
    if ( bm & LCN_FIELD_TYPE_FROSEN )                   \
    {                                                   \
        return LCN_ERR_SETTING_FROZEN_FIELD_TYPE;       \
    }                                                   \
                                                        \
    if ( val )                                          \
    {                                                   \
        bm |= MASK;                                     \
    }                                                   \
    else                                                \
    {                                                   \
        bm &= ( !MASK );                                \
    }                                                   \
                                                        \
    *ft = (lcn_field_type_t) bm;                        \
                                                        \
    return APR_SUCCESS;                                 \
}                                                       \
                                                        \
lcn_bool_t                                              \
lcn_field_type_is_##NAME( lcn_field_type_t ft )         \
{                                                       \
    int bm = (int) ft;                                  \
    return 0 != ( bm & MASK );                          \
}


/**
 * Describes the properties of a field.
 */
#if 0
    /**  #### NOT IMPLEMENTED : NumericType ####
     * Data type of the numeric value
     * @since 3.2
     */
    // public static enum NumericType {INT, LONG, FLOAT, DOUBLE}
#endif


#if 0
    /* #### NOT IMPLEMENTED : more FieldTypes #### */
    private IndexOptions indexOptions = IndexOptions.DOCS_AND_FREQS_AND_POSITIONS;
    private DocValues.Type docValueType;
    private NumericType numericType;
    private int numericPrecisionStep = NumericUtils.PRECISION_STEP_DEFAULT;
#endif

#if 0
    /* #### NOT IMPLEMENTED : FieldType(FieldType ref)  */

/*
  public FieldType(FieldType ref) {
    this.indexed = ref.indexed();
    this.stored = ref.stored();
    this.tokenized = ref.tokenized();
    this.storeTermVectors = ref.storeTermVectors();
    this.storeTermVectorOffsets = ref.storeTermVectorOffsets();
    this.storeTermVectorPositions = ref.storeTermVectorPositions();
    this.omitNorms = ref.omitNorms();
    this.indexOptions = ref.indexOptions();
    this.docValueType = ref.docValueType();
    this.numericType = ref.numericType();
    // Do not copy frozen!
  }
*/
#endif


LCN_FIELD_TYPE_SETTER( LCN_FIELD_TYPE_BINARY,                      binary )
LCN_FIELD_TYPE_SETTER( LCN_FIELD_TYPE_COPY_VALUE,                  copy_value )
LCN_FIELD_TYPE_SETTER( LCN_FIELD_TYPE_FIXED_SIZE,                  fixed_size )
LCN_FIELD_TYPE_SETTER( LCN_FIELD_TYPE_INDEXED,                     indexed )
LCN_FIELD_TYPE_SETTER( LCN_FIELD_TYPE_OMIT_NORMS,                  omit_norms )
LCN_FIELD_TYPE_SETTER( LCN_FIELD_TYPE_STORED,                      stored )
LCN_FIELD_TYPE_SETTER( LCN_FIELD_TYPE_STORE_TERM_VECTORS,          store_term_vectors )
LCN_FIELD_TYPE_SETTER( LCN_FIELD_TYPE_STORE_TERM_VECTOR_OFFSETS,   store_term_vector_offsets )
LCN_FIELD_TYPE_SETTER( LCN_FIELD_TYPE_STORE_TERM_VECTOR_POSITIONS, store_term_vector_positions )
LCN_FIELD_TYPE_SETTER( LCN_FIELD_TYPE_TOKENIZED,                   tokenized )


apr_status_t
lcn_field_type_init( lcn_field_type_t *ft )
{
    *ft = (lcn_field_type_t) 0;
    return APR_SUCCESS;
}
