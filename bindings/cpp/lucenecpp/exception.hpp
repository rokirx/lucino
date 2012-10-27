#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include "lucenecpp.hpp"

#define LCN_MAX_ERROR_MSG_LEN (256)
#define LCN_MAX_FILENAME_LEN  (256)

#define LCN_THROW_MEMORY_EXCEPTION() \
        throw OutOfMemoryException( __FILE__, __LINE__ )

#define LCN_THROW_IO_EXCEPTION( STAT, MSG ) \
        throw IOException( STAT, MSG, __FILE__, __LINE__ )

NAMESPACE_LCN_BEGIN

class Exception
{
public:
    Exception( apr_status_t status, const char* file = NULL, int line = -1,
	       const char* userMessage = NULL );
    
    virtual ~Exception();
    virtual const char* message();

protected:
    static void initBuffer( char* dest, const char* source,
			    unsigned int len );
    apr_status_t _status;

    /* Buffer holding the message of the exception */
    char _msgBuf[ LCN_MAX_ERROR_MSG_LEN ];

    /* Buffer holding the name of the implementation file
     * where the Exception was thrown
     */
    char _fileBuf[ LCN_MAX_FILENAME_LEN];
    char _userMessageBuf[ LCN_MAX_ERROR_MSG_LEN ];

    int _line;
    
};

class OutOfMemoryException : public Exception
{
public:
    OutOfMemoryException( const char* file, int line );
    const char* message();
};

class IOException : public Exception
{
public:
    IOException( apr_status_t, const char* msg, 
		 const char* file, int line );

    virtual const char* message();
    virtual const char* fileName();
protected:
    char _fileNameBuf[ LCN_MAX_FILENAME_LEN ];
};

NAMESPACE_LCN_END

#endif /* EXCEPTION_HPP */
