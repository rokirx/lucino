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

#ifndef DIRECTORY_HPP
#define DIRECTORY_HPP

#include <lcn_store.h>
#include "lucenecpp.hpp"
#include "poolobject.hpp"
#include "list.hpp"
#include "string.hpp"
#include "exception.hpp"

NAMESPACE_LCN_BEGIN

class Directory : public PoolObject<lcn_directory_t>
{
public:
    Directory();

    void open( const String& path, bool doCreate = false ) 
	throw( IOException );
    List<String> list() const;

    void close()
	throw( IOException );
    void renameFile( const String& oldName, const String& newName )
	throw( IOException );
    bool fileExists( const String& fileName ) const
	throw( IOException );
private:

};

NAMESPACE_LCN_END

#endif
