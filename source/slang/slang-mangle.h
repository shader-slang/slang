#ifndef SLANG_MANGLE_H_INCLUDED
#define SLANG_MANGLE_H_INCLUDED

// This file implements the name mangling scheme for the Slang language.

#include "../core/slang-basic.h"
#include "slang-syntax.h"

namespace Slang
{
    struct IRSpecialize;

    String getMangledName(Decl* decl);
    String getMangledName(DeclRef<Decl> const & declRef);
    String getMangledName(DeclRefBase const & declRef);

    String getMangledNameForConformanceWitness(
        Type* sub,
        Type* sup);
    String getMangledNameForConformanceWitness(
        DeclRef<Decl> sub,
        DeclRef<Decl> sup);
    String getMangledNameForConformanceWitness(
        DeclRef<Decl> sub,
        Type* sup);
    String getMangledTypeName(Type* type);
}

#endif
