/*
 * Copyright 2010 Esrille Inc.
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

#ifndef ES_CSSINPUTSTREAM_H
#define ES_CSSINPUTSTREAM_H

#include "U16InputStream.h"

class CSSInputStream : public U16InputStream
{
    void detect(const char* p);
public:
    CSSInputStream(std::istream& stream, const std::string optionalEncoding = "");
};

#endif  // ES_CSSINPUTSTREAM_H