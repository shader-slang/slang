// slang-ir-transform-to-descriptor-handles.h
#pragma once

#include "slang-ir.h"

namespace Slang
{
    struct IRModule;
    class TargetProgram;
    class DiagnosticSink;

    /// Transform resource types in struct fields to DescriptorHandle<ResourceType>
    /// for structs that are used in parameter blocks.
    ///
    /// This transformation:
    /// 1. Identifies structs used within ParameterBlock
    /// 2. Converts resource-type fields in those structs to DescriptorHandle<ResourceType>
    /// 3. Inserts CastDescriptorHandleToResource instructions at field access sites
    ///
    /// This should be called before legalizeResourceTypes for Metal targets.
    void transformResourceTypesToDescriptorHandles(
        TargetProgram* targetProgram, 
        IRModule* module, 
        DiagnosticSink* sink);
} 