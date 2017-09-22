#ifndef SLANG_MANGLE_H_INCLUDED
#define SLANG_MANGLE_H_INCLUDED

// This file implements the name mangling scheme for the Slang language.

#include "../core/basic.h"

namespace Slang
{
    struct Decl;

    String getMangledName(Decl* decl);
}

#endif