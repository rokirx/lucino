#include "string.hpp"

NAMESPACE_LCN_BEGIN

String::String()
{
    _bytes    = NULL;
    _length   = 0;
    _capacity = 0;
}

String::String( const char* c_str )
{
    _length   = (int)strlen( c_str );
    _capacity = _length + 1;
    _bytes    = new char[ _capacity ];

    memcpy( _bytes, c_str, _length );

    _bytes[ _length ] = '\0';
}

String::~String()
{
    if( !isNull() )
    {
        delete[] _bytes;
    }
}

String::String( const String& other )
{
    _bytes    = NULL;
    _length   = 0;
    _capacity = 0;

    operator=( other );
}

char&
String::operator[]( int i ) const
{
    return _bytes[i];
}

String::operator const char*() const
{
    if( isNull() )
    {
        return "";
    }

    return _bytes;
}

const char*
String::c_str() const
{
    if( isNull() )
    {
        return "";
    }

    return _bytes;
}

String&
String::operator=( const String& other )
{
    if( other._bytes == _bytes )
    {
        return *this;
    }

    if( !isNull() )
    {
        delete[] _bytes;
    }

    _capacity = other._capacity;
    _length   = other._length;

    _bytes = new char[_capacity];

    memcpy( _bytes, other._bytes, other._length );
    _bytes[_length] = 0;

    return *this;
}

String&
String::operator=( const char* c_str )
{
    if( _bytes == c_str )
    {
        return *this;
    }

    _length = (int)strlen( c_str );

    if( isNull() )
    {
        _capacity = _length + 1;
        _bytes = new char[_capacity];
        strcpy( _bytes, c_str );
    }

    return *this;
}

bool
String::isEmpty() const
{
    return ( isNull() ) ? true : ( _bytes[0] == '\0' );
}

int
String::length() const
{
    return _length;
}

bool
String::isNull() const
 {
    return ( NULL == _bytes );
}

void
String::increaseCapacityTo( int nBytes )
{
    if( ( 0 == nBytes ) || ( nBytes <= _capacity ) )
    {
        return;
    }

    if( !isNull() )
    {
        char* sav = _bytes;

        _bytes = new char[nBytes];

        if( _length > 0 )
        {
            memcpy( _bytes, sav, _length );
            _bytes[_length] = '\0';
        }

        delete[] sav;
    }
    else
    {
        _bytes = new char[nBytes];
        _bytes[0] = '\0';
    }
}

bool
String::operator==( const String& other ) const
{
    return operator==( other._bytes );
}

bool
String::operator==( const char* other ) const
{
    if( isNull() )
    {
        if( ( other == NULL ) || ( other[0] == 0 ) )
        {
            return true;
        }

        return false;
    }

    if( other == NULL )
    {
        return false;
    }

    return (bool)( strcmp( _bytes, other ) == 0 );
}

NAMESPACE_LCN_END

