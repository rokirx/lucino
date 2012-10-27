#include "indexreader.hpp"

NAMESPACE_LCN_BEGIN

IndexReader::IndexReader()
  : PoolObject<lcn_index_reader_t>()
{
}

#if 0
IndexReader( const String& path ) throw( IOException )
  : PoolObject<lcn_index_reader_t>() 
{
    
}
#endif

void
IndexReader::open( const String& path )
    throw( IOException )
{
    if( !isNull() )
    {
        clear();
    }

    apr_status_t s;

    if( ( s = lcn_index_reader_create_by_path( &(*this),
					       path.c_str(),
					       pool() ) ) )
    {
	LCN_THROW_IO_EXCEPTION( s, path );
    }
}

void 
IndexReader::close() throw( IOException )    
{
    apr_status_t s;
    if( ( s = lcn_index_reader_close( *this ) ) )
    {
	LCN_THROW_IO_EXCEPTION( s, "Error closing IndexReader" );
    }
}

NAMESPACE_LCN_END
