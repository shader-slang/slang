// slang-target-program.h
#pragma once

//
// This file declares the `TargetProgram` class, which is
// primarily used to cache generated target code for a
// linked program/binary and/or its entry points.
//

#include "../core/slang-smart-pointer.h"
#include "slang-hlsl-to-vulkan-layout-options.h"
#include "slang-ir.h"
#include "slang-linkable.h"
#include "slang-target.h"

namespace Slang
{

/// A `TargetProgram` represents a `ComponentType` specialized for a particular `TargetRequest`
///
/// TODO: This should probably be renamed to `TargetComponentType`.
///
/// By binding a component type to a specific target, a `TargetProgram` allows
/// for things like layout to be computed, that fundamentally depend on
/// the choice of target.
///
/// A `TargetProgram` handles request for compiled kernel code for
/// entry point functions. In practice, kernel code can only be
/// correctly generated when the underlying `ComponentType` is "fully linked"
/// (has no remaining unsatisfied requirements).
///
class TargetProgram : public RefObject
{
public:
    TargetProgram(ComponentType* componentType, TargetRequest* targetReq);

    /// Get the underlying program
    ComponentType* getProgram() { return m_program; }

    /// Get the underlying target
    TargetRequest* getTargetReq() { return m_targetReq; }

    /// Get the layout for the program on the target.
    ///
    /// If this is the first time the layout has been
    /// requested, report any errors that arise during
    /// layout to the given `sink`.
    ///
    ProgramLayout* getOrCreateLayout(DiagnosticSink* sink);

    /// Get the layout for the program on the target.
    ///
    /// This routine assumes that `getOrCreateLayout`
    /// has already been called previously.
    ///
    ProgramLayout* getExistingLayout()
    {
        SLANG_ASSERT(m_layout);
        return m_layout;
    }

    /// Get the compiled code for an entry point on the target.
    ///
    /// If this is the first time that code generation has
    /// been requested, report any errors that arise during
    /// code generation to the given `sink`.
    ///
    IArtifact* getOrCreateEntryPointResult(Int entryPointIndex, DiagnosticSink* sink);
    IArtifact* getOrCreateWholeProgramResult(DiagnosticSink* sink);

    IArtifact* getExistingWholeProgramResult() { return m_wholeProgramResult; }
    /// Get the compiled code for an entry point on the target.
    ///
    /// This routine assumes that `getOrCreateEntryPointResult`
    /// has already been called previously.
    ///
    IArtifact* getExistingEntryPointResult(Int entryPointIndex)
    {
        return m_entryPointResults[entryPointIndex];
    }

    IArtifact* _createWholeProgramResult(
        DiagnosticSink* sink,
        EndToEndCompileRequest* endToEndReq = nullptr);

    /// Internal helper for `getOrCreateEntryPointResult`.
    ///
    /// This is used so that command-line and API-based
    /// requests for code can bottleneck through the same place.
    ///
    /// Shouldn't be called directly by most code.
    ///
    IArtifact* _createEntryPointResult(
        Int entryPointIndex,
        DiagnosticSink* sink,
        EndToEndCompileRequest* endToEndReq = nullptr);

    RefPtr<IRModule> getOrCreateIRModuleForLayout(DiagnosticSink* sink);

    RefPtr<IRModule> getExistingIRModuleForLayout() { return m_irModuleForLayout; }

    CompilerOptionSet& getOptionSet() { return m_optionSet; }

    HLSLToVulkanLayoutOptions* getHLSLToVulkanLayoutOptions()
    {
        return m_targetReq->getHLSLToVulkanLayoutOptions();
    }

    bool shouldEmitSPIRVDirectly()
    {
        return isSPIRV(m_targetReq->getTarget()) && getOptionSet().shouldEmitSPIRVDirectly();
    }

private:
    RefPtr<IRModule> createIRModuleForLayout(DiagnosticSink* sink);

    // The program being compiled or laid out
    ComponentType* m_program;

    // The target that code/layout will be generated for
    TargetRequest* m_targetReq;

    // The computed layout, if it has been generated yet
    RefPtr<ProgramLayout> m_layout;

    CompilerOptionSet m_optionSet;

    // Generated compile results for each entry point
    // in the parent `Program` (indexing matches
    // the order they are given in the `Program`)
    ComPtr<IArtifact> m_wholeProgramResult;
    List<ComPtr<IArtifact>> m_entryPointResults;

    RefPtr<IRModule> m_irModuleForLayout;
};

/// Given a target request returns which (if any) intermediate source language is required
/// to produce it.
///
/// If no intermediate source language is required, will return SourceLanguage::Unknown
SourceLanguage getIntermediateSourceLanguageForTarget(TargetProgram* req);

} // namespace Slang
