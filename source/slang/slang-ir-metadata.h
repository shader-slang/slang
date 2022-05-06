// slang-ir-metadata.h
#pragma once

namespace Slang
{

struct PostEmitMetadata;
struct IRModule;

void collectMetadata(const IRModule* irModule, PostEmitMetadata& outMetadata);

}
