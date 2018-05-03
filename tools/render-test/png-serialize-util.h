// png-serialize-util.h
#pragma once

#include "surface.h"
    
namespace renderer_test {

struct PngSerializeUtil 
{
    static Slang::Result write(const char* filename, const Surface& surface);

};

} // renderer_test
