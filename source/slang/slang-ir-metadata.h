// slang-ir-metadata.h
#pragma once

namespace Slang
{

class PostEmitMetadataImpl;
struct IRModule;

void collectMetadata(const IRModule* irModule, PostEmitMetadataImpl& outMetadata);

}
