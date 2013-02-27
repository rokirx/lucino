cmake_minimum_required(VERSION 2.4)

set( APR_INCLUDE /usr/include/apr-1.0 /usr/include/apr-1 )
set( APR_LIB     apr-1 )
set( CMAKE_CXX_COMPILER gcc )
set( CMAKE_C_FLAGS "-D_LARGEFILE64_SOURCE -fPIC -Wall -DDEBUG -g" )

add_subdirectory( build )
add_subdirectory( bindings )

if(EXISTS nv)
	add_subdirectory( nv )
endif(EXISTS nv)
