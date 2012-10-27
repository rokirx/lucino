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

#ifndef PREFIXQUERY_HPP
#define PREFIXQUERY_HPP

#include "string.hpp"
#include "term.hpp"
#include "query.hpp"

NAMESPACE_LCN_BEGIN

class PrefixQuery : public Query
{
public:
    PrefixQuery( const Term& term );
    ~PrefixQuery();
};

NAMESPACE_LCN_END

#endif /* PREFIXQUERY_HPP */
