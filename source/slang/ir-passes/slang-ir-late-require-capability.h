// slang-ir-late-require-capability.h
#pragma once

#include "slang.h"

namespace Slang
{
struct CodeGenContext;
class DiagnosticSink;
struct IRModule;

enum class EnumRequirementMaterializationPhase
{
    BeforeSpecialization,
    AfterSpecialization,
    Final,
};

/// Turn declaration-owned enum obligations into checks at their source uses. Keep unresolved
/// anchors through intermediate specialization so later passes can expose their concrete owners.
void materializeEnumRequirements(
    IRModule* module,
    EnumRequirementMaterializationPhase phase =
        EnumRequirementMaterializationPhase::BeforeSpecialization);

/// Validate and eliminate late capability requirements and deferred enum conversions. Diagnose
/// missing capabilities as warnings or errors depending on whether restrictive capability checks
/// are enabled (-restrictive-capability-check).
void processLateRequireCapabilityInsts(
    IRModule* module,
    CodeGenContext* codeGenContext,
    DiagnosticSink* sink);

} // namespace Slang
