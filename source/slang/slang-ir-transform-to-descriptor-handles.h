// slang-ir-transform-to-descriptor-handles.h
#pragma once

#include "slang-ir.h"

namespace Slang
{
    struct IRModule;
    class TargetProgram;
    class DiagnosticSink;

    /// Transform resource types in struct fields to DescriptorHandle<ResourceType>
    /// This should be called before legalizeResourceTypes for Metal targets.
    void transformResourceTypesToDescriptorHandles(
        TargetProgram* targetProgram, 
        IRModule* module, 
        DiagnosticSink* sink);
} 