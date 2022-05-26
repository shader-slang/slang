// render-vk.h
#pragma once

#include "../command-encoder-com-forward.h"
#include "../mutable-shader-object.h"
#include "../renderer-shared.h"
#include "../transient-resource-heap-base.h"
#include "core/slang-chunked-list.h"
#include "vk-api.h"
#include "vk-descriptor-allocator.h"
#include "vk-device-queue.h"

namespace gfx
{
namespace vk
{
using namespace Slang;

enum
{

    kMaxPushConstantSize = 256,
    kMaxDescriptorSets = 8,
};






















} // namespace vk
} // namespace gfx
