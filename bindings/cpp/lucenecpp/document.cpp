#include "document.hpp"
#include "analyzermap.hpp"

NAMESPACE_LCN_BEGIN

static void createHash( apr_hash_t** hash, apr_pool_t* pool )
{
    *hash = apr_hash_make( pool );
}

Document::Document()
  : PoolObject<lcn_document_t>()
{}

String 
Document::get( const String& field )
    throw( IOException )
{
    apr_status_t s;
    AutoReleasePool pool;
    char* c_result;

    if( ( s = lcn_document_get( *this, &c_result, field, pool ) ) )
    {
	LCN_THROW_IO_EXCEPTION( s, "Error getting Documentfield" );
    }

    return String( c_result );
}

int
Document::fieldCount() const
{
    return (int)lcn_list_size( lcn_document_fields( *this ) );
}

Document
Document::fromDump( const char* dumpTxt,
                    const char** endPos,
                    const lcn_document_dump_iterator_t *iterator )
{
    apr_status_t s;
    Document result;

    if( ( s = lcn_document_create_from_dump( &result,
                                             iterator,
                                             dumpTxt, 
                                             endPos, 
                                             result.pool() )
            ) )
    {
        LCN_THROW_IO_EXCEPTION( s, "Error creating Document from Dump" );
    }

    return result;
}

void
Document::field( lcn_field_t** field, lcn_document_t* doc, int n )
{
    *field = (lcn_field_t*)lcn_list_get( 
        lcn_document_fields( doc ), n );
}

Field
Document::field( int nth ) const
{
    Field result;

    field( &result, *this, nth );

    result.assertNotNull();

    return result;
}

bool 
Document::sameFieldsAs( const Document& other ) const
{
    if( fieldCount() != other.fieldCount() )
    {
        return false;
    }

    for( int i = 0; i < fieldCount(); i++ )
    {
        if( field( i ).name() != other.field( i ).name() )
        {
            return false;
        }
    }

    return true;
}

NAMESPACE_LCN_END
