// vk-api.cpp
#include "vk-api.h"

#include "core/slang-string.h"

namespace gfx {

void installPipelineDumpLayer(VulkanApi& api);
void writePipelineDump(Slang::UnownedStringSlice path);

} // renderer_test
