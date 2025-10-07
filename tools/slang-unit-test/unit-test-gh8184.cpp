// unit-test-gh8184.cpp

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// A regression test for github issue 8184.
//
// We fixed three issues with this regression test:
// 1. After generating IR for a SpecializeComponentType, we should also strip the frontend
//    decorations from the IR so there is no HighLevelDeclDecoration that will go into the backend.
// 2. When lowering a static const inside a generic function, we should not give the static const
//    a linkage, because it won't such constant will not appear in global scope. Trying to give it a
//    linkage decoration will lead to the parent generic (for the function) to have two duplicate
//    Export/Import decorations with different mangle names, and confuses the linker.
// 3. Make sure internal exceptions does not leak through
//    IComponentType::getEntryPointCode/getTargetCode.
//
SLANG_UNIT_TEST(gh8184)
{
    ComPtr<slang::IGlobalSession> globalSession;
    {
        SlangGlobalSessionDesc globalDesc = {};
        SLANG_CHECK_ABORT(createGlobalSession(&globalDesc, globalSession.writeRef()) == SLANG_OK);
    }

    ComPtr<slang::ISession> session;
    {
        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_WGSL;

        slang::SessionDesc sessionDesc = {};
        sessionDesc.targets = &targetDesc;
        sessionDesc.targetCount = 1;
        sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;

        SLANG_CHECK_ABORT(
            globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);
    }

    const char* shaderCode = R"SLANG(
          interface Transformation
          {
              float3 apply(float3 coord);
              static const uint32_t kParamCount;
          }

          struct T1 : Transformation {
              float3 apply(float3 coord) { return float3(0, 0, 0); }
              static const uint32_t kParamCount = 2;
          };

          struct T2 : Transformation {
              float3 apply(float3 coord) { return float3(0, 0, 0); }
              static const uint32_t kParamCount = 4;
          };

          [shader("compute")]
          [numthreads(1, 1, 1)]
          void XYPass<T>()
          where T : Transformation
          {
              static const uint32_t kParamCount = T::kParamCount;
          }
      )SLANG";

    Slang::ComPtr<slang::IModule> module;
    Slang::ComPtr<slang::IBlob> diagnostics;
    {
        const char* moduleName = "bugrepro";
        const char* virtualPath = "bugrepro.slang";
        module = session->loadModuleFromSourceString(
            moduleName,
            virtualPath,
            shaderCode,
            diagnostics.writeRef());
        SLANG_CHECK_ABORT(module != nullptr);
    }

    Slang::ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_CHECK_ABORT(module->findEntryPointByName("XYPass", entryPoint.writeRef()) == SLANG_OK);

    Slang::ComPtr<slang::IComponentType> specializedEntryPoint;
    {
        slang::ProgramLayout* programLayout = module->getLayout();
        SLANG_CHECK_ABORT(programLayout != nullptr);
        auto* t1Type = programLayout->findTypeByName("T1");
        SLANG_CHECK_ABORT(t1Type != nullptr);

        slang::SpecializationArg arg = {};
        arg.kind = slang::SpecializationArg::Kind::Type;
        arg.type = t1Type;

        SLANG_CHECK_ABORT(
            entryPoint
                ->specialize(&arg, 1, specializedEntryPoint.writeRef(), diagnostics.writeRef()) ==
            SLANG_OK);
    }

    Slang::ComPtr<slang::IComponentType> program;
    {
        slang::IComponentType* components[] = {module.get(), specializedEntryPoint.get()};
        SLANG_CHECK_ABORT(
            session->createCompositeComponentType(components, 2, program.writeRef()) == SLANG_OK);
    }

    Slang::ComPtr<slang::IComponentType> linked;
    SLANG_CHECK_ABORT(program->link(linked.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    Slang::ComPtr<slang::IBlob> code;
    SLANG_CHECK(
        linked->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef()) == SLANG_OK);
}
