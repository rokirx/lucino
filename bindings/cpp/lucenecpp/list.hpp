/* 
 * Copyright 2006 Sebastian Morawietz
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIST_HPP
#define LIST_HPP

#include <lcn_util.h>
#include "lucenecpp.hpp"


NAMESPACE_LCN_BEGIN

template <typename T>
class List
{
public:
    List() { apr_pool_create( &_pool, NULL ); lcn_list_create( &_c_list, 10, _pool ); }
    ~List() { 
        
        for( unsigned int i = 0; i < lcn_list_size( _c_list ); i++ )
        {
            T* oneT = (T*)lcn_list_get( _c_list, i );
            delete oneT;
        }
        apr_pool_destroy( _pool );
    };
    List<T>& push_back( const T& t ) {
        T* newT = new T();
        (*newT ) = t;
        lcn_list_add( _c_list, (void*)newT );
        
        return *this;
    }
    
    T& operator[]( int i ){
        T* oneT = (T*)lcn_list_get( _c_list, (unsigned int)i );
        return *oneT;
    }
    int size() { return (int)lcn_list_size(_c_list); }
private:
    apr_pool_t* _pool;
    lcn_list_t* _c_list;
};

NAMESPACE_LCN_END

#endif /* LIST_HPP */
