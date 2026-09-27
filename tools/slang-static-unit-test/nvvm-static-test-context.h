// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef SLANG_TOOLS_NVVM_STATIC_TEST_CONTEXT_H
#define SLANG_TOOLS_NVVM_STATIC_TEST_CONTEXT_H

#include "compiler-core/slang-diagnostic-sink.h"
#include "slang/slang-code-gen.h"
#include "slang/slang-module.h"
#include "slang/slang-session.h"
#include "static-unit-test-env.h"

namespace Slang
{

// Give direct preflight and type-lowering calls a real CUDA target and diagnostic owner without
// invoking target compilation. Each test supplies its own canonical IR instructions.
struct NVVMStaticTestContext
{
    static TargetProgram* addTarget(Module* module)
    {
        SLANG_RELEASE_ASSERT(module);
        slang::TargetDesc desc = {};
        desc.format = SLANG_PTX;
        auto linkage = module->getLinkage();
        linkage->addTarget(desc);
        return module->getTargetProgram(linkage->targets.getLast());
    }

    explicit NVVMStaticTestContext(UnitTestContext* testContext)
        : env(testContext)
        , owner(
              env.checkModuleFromSource("nvvmStaticTestContext", "struct ContextOwner { uint x; }"))
        , targetProgram(addTarget(owner))
        , sink(owner->getLinkage()->getSourceManager(), nullptr)
        , shared(targetProgram, entryIndices, &sink, nullptr)
        , codeGen(&shared)
    {
    }

    StaticUnitTestEnv env;
    Module* owner;
    TargetProgram* targetProgram;
    DiagnosticSink sink;
    CodeGenContext::EntryPointIndices entryIndices;
    CodeGenContext::Shared shared;
    CodeGenContext codeGen;
};

} // namespace Slang

#endif
