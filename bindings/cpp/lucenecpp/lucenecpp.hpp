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

#ifndef LUCENECPP_HPP
#define LUCENECPP_HPP

#include <lucene.h>
#include <iostream>

#ifdef WIN32
#pragma warning( disable : 4290 )
#endif

#define NAMESPACE_LCN_BEGIN namespace lcn {
#define NAMESPACE_LCN_END   };

NAMESPACE_LCN_BEGIN

using std::cerr;
using std::endl;

void Initialize();
void Terminate();

NAMESPACE_LCN_END

#endif
