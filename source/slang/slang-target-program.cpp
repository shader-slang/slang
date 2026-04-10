// slang-target-program.cpp
#include "slang-target-program.h"

#include "slang-compiler.h"
#include "slang-rich-diagnostics.h"
#include "slang-type-layout.h"

namespace Slang
{

//
// TargetProgram
//

TargetProgram::TargetProgram(ComponentType* componentType, TargetRequest* targetReq)
    : m_program(componentType), m_targetReq(targetReq)
{
    m_entryPointResults.setCount(componentType->getEntryPointCount());
    m_optionSet.overrideWith(m_program->getOptionSet());
    m_optionSet.inheritFrom(targetReq->getOptionSet());
}

IArtifact* TargetProgram::_createWholeProgramResult(
    DiagnosticSink* sink,
    EndToEndCompileRequest* endToEndReq)
{
    // We want to call `emitEntryPoints` function to generate code that contains
    // all the entrypoints defined in `m_program`.
    // The current logic of `emitEntryPoints` takes a list of entry-point indices to
    // emit code for, so we construct such a list first.
    List<Int> entryPointIndices;

    entryPointIndices.setCount(m_program->getEntryPointCount());
    for (Index i = 0; i < entryPointIndices.getCount(); i++)
        entryPointIndices[i] = i;

    CodeGenContext::Shared sharedCodeGenContext(this, entryPointIndices, sink, endToEndReq);
    CodeGenContext codeGenContext(&sharedCodeGenContext);

    ComPtr<IArtifact> artifact;
    if (SLANG_FAILED(codeGenContext.emitEntryPoints(artifact)))
    {
        return nullptr;
    }

    {
        std::lock_guard<std::mutex> lock(m_resultCacheMutex);
        if (m_wholeProgramResult)
            return m_wholeProgramResult;
        m_wholeProgramResult = artifact;
    }

    return m_wholeProgramResult;
}

IArtifact* TargetProgram::_createEntryPointResult(
    Int entryPointIndex,
    DiagnosticSink* sink,
    EndToEndCompileRequest* endToEndReq)
{
    if (entryPointIndex < 0)
        return nullptr;

    // It is possible that entry points got added to the `Program`
    // *after* we created this `TargetProgram`, so there might be
    // a request for an entry point that we didn't allocate space for.
    //
    // TODO: Change the construction logic so that a `Program` is
    // constructed all at once rather than incrementally, to avoid
    // this problem.
    //
    CodeGenContext::EntryPointIndices entryPointIndices;
    entryPointIndices.add(entryPointIndex);

    CodeGenContext::Shared sharedCodeGenContext(this, entryPointIndices, sink, endToEndReq);
    CodeGenContext codeGenContext(&sharedCodeGenContext);

    ComPtr<IArtifact> artifact;
    if (SLANG_FAILED(codeGenContext.emitEntryPoints(artifact)))
    {
        return nullptr;
    }

    {
        std::lock_guard<std::mutex> lock(m_resultCacheMutex);
        if (entryPointIndex >= m_entryPointResults.getCount())
            m_entryPointResults.setCount(entryPointIndex + 1);
        if (m_entryPointResults[entryPointIndex])
            return m_entryPointResults[entryPointIndex];
        m_entryPointResults[entryPointIndex] = artifact;
    }

    return m_entryPointResults[entryPointIndex];
}

IArtifact* TargetProgram::getOrCreateWholeProgramResult(DiagnosticSink* sink)
{
    {
        std::lock_guard<std::mutex> lock(m_resultCacheMutex);
        if (m_wholeProgramResult)
            return m_wholeProgramResult;
    }

    // If we haven't yet computed a layout for this target
    // program, we need to make sure that is done before
    // code generation.
    //
    if (!getOrCreateIRModuleForLayout(sink))
    {
        return nullptr;
    }

    return _createWholeProgramResult(sink);
}

IArtifact* TargetProgram::getOrCreateEntryPointResult(Int entryPointIndex, DiagnosticSink* sink)
{
    if (entryPointIndex < 0)
        return nullptr;

    {
        std::lock_guard<std::mutex> lock(m_resultCacheMutex);
        if (entryPointIndex >= m_entryPointResults.getCount())
            m_entryPointResults.setCount(entryPointIndex + 1);

        if (IArtifact* artifact = m_entryPointResults[entryPointIndex])
            return artifact;
    }

    try
    {
        // If we haven't yet computed a layout for this target
        // program, we need to make sure that is done before
        // code generation.
        //
        if (!getOrCreateIRModuleForLayout(sink))
        {
            return nullptr;
        }

        return _createEntryPointResult(entryPointIndex, sink);
    }
    catch (const Exception& e)
    {
        sink->diagnose(Diagnostics::CompilationAbortedDueToException{
            .exceptionType = typeid(e).name(),
            .exceptionMessage = e.Message});
        return nullptr;
    }
}

} // namespace Slang
