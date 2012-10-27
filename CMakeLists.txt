
set( APR_INCLUDE /usr/include/apr-1.0 )
set( APR_LIB     apr-1 )
set( CMAKE_CXX_COMPILER gcc )

add_subdirectory( build )
add_subdirectory( bindings )
