cmake_minimum_required(VERSION 2.4)

SET( CMAKE_BUILD_TYPE "Release" )

if( NOT APR_INCLUDE )
    if ( WIN32 )
        set( APR_INCLUDE "C:\\Program Files\\Microsoft Visual Studio .NET 2003\\Vc7\\include\\apr-1" )
    else ( WIN32 )
        set( APR_INCLUDE /usr/include/apr-1.0 /usr/include/apr-1 )
    endif ( WIN32 )
endif( NOT APR_INCLUDE )

if( NOT APR_LIB )
    if ( WIN32 )
        set( APR_LIB apr-1.lib )
    else ( WIN32 )
        set( APR_LIB apr-1 )
    endif ( WIN32 )
endif(NOT APR_LIB)

if ( NOT WIN32 )
    set( CMAKE_CXX_COMPILER gcc )
    set( CMAKE_C_FLAGS "-D_LARGEFILE64_SOURCE -fPIC -Wall -DDEBUG -g" )
endif ( NOT WIN32 )

add_subdirectory( build )
add_subdirectory( bindings )

if(EXISTS nv)
	add_subdirectory( nv )
endif(EXISTS nv)
