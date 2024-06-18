#pragma once

#include <stddef.h>
#include <stdint.h>

struct tint_CompileRequest
{
    uint32_t const* spirvCode;
    size_t spirvCodeLength;
    char const* resultBuffer;
    size_t resultBufferSize;
};

typedef int (*tint_CompileFunc)(tint_CompileRequest* request);
