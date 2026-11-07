#include "core/slang-io.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

SLANG_UNIT_TEST(vkd3dGeneratedDXBC)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_VKD3D)))
    {
        SLANG_IGNORE_TEST;
    }
    globalSession->setDownstreamCompilerForTransition(
        SLANG_HLSL,
        SLANG_DXBC,
        SLANG_PASS_THROUGH_VKD3D);

    slang::TargetDesc target;
    target.format = SLANG_DXBC;
    target.profile = globalSession->findProfile("cs_5_0");
    slang::SessionDesc desc;
    desc.targetCount = 1;
    desc.targets = &target;
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(desc, session.writeRef()) == SLANG_OK);

    const char* path = "tests/downstream/vkd3d/compute.slang";
    String source;
    SLANG_CHECK_ABORT(File::readAllText(path, source) == SLANG_OK);
    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "vkd3d",
        path,
        source.getBuffer(),
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);
    ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_CHECK_ABORT(
        module->findEntryPointByName("computeMain", entryPoint.writeRef()) == SLANG_OK);

    slang::IComponentType* components[] = {module, entryPoint};
    ComPtr<slang::IComponentType> program;
    SLANG_CHECK_ABORT(
        session->createCompositeComponentType(
            components,
            2,
            program.writeRef(),
            diagnostics.writeRef()) == SLANG_OK);
    ComPtr<slang::IComponentType> linked;
    SLANG_CHECK_ABORT(program->link(linked.writeRef(), diagnostics.writeRef()) == SLANG_OK);
    ComPtr<slang::IBlob> code;
    SLANG_CHECK_ABORT(
        linked->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef()) == SLANG_OK);
    SLANG_CHECK_ABORT(code != nullptr && code->getBufferSize() >= 32);
    SLANG_CHECK(memcmp(code->getBufferPointer(), "DXBC", 4) == 0);
}
