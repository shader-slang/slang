// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Standard-module discovery must work for embedding applications without an additional source
// search path. Separate sessions also verify that no earlier import supplies a dependency.
SLANG_UNIT_TEST(standardHostModulesDiscoverable)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::CompilerOptionEntry experimental = {};
    experimental.name = slang::CompilerOptionName::ExperimentalFeature;
    experimental.value.kind = slang::CompilerOptionValueKind::Int;
    experimental.value.intValue0 = 1;
    slang::SessionDesc sessionDesc = {};
    sessionDesc.compilerOptionEntries = &experimental;
    sessionDesc.compilerOptionEntryCount = 1;

    const char* modules[] = {
        "slang/runtime",
        "slang/pipeline_rt",
        "slang/runtime/compute/grid",
        "slang/compute/buffer_utils",
        "slang/compute/texture_utils",
        "slang/resource/resizable_buffer",
        "slang/tensor",
        "slang/tensor/differentiable",
    };
    for (const char* name : modules)
    {
        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);
        ComPtr<ISlangBlob> diagnostics;
        auto module = session->loadModule(name, diagnostics.writeRef());
        SLANG_CHECK_ABORT(module != nullptr);
    }

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);
    ComPtr<ISlangBlob> diagnostics;
    const char* source = R"(
        import slang.runtime;
        import slang.runtime.compute.grid;
        import slang.compute.buffer_utils;
        import slang.compute.texture_utils;
        import slang.tensor;
        import slang.tensor.differentiable;

        [host]
        Tensor<float, 1> makeValues()
        {
            DispatchGrid<uint> grid = Grid(4u);
            let values = Tensor<float, 1>(grid.size);
            values.fill(2.0f);
            return values.map((uint1 index, float value) => value + 1.0f);
        }
    )";
    auto consumer = session->loadModuleFromSourceString(
        "standard_host_consumer",
        "standard-host-consumer.slang",
        source,
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(consumer != nullptr);
}
