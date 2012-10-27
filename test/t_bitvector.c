#include "test_all.h"
#include "lcn_bitvector.h"

/**
 * Test the default constructor on lcn_bitvector_ts of various sizes.
 */
static void
construct_size( CuTest* tc, int size, apr_pool_t* pool )
{
    apr_pool_t* child_pool;
    lcn_bitvector_t* bv;

    LCN_TEST( apr_pool_create( &child_pool, pool ) );
    LCN_TEST( lcn_bitvector_create( &bv, size, child_pool ) );
    CuAssertIntEquals( tc, size, lcn_bitvector_size( bv ) );
    apr_pool_destroy( child_pool );
}

static void test_construct_size(CuTest* tc)
{
    apr_pool_t* pool;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );

    construct_size(tc, 8, pool );
    construct_size(tc, 20, pool );
    construct_size(tc, 100, pool );
    construct_size(tc, 1000, pool );

    apr_pool_destroy( pool );
}


/**
 * Test the get and set methods on lcn_bitvector_ts of various sizes.
 */
static void
test_get_set_vectors_of_size( CuTest *tc, int n )
{
    unsigned int i;
    lcn_bitvector_t* bv;
    apr_pool_t* pool;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_bitvector_create( &bv, n, pool ) );

    for( i=0; i < lcn_bitvector_size( bv ); i++ )
    {
        CuAssertTrue( tc, ! lcn_bitvector_get_bit( bv, i ) );
        lcn_bitvector_set_bit(bv,i);
        CuAssertTrue( tc, lcn_bitvector_get_bit( bv,i ) );
    }
}

static void test_get_set( CuTest *tc )
{
    test_get_set_vectors_of_size( tc, 8 );
    test_get_set_vectors_of_size( tc, 20 );
    test_get_set_vectors_of_size( tc, 100 );
    test_get_set_vectors_of_size( tc, 1000 );
}


/**
 * Test the clear() method on lcn_bitvector_ts of various sizes.
 */
static void
test_clear_vector_of_size(CuTest *tc, int n )
{
    lcn_bitvector_t* bv;
    unsigned int i;
    apr_pool_t* pool;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );



    LCN_TEST( lcn_bitvector_create( &bv,n, pool) );

    for( i=0; i < lcn_bitvector_size(bv); i++ )
    {
        /* ensure a set bit is cleared */
        CuAssertTrue( tc, ! lcn_bitvector_get_bit( bv, i ) );
        lcn_bitvector_set_bit(bv,i);
        CuAssertTrue( tc, lcn_bitvector_get_bit( bv,i ) );
        lcn_bitvector_clear_bit(bv,i);
        CuAssertTrue( tc, ! lcn_bitvector_get_bit(bv, i ) );
    }

    apr_pool_destroy( pool);
}

static void
test_clear( CuTest *tc )
{
    test_clear_vector_of_size(tc, 8 );
    test_clear_vector_of_size(tc, 20 );
    test_clear_vector_of_size(tc, 100 );
    test_clear_vector_of_size(tc, 1000 );
}

static void
test_count_vector_of_size( CuTest *tc, int n )
{
    lcn_bitvector_t* bv;
    unsigned int i;
    apr_pool_t* pool;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );

    LCN_TEST( lcn_bitvector_create( &bv,n, pool) );

    CuAssertIntEquals( tc, 0, lcn_bitvector_count( bv ) );

    /* test count when incrementally setting bits */

    for( i=0; i < lcn_bitvector_size(bv); i++ )
    {
        CuAssertTrue(tc, ! lcn_bitvector_get_bit(bv, i ) );
        CuAssertIntEquals( tc, i, lcn_bitvector_count( bv ) );
        lcn_bitvector_set_bit(bv, i );
        CuAssertTrue( tc, lcn_bitvector_get_bit(bv, i ) );
        CuAssertIntEquals( tc, i+1, lcn_bitvector_count( bv ) );
    }


    LCN_TEST( lcn_bitvector_create( &bv, n, pool ) );

    /* test count when setting then clearing bits */

    for( i=0; i < lcn_bitvector_size(bv); i++ )
    {
        CuAssertTrue( tc, ! lcn_bitvector_get_bit(bv, i) );
        CuAssertIntEquals( tc, 0, lcn_bitvector_count(bv) );
        lcn_bitvector_set_bit(bv, i );
        CuAssertTrue( tc, lcn_bitvector_get_bit(bv, i ) );
        CuAssertIntEquals( tc, 1, lcn_bitvector_count(bv) );
        lcn_bitvector_clear_bit(bv, i );
        CuAssertTrue( tc, ! lcn_bitvector_get_bit(bv, i ) );
        CuAssertTrue( tc, ! lcn_bitvector_count(bv) );
    }
}


/**
 * Test the count() method on lcn_bitvector_ts of various sizes.
 */
static void
test_count( CuTest *tc )
{
    lcn_bitvector_t* bv;
    apr_pool_t* pool;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );

    LCN_TEST( lcn_bitvector_create( &bv, 12, pool) );

    CuAssertIntEquals( tc, 0, lcn_bitvector_count( bv ) );

    CuAssertTrue( tc, !lcn_bitvector_get_bit( bv, 0  ) );
    CuAssertTrue( tc, !lcn_bitvector_get_bit( bv, 1  ) );
    CuAssertTrue( tc, !lcn_bitvector_get_bit( bv, 2  ) );
    CuAssertTrue( tc, !lcn_bitvector_get_bit( bv, 3  ) );
    CuAssertTrue( tc, !lcn_bitvector_get_bit( bv, 4  ) );
    CuAssertTrue( tc, !lcn_bitvector_get_bit( bv, 5  ) );
    CuAssertTrue( tc, !lcn_bitvector_get_bit( bv, 6  ) );
    CuAssertTrue( tc, !lcn_bitvector_get_bit( bv, 7  ) );
    CuAssertTrue( tc, !lcn_bitvector_get_bit( bv, 8  ) );
    CuAssertTrue( tc, !lcn_bitvector_get_bit( bv, 9  ) );
    CuAssertTrue( tc, !lcn_bitvector_get_bit( bv, 10  ) );
    CuAssertTrue( tc, !lcn_bitvector_get_bit( bv, 11  ) );
    CuAssertTrue( tc, !lcn_bitvector_get_bit( bv, 12  ) );


    LCN_TEST( lcn_bitvector_not( bv ) );
    CuAssertIntEquals( tc, 12, lcn_bitvector_count( bv ) );

    CuAssertTrue( tc, lcn_bitvector_get_bit( bv, 0  ) );
    CuAssertTrue( tc, lcn_bitvector_get_bit( bv, 1  ) );
    CuAssertTrue( tc, lcn_bitvector_get_bit( bv, 2  ) );
    CuAssertTrue( tc, lcn_bitvector_get_bit( bv, 3  ) );
    CuAssertTrue( tc, lcn_bitvector_get_bit( bv, 4  ) );
    CuAssertTrue( tc, lcn_bitvector_get_bit( bv, 5  ) );
    CuAssertTrue( tc, lcn_bitvector_get_bit( bv, 6  ) );
    CuAssertTrue( tc, lcn_bitvector_get_bit( bv, 7  ) );
    CuAssertTrue( tc, lcn_bitvector_get_bit( bv, 8  ) );
    CuAssertTrue( tc, lcn_bitvector_get_bit( bv, 9  ) );
    CuAssertTrue( tc, lcn_bitvector_get_bit( bv, 10  ) );
    CuAssertTrue( tc, lcn_bitvector_get_bit( bv, 11  ) );

    LCN_TEST( lcn_one_bitvector_create( &bv, 16, pool ));
    CuAssertIntEquals( tc, 16, lcn_bitvector_count( bv ) );

    LCN_TEST( lcn_one_bitvector_create( &bv, 11, pool ));
    CuAssertIntEquals( tc, 11, lcn_bitvector_count( bv ) );
    lcn_bitvector_set_bit( bv, 0 );
    CuAssertIntEquals( tc, 11, lcn_bitvector_count( bv ) );

    test_count_vector_of_size( tc, 8 );
    test_count_vector_of_size( tc, 20 );
    test_count_vector_of_size( tc, 100 );
    test_count_vector_of_size( tc, 1000 );
}

static void
do_test_write_read( CuTest *tc, int n )
{
    lcn_directory_t *dir;
    lcn_bitvector_t *bv;
    unsigned int i;
    apr_pool_t* pool;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );

    LCN_TEST( lcn_bitvector_create( &bv,n, pool) );

    lcn_ram_directory_create( &dir, pool );

    /* test count when incrementally setting bits */
    for( i = 0; i < lcn_bitvector_size( bv ); i++ )
    {
        lcn_bitvector_t *bvc;

        CuAssertTrue( tc, ! lcn_bitvector_get_bit(bv, i) );
        CuAssertIntEquals( tc, i, lcn_bitvector_count( bv ) );
        lcn_bitvector_set_bit(bv, i);
        CuAssertTrue( tc, lcn_bitvector_get_bit(bv, i ));
        CuAssertIntEquals( tc, i+1, lcn_bitvector_count( bv ) );

        /* test write directory functions */

        lcn_bitvector_write( bv, dir, "TESTBV", pool );
        LCN_TEST( lcn_bitvector_from_dir( &bvc,
                                          dir,
                                          "TESTBV",
                                          pool ) );

        /* compare bit vectors with bits set incrementally */
        CuAssertTrue( tc, lcn_bitvector_equals( bv, bvc ) );

        /* test old style writing */
        bvc = NULL;
        LCN_TEST( lcn_bitvector_dump_file( bv, "bv_file", pool ));
        LCN_TEST( lcn_bitvector_create_from_file( &bvc, "bv_file", pool ));
        CuAssertTrue( tc, lcn_bitvector_equals( bv, bvc ) );
        apr_file_remove("bv_file", pool );
    }

    apr_pool_destroy( pool );
}

/**
 * Test writing and construction to/from lcn_directory_t.
 */
static void
test_write_read( CuTest *tc )
{
    do_test_write_read(tc, 8);
#if 0
    do_test_write_read(tc, 20);
    do_test_write_read(tc, 100);
    do_test_write_read(tc, 1000);
#endif
}

static void
test_alloc_bits( CuTest *tc )
{
    lcn_bitvector_t *bv, *bv1, *bv2;
    apr_pool_t *pool;

    LCN_TEST( apr_pool_create( &pool, main_pool ));
    LCN_TEST( lcn_null_bitvector_create( &bv, 100, pool ));
    CuAssertIntEquals( tc, 100, lcn_bitvector_size( bv ));
    CuAssertTrue( tc, NULL == lcn_bitvector_bits( bv ));
    LCN_TEST( lcn_bitvector_alloc_bits( bv ));
    CuAssertTrue( tc, NULL != lcn_bitvector_bits( bv ));

    /* test clone */
    LCN_TEST( lcn_null_bitvector_create( &bv, 100, pool ));
    LCN_TEST( lcn_bitvector_clone( &bv1, bv, pool ));
    LCN_TEST( lcn_null_bitvector_create( &bv2, 101, pool ));
    CuAssertTrue( tc, lcn_bitvector_equals( bv, bv1 ));
    CuAssertTrue( tc, ! lcn_bitvector_equals( bv2, bv1 ));

    /* test clone */
    LCN_TEST( lcn_null_bitvector_create( &bv, 100, pool ));
    LCN_TEST( lcn_bitvector_set_bit( bv, 4));
    LCN_TEST( lcn_bitvector_clone( &bv1, bv, pool ));
    CuAssertTrue( tc, lcn_bitvector_equals( bv, bv1 ));

    apr_pool_destroy( pool );
}


static void
test_write_null_vector ( CuTest *tc )
{
    lcn_bitvector_t *bv;
    lcn_directory_t *dir;
    apr_pool_t *pool, *pool2;
    apr_pool_t *ram_dir_pool;
    unsigned int i;

    LCN_TEST( apr_pool_create( &pool, main_pool ));
    LCN_TEST( apr_pool_create( &ram_dir_pool, main_pool ));

    LCN_TEST( lcn_null_bitvector_create( &bv, 100, pool ));
    LCN_TEST( lcn_ram_directory_create( &dir, ram_dir_pool ) );
    LCN_TEST( lcn_bitvector_write( bv, dir, "TESTBV", ram_dir_pool ));

    LCN_TEST( apr_pool_create( &pool2, main_pool ));
    apr_pool_destroy( pool );

    LCN_TEST( lcn_bitvector_from_dir( &bv, dir, "TESTBV", pool2 ) );

    CuAssertIntEquals( tc, 100, lcn_bitvector_size( bv ));

    for( i = 0; i < 100; i++ )
    {
        CuAssertIntEquals( tc, 0, lcn_bitvector_get_bit( bv, i ));
    }

    apr_pool_destroy( pool2 );
    apr_pool_destroy( ram_dir_pool );
}

static void
test_write_one_vector ( CuTest *tc )
{
    lcn_bitvector_t *bv;
    lcn_directory_t *dir;
    apr_pool_t *pool, *pool2;
    apr_pool_t *ram_dir_pool;
    unsigned int i;

    LCN_TEST( apr_pool_create( &pool, main_pool ));
    LCN_TEST( apr_pool_create( &ram_dir_pool, main_pool ));

    LCN_TEST( lcn_one_bitvector_create( &bv, 100, pool ));
    LCN_TEST( lcn_ram_directory_create( &dir, ram_dir_pool ) );
    LCN_TEST( lcn_bitvector_write( bv, dir, "TESTBV", ram_dir_pool ));

    LCN_TEST( apr_pool_create( &pool2, main_pool ));
    apr_pool_destroy( pool );

    LCN_TEST( lcn_bitvector_from_dir( &bv, dir, "TESTBV", pool2 ) );

    CuAssertIntEquals( tc, 100, lcn_bitvector_size( bv ));

    for( i = 0; i < 100; i++ )
    {
        CuAssertIntEquals( tc, 1, lcn_bitvector_get_bit( bv, i ));
    }

    apr_pool_destroy( pool2 );
    apr_pool_destroy( ram_dir_pool );
}

static void
test_load_not_existing_bitvector( CuTest* tc )
{
    lcn_bitvector_t *bv;
    apr_pool_t* pool;

    LCN_TEST( apr_pool_create( &pool, main_pool ));
    CuAssertTrue( tc, APR_SUCCESS != lcn_bitvector_create_from_file( &bv, "~/thisisnotafilter.flt", pool ) );

    apr_pool_destroy( pool );
}

static void
test_or( CuTest* tc )
{
    lcn_bitvector_t *bv1, *bv2, *bv3;
    apr_pool_t *pool;

    LCN_TEST( apr_pool_create( &pool, main_pool ));

    LCN_TEST( lcn_bitvector_create( &bv1, 20, pool ));
    LCN_TEST( lcn_bitvector_set_bit( bv1, 10 ));
    CuAssertIntEquals(tc, 1, lcn_bitvector_count( bv1 ));

    LCN_TEST( lcn_bitvector_set_bit( bv1, 15));
    CuAssertIntEquals(tc, 2, lcn_bitvector_count( bv1 ));

    LCN_TEST( lcn_bitvector_create( &bv2, 20, pool ));
    LCN_TEST( lcn_bitvector_set_bit( bv2, 11 ));
    CuAssertIntEquals(tc, 1, lcn_bitvector_count( bv2 ));

    LCN_TEST( lcn_bitvector_set_bit( bv2, 13));
    CuAssertIntEquals(tc, 2, lcn_bitvector_count( bv2 ));

    LCN_TEST( lcn_bitvector_or( &bv3, bv1, bv2, pool ));
    CuAssertIntEquals(tc, 4, lcn_bitvector_count( bv3 ));

    {
        lcn_list_t *list;

        LCN_TEST( lcn_list_create( &list, 10, pool ));
        LCN_TEST( lcn_list_add( list, bv1 ));
        LCN_TEST( lcn_list_add( list, bv2 ));
        LCN_TEST( lcn_bitvector_create_by_concat( &bv3, list, pool ));

        CuAssertIntEquals(tc, 4, lcn_bitvector_count( bv3 ));
        CuAssertIntEquals(tc, 40, lcn_bitvector_size( bv3 ));
        CuAssertTrue( tc, lcn_bitvector_get_bit( bv3, 10 ));
        CuAssertTrue( tc, lcn_bitvector_get_bit( bv3, 15 ));
        CuAssertTrue( tc, lcn_bitvector_get_bit( bv3, 31 ));
        CuAssertTrue( tc, lcn_bitvector_get_bit( bv3, 33 ));
    }

    lcn_bitvector_clear_bit( bv1, 10 );
    lcn_bitvector_clear_bit( bv1, 15 );
    CuAssertIntEquals(tc, 0, lcn_bitvector_count( bv1 ));

    LCN_TEST( lcn_bitvector_or( &bv3, bv1, bv2, pool ));
    CuAssertIntEquals(tc, 2, lcn_bitvector_count( bv3 ));

    LCN_TEST( lcn_bitvector_clone( &bv3, bv2, pool ));
    CuAssertIntEquals(tc, 2, lcn_bitvector_count( bv3 ));

    LCN_TEST( lcn_null_bitvector_create( &bv1, 20, pool ));
    LCN_TEST( lcn_bitvector_or( &bv3, bv1, bv2, pool ));
    CuAssertIntEquals(tc, 2, lcn_bitvector_count( bv3 ));

    apr_pool_destroy( pool );
}


static void
test_uor( CuTest* tc )
{
    lcn_bitvector_t *bv1, *bv2, *bv3;
    apr_pool_t *pool;

    LCN_TEST( apr_pool_create( &pool, main_pool ));

    LCN_TEST( lcn_bitvector_create( &bv1, 20, pool ));
    LCN_TEST( lcn_bitvector_set_bit( bv1, 10 ));
    CuAssertIntEquals(tc, 1, lcn_bitvector_count( bv1 ));

    LCN_TEST( lcn_bitvector_set_bit( bv1, 15));
    CuAssertIntEquals(tc, 2, lcn_bitvector_count( bv1 ));

    LCN_TEST( lcn_bitvector_create( &bv2, 20, pool ));
    LCN_TEST( lcn_bitvector_set_bit( bv2, 11 ));
    CuAssertIntEquals(tc, 1, lcn_bitvector_count( bv2 ));

    LCN_TEST( lcn_bitvector_set_bit( bv2, 13));
    CuAssertIntEquals(tc, 2, lcn_bitvector_count( bv2 ));

    LCN_TEST( lcn_bitvector_uor( bv1, bv2 ));
    CuAssertIntEquals(tc, 4, lcn_bitvector_count( bv1 ));

    lcn_bitvector_clear_bit( bv1, 10 );
    lcn_bitvector_clear_bit( bv1, 15 );
    CuAssertIntEquals(tc, 2, lcn_bitvector_count( bv1 ));

    lcn_bitvector_clear_bit( bv1, 11 );
    lcn_bitvector_clear_bit( bv1, 13 );
    CuAssertIntEquals(tc, 0, lcn_bitvector_count( bv1 ));

    LCN_TEST( lcn_bitvector_uor( bv1, bv2 ));
    CuAssertIntEquals(tc, 2, lcn_bitvector_count( bv1 ));

    LCN_TEST( lcn_bitvector_clone( &bv3, bv1, pool ));
    CuAssertIntEquals(tc, 2, lcn_bitvector_count( bv3 ));

    LCN_TEST( lcn_null_bitvector_create( &bv1, 20, pool ));
    LCN_TEST( lcn_bitvector_uor( bv1, bv2 ));
    CuAssertIntEquals(tc, 2, lcn_bitvector_count( bv1 ));

    apr_pool_destroy( pool );
}

char *files[] = { "sf_ap_001.flt", "poolhb.flt","doctype_a.flt", "doctype_b.flt", "doctype_g.flt", "doctype_h.flt", "doctype_j.flt",
                 "doctype_s.flt", "doctype_u.flt", "doctype_v.flt", "doctype_vg.flt", "doctype_vo.flt",
                 "doctype_vv.flt", "doctype_z.flt", "d_stplkteu.flt", "poolbb.flt", "poolbe.flt",
                 "poolbw.flt", "poolby.flt", "poolhe.flt", "poolhh.flt", "poolmv.flt",
                 "poolnw.flt", "poolrp.flt", "poolsh.flt", "poolsl.flt", "poolsn.flt", "poolst.flt",
                 "poolth.flt", "r_.flt", "r_lreg.flt", "r_ls.flt", "r_mb.flt", "r_mfas.flt",
                 "r_mfeluf.flt", "r_mf.flt", "r_mi.flt", "r_mj.flt", "r_mk.flt", "r_ml.flt",
                 "r_ms.flt", "r_mu.flt", "r_mw.flt", "r_mwk.flt", "r_sozm.flt", "r_stk.flt",
                 "sf_ap_000.flt", "sf_ap_002.flt", "sf_ap_003.flt", "sf_ap_004.flt", "sf_ap_005.flt",
                 "sf_ap_006.flt", "sf_ap_007.flt", "sf_ap_008.flt", "sf_edv_000.flt", "sf_edv_001.flt", "sf_edv_002.flt",
                 "sf_edv_003.flt", "sf_edv_004.flt", "sf_edv_005.flt", "sf_edv_006.flt", "sf_edv_007.flt", "sf_edv_008.flt",
                 "sf_int_000.flt", "sf_int_001.flt", "sf_int_002.flt", "sf_int_003.flt", "sf_int_004.flt", "sf_int_005.flt",
                 "sf_int_006.flt", "sf_rpf_000.flt", "sf_rpf_001.flt", "sf_rpf_002.flt", "sf_rpf_003.flt", "sf_rpf_004.flt",
                 "sf_rpf_005.flt", "sf_rpf_006.flt", "sf_rpf_007.flt", "sf_rpf_008.flt", "sf_rpf_009.flt", "sf_rpf_010.flt",
                 "sf_rpf_011.flt", "sf_rpf_012.flt", "sf_rpf_013.flt", "sf_rpf_014.flt", "sf_rpf_015.flt", "sf_sor_000.flt",
                 "sf_sor_001.flt", "sf_sor_002.flt", "sf_sor_003.flt", "sf_soz_000.flt", "sf_soz_001.flt", "sf_soz_002.flt",
                 "sf_soz_003.flt", "sf_soz_004.flt", "sf_soz_005.flt", "sf_soz_006.flt", "sf_soz_007.flt", "sf_soz_008.flt",
                 "sf_soz_009.flt", "sf_soz_010.flt", "sf_soz_011.flt", "sf_soz_012.flt", "sf_soz_013.flt", "sf_soz_014.flt",
                 "sf_soz_015.flt", "sf_soz_016.flt", "sf_sta_000.flt", "sf_sta_001.flt", "sf_sta_002.flt", "sf_sta_003.flt",
                 "sf_sta_004.flt", "sf_sta_005.flt", "sf_sta_006.flt", "sf_sta_007.flt", "sf_sta_008.flt", "sf_svr_000.flt",
                 "sf_svr_001.flt", "sf_svr_002.flt", "sf_svr_003.flt", "sf_svr_004.flt", "sf_svr_005.flt", "sf_svr_006.flt",
                 "sf_svr_007.flt", "sf_vwr_000.flt", "sf_vwr_001.flt", "sf_vwr_002.flt", "sf_vwr_003.flt", "sf_vwr_004.flt",
                 "sf_vwr_005.flt", "sf_vwr_006.flt", "sf_vwr_007.flt", "sf_vwr_008.flt", "sf_vwr_009.flt", "sf_vwr_010.flt",
                 "sf_vwr_011.flt", "sf_vwr_012.flt", "sf_vwr_013.flt", "sf_vwr_014.flt", "sf_vwr_015.flt", "sf_vwr_016.flt",
                 "sf_vwr_017.flt", "sf_vwr_018.flt", "sf_vwr_019.flt", "sf_vwr_020.flt", "sf_vwr_021.flt", "sf_vwr_022.flt",
                 "sf_vwr_023.flt", "sf_wirt_000.flt", "sf_wirt_001.flt", "sf_wirt_002.flt", "sf_wirt_003.flt", "sf_wirt_004.flt",
                 "sf_wirt_005.flt", "sf_wirt_006.flt", "sf_wirt_007.flt", "sf_wirt_008.flt", "sf_wirt_009.flt", "sf_wirt_010.flt",
                 "sf_wirt_011.flt", "sf_wirt_012.flt", "sf_wirt_013.flt", "sf_wirt_014.flt", "sf_wirt_015.flt", "sf_wirt_016.flt",
                 "sf_wirt_017.flt", "sf_wirt_018.flt", "sf_wirt_019.flt", "sf_wirt_020.flt", "sf_wirt_021.flt", "sf_wirt_022.flt",
                 "sf_zir_000.flt", "sf_zir_001.flt", "sf_zir_002.flt", "sf_zir_003.flt", "sf_zir_004.flt", "sf_zir_005.flt",
                 "sf_zir_006.flt", "sf_zir_007.flt", "sf_zir_008.flt", "stplkteu.flt", "stplktfn.flt", "stplktni.flt",
                 "stplktwe.flt", "tv_aip.flt", "tv_ango.flt", "tv_angw.flt", "tv_arbo.flt", "tv_arbw.flt",
                 "tv_atz.flt", "tv_ausz.flt", "tv_krpf.flt", "tv_neu.flt", "tv_prak.flt", "tv_spark.flt", "tv_weit.flt", NULL
    };

#if 0
static void
test_write_gap( CuTest* tc )
{
    unsigned int i;
    const char path[] = "/home/rk/filter/";
    const char pathc[] = "/home/rk/filterc/";
    apr_pool_t *pool;

    LCN_TEST( apr_pool_create( &pool, main_pool ));

    for( i = 0; i < NULL != files[i]; i++ )
    {
        //fprintf(stderr, "write %s\n", files[i] );
        lcn_bitvector_t *bv;
        lcn_ostream_t *os;
        unsigned int j = 0;

        LCN_TEST( lcn_bitvector_create_from_file( &bv,
                                                  apr_pstrcat( pool, pathc, files[i], NULL ),
                                                  pool ));
#if 0
        LCN_TEST( lcn_bitvector_write_file( bv,
                                            apr_pstrcat( pool, pathc, files[i], NULL ),
                                            pool ));
#endif
        apr_pool_clear( pool );
    }
}
#endif

CuSuite *make_bitvector_suite (void)
{
    CuSuite *s= CuSuiteNew();

#if 1
    SUITE_ADD_TEST( s, test_construct_size );
    SUITE_ADD_TEST( s, test_get_set        );
    SUITE_ADD_TEST( s, test_clear          );
    SUITE_ADD_TEST( s, test_count          );
    SUITE_ADD_TEST( s, test_write_read     );
    SUITE_ADD_TEST( s, test_alloc_bits     );
    SUITE_ADD_TEST( s, test_write_null_vector );
    SUITE_ADD_TEST( s, test_write_one_vector );
    SUITE_ADD_TEST( s, test_load_not_existing_bitvector );
    SUITE_ADD_TEST( s, test_or );
    SUITE_ADD_TEST( s, test_uor );
#endif
    //SUITE_ADD_TEST( s, test_write_gap );

    return s;
}
