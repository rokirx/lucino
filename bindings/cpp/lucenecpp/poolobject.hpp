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

#ifndef POOLOBJECT_HPP
#define POOLOBJECT_HPP

#include "lcn_util.h"
#include "autoreleasepool.hpp"
#include "exception.hpp"
#include "list.hpp"
NAMESPACE_LCN_BEGIN

template <typename T>
class PoolObject
{
public:
    PoolObject() { _t = NULL; }
    PoolObject( const PoolObject<T>& other ) : _pool( other._pool ) {
        _t = other._t;
    }
    inline T** operator&() { return &_t; }

    operator T*() const {  return _t; }

    AutoReleasePool& pool() { return _pool; } 

    bool isNull() const { return ( _t == NULL ); };

    void assertNotNull() throw( OutOfMemoryException ) {
	if( isNull() )
	{
	    LCN_THROW_MEMORY_EXCEPTION();
	}
    }

    void clear() { _t = NULL; _pool.clear(); }

    PoolObject& operator=( const PoolObject<T>& other ) {
        _pool = other._pool;
        _t    = other._t;
        return *this;
    }
    void retain( AutoReleasePool& pool ) { _retainedPools.push_back( pool ); }
private:
    AutoReleasePool _pool;
    T* _t;
    List<AutoReleasePool> _retainedPools;
};

NAMESPACE_LCN_END

#endif /* POOLOBJECT_HPP */

