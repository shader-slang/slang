// render-vk.cpp
#include "render-vk.h"
#include "core/slang-blob.h"
#include "vk-util.h"

// Vulkan has a different coordinate system to ogl
// http://anki3d.org/vulkan-coordinate-system/
#ifndef ENABLE_VALIDATION_LAYER
#    if _DEBUG
#        define ENABLE_VALIDATION_LAYER 1
#    else
#        define ENABLE_VALIDATION_LAYER 0
#    endif
#endif

#ifdef _MSC_VER
#    include <stddef.h>
#    pragma warning(disable : 4996)
#    if (_MSC_VER < 1900)
#        define snprintf sprintf_s
#    endif
#endif

#if SLANG_WINDOWS_FAMILY
#    include <dxgi1_2.h>
#endif

namespace gfx
{
using namespace Slang;

namespace vk
{
namespace
{

} // namespace















































} // namespace vk



} // namespace gfx
