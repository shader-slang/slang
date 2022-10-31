#ifndef SLANG_CPP_HOST_PRELUDE_H
#define SLANG_CPP_HOST_PRELUDE_H

#include <cstdio>
#include <cmath>
#include <cstring>

#define SLANG_COM_PTR_ENABLE_REF_OPERATOR 1

#include "../source/slang-rt/slang-rt.h"
#include "../slang-com-ptr.h"
#include "slang-cpp-types.h"

#ifdef SLANG_LLVM
#include "slang-llvm.h"
#else // SLANG_LLVM
#   if SLANG_GCC_FAMILY && __GNUC__ < 6
#       include <cmath>
#       define SLANG_PRELUDE_STD std::
#   else
#       include <math.h>
#       define SLANG_PRELUDE_STD
#   endif

#   include <assert.h>
#   include <stdlib.h>
#   include <string.h>
#   include <stdint.h>
#endif // SLANG_LLVM


#include "slang-cpp-scalar-intrinsics.h"

using namespace Slang;

template<typename TResult, typename... Args>
using Slang_FuncType = TResult(SLANG_MCALL *)(Args...);

#endif
