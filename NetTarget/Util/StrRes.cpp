// StrRes.cpp
//
//
// Copyright (c) 1995-1998 - Richard Langlois and Grokksoft Inc.
//
// Licensed under GrokkSoft HoverRace SourceCode License v1.0(the "License");
// you may not use this file except in compliance with the License.
//
// A copy of the license should have been attached to the package from which 
// you have taken this file. If you can not find the license you can not use 
// this file.
//
//
// The author makes no representations about the suitability of
// this software for any purpose.  It is provided "as is" "AS IS",
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.
//
// See the License for the specific language governing permissions 
// and limitations under the License.
//

// Project Includes
#include "StrRes.h"
#include "nomfc_stdafx.h"
#include <unordered_map>
#include <string>
#include <stdexcept>
#include <memory>fixed minor include errors in Util. 
Util & ColorTools now building on linux.

static std::unique_ptr<std::unordered_map<int, std::string>> g_stringTable;

void EnsureStringTableInitialized()
{
    static bool initialized = false;
    if (!initialized)
    {
         g_stringTable = std::make_unique<std::unordered_map<int, std::string>>();
         LoadGameStringResources(g_stringTable.get());
         initialized = true;
    }
}

const char* MR_LoadString( int pResource )
{
    EnsureStringTableInitialized();

    auto it = g_stringTable->find(pResource);
    if (it != g_stringTable->end()) {
        return it->second.c_str();
    }
    return "RESOURCE NOT FOUND";
}

const char* MR_LoadStringBuffered( int pResource )
{
    EnsureStringTableInitialized();

    auto it = g_stringTable->find(pResource);
    if (it != g_stringTable->end()) {
        return it->second.c_str();
    }
    return "RESOURCE NOT FOUND";
}