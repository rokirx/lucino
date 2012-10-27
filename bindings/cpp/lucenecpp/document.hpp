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

#ifndef DOCUMENT_HPP
#define DOCUMENT_HPP

#include "poolobject.hpp"
#include "string.hpp"
#include "exception.hpp"
#include "analyzermap.hpp"
#include "field.hpp"

NAMESPACE_LCN_BEGIN

class Document : public PoolObject<lcn_document_t>
{
public:
    Document();
  
    String get( const String& field ) throw( IOException );

    int fieldCount() const;
    Field field( int nth ) const;

    bool sameFieldsAs( const Document& other ) const;

    static Document fromDump( const char* dumpTxt, 
                              const char** endPos,
                              const lcn_document_dump_iterator_t *iterator );
private:
    static void field(lcn_field_t** field, lcn_document_t* doc, int n );
};

NAMESPACE_LCN_END

#endif /* DOCUMENT_HPP */
