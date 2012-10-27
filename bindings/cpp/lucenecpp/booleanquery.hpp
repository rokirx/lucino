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

#ifndef BOOLEANQUERY_HPP
#define BOOLEANQUERY_HPP

#include "query.hpp"
#include "list.hpp"

NAMESPACE_LCN_BEGIN

class BooleanQuery : public Query
{
public:
    BooleanQuery();

    enum Occur
    {
        Should,
        Must,
        MustNot
    };

    void add( const Query& query, Occur occur );

    static Occur occurFromCType( lcn_boolean_clause_occur_t occur );
private:
    List<Query> _clauses;
};

NAMESPACE_LCN_END

#endif /* BOOLEANQUERY_HPP */
