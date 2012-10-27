#include "directory.hpp"

NAMESPACE_LCN_BEGIN

Directory::Directory()
  : PoolObject<lcn_directory_t>()
{}

void
Directory::open( const String& path, bool doCreate )
    throw( IOException )
{
    apr_status_t s;  

    if( ( s = lcn_fs_directory_create( &((*this)), path, (lcn_bool_t)doCreate,
				       pool() ) ) )
    {
	LCN_THROW_IO_EXCEPTION( s, (const char*)path);
    }
}

void
Directory::close()
    throw( IOException )
{
    apr_status_t s;
    if( ( s = lcn_directory_close( *this ) ) )
    {
	LCN_THROW_IO_EXCEPTION( s, "Error closing Directory" );
    }
}

List<String> 
Directory::list() const
{
    List<String> result;
    PoolObject<lcn_list_t> c_list;

    lcn_directory_list( *this, &c_list, c_list.pool() );
    
    for( unsigned int i = 0; i < lcn_list_size( c_list ); i++ )
    {
        result.push_back( String( (const char*)lcn_list_get( c_list, i ) ) );
    }
    
    return result;
}

void
Directory::renameFile( const String& oldName, const String& newName )
    throw( IOException )
{
    apr_status_t s;

    if( ( s = lcn_directory_rename_file( *this, oldName, newName ) ) )
    {
	LCN_THROW_IO_EXCEPTION( s, "Error renaming file" );	
    }
}


bool 
Directory::fileExists( const String& fileName ) const
    throw( IOException )
{
    apr_status_t s;
    lcn_bool_t exists;

    if( ( s = lcn_directory_file_exists( *this, fileName, &exists ) ) )
    {
	LCN_THROW_IO_EXCEPTION( s, fileName );
    }
    
    return (bool)exists;
}

NAMESPACE_LCN_END
