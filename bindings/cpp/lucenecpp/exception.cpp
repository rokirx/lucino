#include "exception.hpp"

NAMESPACE_LCN_BEGIN


Exception::Exception( apr_status_t status, const char* file, int line,
		      const char* userMessage )
{
    _status = status;

    initBuffer( _fileBuf, file, LCN_MAX_FILENAME_LEN );
    initBuffer( _userMessageBuf, userMessage,
		LCN_MAX_ERROR_MSG_LEN );
    _line = line;
}

Exception::~Exception()
{

}

void
Exception::initBuffer( char* dest, const char* source, 
		       unsigned int destLen )
{
    if( NULL == source )
    {
	dest[0] = '\0';
    }
    else
    {
	apr_cpystrn( dest, source, destLen );
    }
}

const char* 
Exception::message()
{
    unsigned int fileLen;
    const char* filePtr = _fileBuf;
    filePtr += ( fileLen = strlen( _fileBuf ) ) - 1;

    while( fileLen )
    {
	if( *filePtr == '/' || *filePtr == '\\' )
	{
	    filePtr++;
	    break;
	}
	filePtr--; fileLen--;
    }

    char errBuf[ LCN_MAX_ERROR_MSG_LEN];

    lcn_strerror( _status, errBuf, LCN_MAX_ERROR_MSG_LEN );

    apr_snprintf( _msgBuf, LCN_MAX_ERROR_MSG_LEN, "%s : %d : \"%s\" : %s", 
		  filePtr, 
		  _line, 
		  _userMessageBuf,
		  errBuf );
    
    return _msgBuf;
}

OutOfMemoryException::OutOfMemoryException( const char* file, int line )
    : Exception( APR_ENOMEM, file, line )
{}

const char*
OutOfMemoryException::message()
{
    char errorBuf[ LCN_MAX_ERROR_MSG_LEN];

    apr_cpystrn( errorBuf, Exception::message(), 
		 LCN_MAX_ERROR_MSG_LEN );
    
    apr_snprintf( _msgBuf, LCN_MAX_ERROR_MSG_LEN,
		  "OutOfMemoryException: %s ",
		  errorBuf );

    return _msgBuf;
}

IOException::IOException( apr_status_t status, const char* userMessage,
			  const char* file, int line )
    : Exception( status, file, line, userMessage )
{
    fprintf( stderr, "%s : %d\n", file, line );
    initBuffer( _fileNameBuf, NULL, 0 );
}

const char* 
IOException::message()
{
    char errorBuf[ LCN_MAX_ERROR_MSG_LEN];

    apr_cpystrn( errorBuf, Exception::message(), 
		 LCN_MAX_ERROR_MSG_LEN );

    apr_snprintf( _msgBuf, LCN_MAX_ERROR_MSG_LEN,
		  "IOException on '%s' : %s ", _fileNameBuf,
		  errorBuf );

    return _msgBuf;
}

const char* 
IOException::fileName()
{
    return _fileNameBuf;
}

NAMESPACE_LCN_END
