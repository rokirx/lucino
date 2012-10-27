#include "indexsearcher.hpp"

NAMESPACE_LCN_BEGIN

IndexSearcher::IndexSearcher()
  : Searcher()
{
}

IndexSearcher::IndexSearcher( const String& path  ) throw( IOException )
  : Searcher()
{
    open( path );
}

void
IndexSearcher::open( const String& path ) throw( IOException )
{
    apr_status_t rv;
    _path = path;

    if( ( rv = lcn_index_searcher_create_by_path( &(*this), 
						  _path , 
						  pool() ) ) )
    {
	LCN_THROW_IO_EXCEPTION( rv, _path );
    }

}
void
IndexSearcher::close() throw( IOException )
{
    if( isNull() )
    {
        return; 
    }

    apr_status_t rv;

    if( ( rv = lcn_index_searcher_close( *this ) ) )
    {
	LCN_THROW_IO_EXCEPTION( rv, _path );
    }
}
NAMESPACE_LCN_END
