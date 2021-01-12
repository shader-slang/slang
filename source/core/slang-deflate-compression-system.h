#ifndef SLANG_DEFLATE_COMPRESSION_SYSTEM_H
#define SLANG_DEFLATE_COMPRESSION_SYSTEM_H

#include "slang-basic.h"

#include "slang-compression-system.h"

#include "../../slang-com-ptr.h"

namespace Slang
{

class DeflateCompressionSystem
{
public:
    /* Get the Deflate compression system singleton. */
    static ICompressionSystem* getSingleton();
};

}

#endif
