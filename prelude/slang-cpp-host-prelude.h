#ifndef SLANG_CPP_HOST_PRELUDE_H
#define SLANG_CPP_HOST_PRELUDE_H

#include <cstdio>
#include <cmath>
#include <cstring>

#define SLANG_COM_PTR_ENABLE_REF_OPERATOR 1

#include "../source/slang-rt/slang-rt.h"
#include "../slang-com-ptr.h"
#include "slang-cpp-types.h"

using namespace Slang;

template<typename TResult, typename... Args>
using Slang_FuncType = TResult(SLANG_MCALL *)(Args...);

#endif
