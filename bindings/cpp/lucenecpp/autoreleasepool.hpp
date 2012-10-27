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

#ifndef AUTORELEASEPOOL_HPP
#define AUTORELEASEPOOL_HPP

#include "apr_pools.h"
#include "lucenecpp.hpp"

NAMESPACE_LCN_BEGIN

/**
 * @brief Class for safe use of APR-Pools
 *
 * AutoReleasePool holds a reference-counter.
 * If this becomes 0 after decrementing in destructor,
 * the internal apr_pool_t is destroyed
 */

class AutoReleasePool
{
public:

    /**
     * Standard-Constructor
     */
    AutoReleasePool();
    
    /**
     * @brief Generates a new pool with <code>parent</code> as
     * parent pool
     */

    AutoReleasePool( apr_pool_t* parent );

    /**
     * @brief Copy-Constructor.
     * Increments the Reference-Counter on the internal structure
     */

    AutoReleasePool( const AutoReleasePool& other );

    /**
     * @brief Releases the internal pool, retaining the <code>other</code>
     * pool, i.e. incrementing the reference counter
     */

    AutoReleasePool& operator=( const AutoReleasePool& other );

    /**
     * @brief Decrements the Reference-Counter on the internal structure.
     *        If 0, destroys the internal pool
     */

    ~AutoReleasePool();

    /**
     * @brief Tells you, by how many Objects the internal structure is
     *        momentarily referenced
     */

    int refCount() const;

    /**
     * @brief refCount() == 1 : the internal pool is cleared
     *        refCount() >  1 : the binding to the internal structure
     *                          is released, and a new, empty pool is allocated
     */

    void clear();

    /**
     * @brief Returns the pointer of the internal structure.
     *        No need to say, that destroying or clearing by hand would be
     *        a universally stupid idea.
     */
    apr_pool_t* pool();

    /**
     * @brief Cast-Operator to the apr_pool_t*
     */

    operator apr_pool_t*();

    void retain( AutoReleasePool& other );

protected:
private:
    void createPool( apr_pool_t* parent = NULL );
    void destroyPool();
    void clearPool();

    apr_pool_t* _pool;
    mutable int* _refCount;
    bool _poolWasKilled;
};

NAMESPACE_LCN_END

#endif /* AUTORELEASEPOOL_HPP */
