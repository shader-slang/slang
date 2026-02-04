// unit-test-entrypoint-compile.cpp

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

SLANG_UNIT_TEST(entryPointCompile)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    // Request SPIR-V disassembly so we can check the content.
    targetDesc.format = SLANG_SPIRV_ASM;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    List<slang::CompilerOptionEntry> optionEntries;
    {
        slang::CompilerOptionEntry entry;
        entry.name = slang::CompilerOptionName::EnableEffectAnnotations;
        entry.value.kind = slang::CompilerOptionValueKind::Int;
        entry.value.intValue0 = 1;
        optionEntries.add(entry);
    }

    sessionDesc.compilerOptionEntries = optionEntries.getBuffer();
    sessionDesc.compilerOptionEntryCount = (uint32_t)optionEntries.getCount();

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    String userSourceBody;
    File::readAllText("d:\\test\\valve-dbg.hlsl", userSourceBody);
    auto srcBlob = StringBlob::moveCreate(_Move(userSourceBody));
    auto module = session->loadModuleFromSource("m", "m.slang", srcBlob, diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "MainVs",
        SLANG_STAGE_VERTEX,
        entryPoint.writeRef(),
        diagnosticBlob.writeRef());

    ComPtr<slang::IComponentType> linkedProgram;
    entryPoint->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(linkedProgram != nullptr);

    ComPtr<slang::IBlob> code;
    linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(code != nullptr);

    SLANG_CHECK_ABORT(code->getBufferSize() != 0);
}
