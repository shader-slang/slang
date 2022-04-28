// slang-ir-metadata.h
#pragma once

namespace Slang
{

class PostEmitMetadata;
struct IRModule;

void collectMetadata(const IRModule* irModule, PostEmitMetadata& outMetadata);

}
