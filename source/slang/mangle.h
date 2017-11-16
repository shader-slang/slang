#ifndef SLANG_MANGLE_H_INCLUDED
#define SLANG_MANGLE_H_INCLUDED

// This file implements the name mangling scheme for the Slang language.

#include "../core/basic.h"
#include "syntax.h"

namespace Slang
{
    String getMangledName(Decl* decl);
    String getMangledName(DeclRef<Decl> const & declRef);
    String getMangledName(DeclRefBase const & declRef);

    String mangleSpecializedFuncName(String baseName, RefPtr<Substitutions> subst);
    String getMangledNameForConformanceWitness(
        Type* sub,
        Type* sup);

    String getMangledTypeName(Type* type);
}

#endif