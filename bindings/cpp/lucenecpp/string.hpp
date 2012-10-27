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

#ifndef STRING_HPP
#define STRING_HPP

#include <lcn_util.h>
#include "lucenecpp.hpp"

NAMESPACE_LCN_BEGIN

class String
{
public:
    String();
    String( const char* c_str );
    String( const String& other );
    ~String();
    int length() const;
    bool isNull() const;
    bool isEmpty() const;

    char& operator[]( int i ) const;
    String& operator=( const String& other );
    String& operator=( const char* c_str );

    bool operator==( const String& other ) const;
    bool operator==( const char* other ) const;

    operator const char*() const;
    const char* c_str() const;
private:
    void increaseCapacityTo( int nBytes ); 

    int   _length;
    int   _capacity;
    char* _bytes;
};

NAMESPACE_LCN_END

#endif /* STRING_HPP */
