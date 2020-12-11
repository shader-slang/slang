// slang-capability-defs.h

// This file uses macros to define the capability "atoms" that
// are used by the `CapabilitySet` implementation.
//
// Any file that `#include`s this file is required to set
// the `SLANG_CAPABILITY_ATOM` macro before including it.
//
#ifndef SLANG_CAPABILITY_ATOM
#error Must define SLANG_CAPABILITY_ATOM before including.
#endif
//
// It is not necessary to `#undef` the macro in the client
// file, because this file will `#undef` it at the end.

// Our representation allows each capability atom to define
// a number of other base atoms that it "inherits" from.
//
// Different atoms will need different numbers of bases,
// so we will define a few different macros that wrap
// `SLANG_CAPABILITY_ATOM` and let us handle the cases
// more conveniently.
//
// TODO: There is probably a way to handle this with
// variadic macros.
//
#define SLANG_CAPABILITY_ATOM4(ENUMERATOR, NAME, FLAGS, BASE0, BASE1, BASE2, BASE3) \
    SLANG_CAPABILITY_ATOM(ENUMERATOR, NAME, FLAGS, BASE0, BASE1, BASE2, BASE3)

#define SLANG_CAPABILITY_ATOM3(ENUMERATOR, NAME, FLAGS, BASE0, BASE1, BASE2) \
    SLANG_CAPABILITY_ATOM(ENUMERATOR, NAME, FLAGS, BASE0, BASE1, BASE2, Invalid)

#define SLANG_CAPABILITY_ATOM2(ENUMERATOR, NAME, FLAGS, BASE0, BASE1) \
    SLANG_CAPABILITY_ATOM(ENUMERATOR, NAME, FLAGS, BASE0, BASE1, Invalid, Invalid)

#define SLANG_CAPABILITY_ATOM1(ENUMERATOR, NAME, FLAGS, BASE0) \
    SLANG_CAPABILITY_ATOM(ENUMERATOR, NAME, FLAGS, BASE0, Invalid, Invalid, Invalid)

#define SLANG_CAPABILITY_ATOM0(ENUMERATOR, NAME, FLAGS) \
    SLANG_CAPABILITY_ATOM(ENUMERATOR, NAME, FLAGS, Invalid, Invalid, Invalid, Invalid)

// The `__target` capability exists only to provide a common
// abstract base for the capabilities that represent each
// of our compilation targets.
//
SLANG_CAPABILITY_ATOM0(Target,   __target,   Abstract)

SLANG_CAPABILITY_ATOM1(HLSL,     hlsl,      Concrete,   Target)
SLANG_CAPABILITY_ATOM1(GLSL,     glsl,      Concrete,   Target)
SLANG_CAPABILITY_ATOM1(C,        c,         Concrete,   Target)
SLANG_CAPABILITY_ATOM1(CPP,      cpp,       Concrete,   Target)
SLANG_CAPABILITY_ATOM1(CUDA,     cuda,      Concrete,   Target)
SLANG_CAPABILITY_ATOM1(SPIRV,    spirv,     Concrete,   Target)


#undef SLANG_CAPABILITY_ATOM0
#undef SLANG_CAPABILITY_ATOM1
#undef SLANG_CAPABILITY_ATOM2
#undef SLANG_CAPABILITY_ATOM3
#undef SLANG_CAPABILITY_ATOM4

#undef SLANG_CAPABILITY_ATOM
