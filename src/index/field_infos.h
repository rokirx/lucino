#ifndef FIELD_INFOS_H
#define FIELD_INFOS_H


#define LCN_FIELD_INFO_IS_INDEXED                       ( 0x1)
#define LCN_FIELD_INFO_STORE_TERM_VECTOR                ( 0x2)
#define LCN_FIELD_INFO_STORE_POSITION_WITH_TERM_VECTOR  ( 0x4)
#define LCN_FIELD_INFO_STORE_OFFSET_WITH_TERM_VECTOR    ( 0x8)
#define LCN_FIELD_INFO_OMIT_NORMS                       (0x10)

/* these are C-specific extensions */

//#define LCN_FIELD_INFO_HAS_SEPARATE_STORAGE             (0x20)
#define LCN_FIELD_INFO_FIXED_SIZE                       (0x40)


typedef struct lcn_field_infos_t lcn_field_infos_t;

struct lcn_field_info_t {

    const char *name;
    unsigned int field_bits;
    unsigned char bits;
    unsigned int number;

    lcn_field_info_t *next;
};

struct lcn_field_infos_t {

    apr_pool_t *pool;

    lcn_field_info_t *first_info;
    lcn_field_info_t *last_info;

    int format;

    unsigned int size;
};


unsigned int
lcn_field_infos_size( lcn_field_infos_t *field_infos );

apr_status_t
lcn_field_infos_field_number( lcn_field_infos_t *field_infos,
                              unsigned int *field_number,
                              const char *field_name );

apr_status_t
lcn_field_infos_by_number( lcn_field_infos_t *field_infos,
                           lcn_field_info_t **field_info,
                           unsigned int field_number );

apr_status_t
lcn_field_infos_nth_info( lcn_field_infos_t *field_infos,
                          lcn_field_info_t **field_info,
                          unsigned int n );

apr_status_t
lcn_field_infos_by_name( lcn_field_infos_t *field_infos,
                         lcn_field_info_t **field_info,
                         const char *field_name );

apr_status_t
lcn_field_infos_name_by_number( lcn_field_infos_t *field_infos,
                                char **field_name,
                                unsigned int field_number );


apr_status_t
lcn_field_infos_create( lcn_field_infos_t **field_infos,
                        apr_pool_t *pool );

apr_status_t
lcn_field_infos_create_from_dir( lcn_field_infos_t **field_infos,
                                 lcn_directory_t *directory,
                                 const char *segment,
                                 apr_pool_t *pool );

apr_status_t
lcn_field_infos_add_document( lcn_field_infos_t *field_infos,
                              lcn_document_t *doc );

apr_status_t
lcn_field_infos_add_field_info( lcn_field_infos_t *field_infos,
                                const char *name,
                                unsigned char bits );

apr_status_t
lcn_field_infos_write_to_ostream( lcn_field_infos_t *field_infos,
                                  lcn_ostream_t *ostream );

apr_status_t
lcn_field_infos_read( lcn_field_infos_t *field_infos,
                      lcn_index_input_t *in,
                      apr_pool_t *pool );

apr_status_t
lcn_field_infos_read_from_input_stream( lcn_field_infos_t **field_infos,
                                        lcn_index_input_t *input_stream );

lcn_bool_t
lcn_field_info_omit_norms( lcn_field_info_t *field_info );

apr_status_t
lcn_field_infos_write( lcn_field_infos_t *field_infos,
                       lcn_directory_t *directory,
                       const char *seg_name );


#endif /* FIELD_INFOS_H */
