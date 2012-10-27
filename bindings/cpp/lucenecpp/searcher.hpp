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

#ifndef SEARCHER_HPP
#define SEARCHER_HPP

#include <lcn_search.h>
#include "poolobject.hpp"
#include "string.hpp"
#include "hits.hpp"
#include "query.hpp"
#include "exception.hpp"

NAMESPACE_LCN_BEGIN

class Searcher : public PoolObject<lcn_searcher_t>
{
public:
    Hits search( const Query& query ) throw( IOException );
};


NAMESPACE_LCN_END

#endif /* SEARCHER_HPP */
