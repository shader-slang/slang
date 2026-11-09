#pragma once

#include "slang-hgvm/slang-hgvm.h"
#include "unit-test/slang-unit-test.h"

namespace Slang::HGVM
{
/// Compiles a host program through the same HGBC path used by slangi.
inline RefPtr<Program> compileHGVMTestProgram(
    slang::IGlobalSession* global,
    const char* source,
    SlangDebugInfoLevel debugLevel = SLANG_DEBUG_INFO_LEVEL_NONE,
    const char* moduleName = "hgvm_events")
{
    slang::CompilerOptionEntry options[2]{};
    options[0].name = slang::CompilerOptionName::ExperimentalFeature;
    options[0].value.kind = slang::CompilerOptionValueKind::Int;
    options[0].value.intValue0 = 1;
    options[1].name = slang::CompilerOptionName::DebugInformation;
    options[1].value.kind = slang::CompilerOptionValueKind::Int;
    options[1].value.intValue0 = debugLevel;
    slang::TargetDesc target{};
    target.format = SLANG_HGBC;
    slang::SessionDesc desc{};
    desc.targets = &target;
    desc.targetCount = 1;
    desc.compilerOptionEntries = options;
    desc.compilerOptionEntryCount = SLANG_COUNT_OF(options);
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(global->createSession(desc, session.writeRef())));
    ComPtr<slang::IBlob> diagnostics;
    String sourcePath = String(moduleName) + ".slang";
    auto module = session->loadModuleFromSourceString(
        moduleName,
        sourcePath.getBuffer(),
        source,
        diagnostics.writeRef());
    if (diagnostics)
        fprintf(stderr, "%s", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(module);
    ComPtr<slang::IEntryPoint> entry;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->findEntryPointByName("main", entry.writeRef())));
    slang::IComponentType* parts[] = {module, entry};
    ComPtr<slang::IComponentType> composite, linked;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        parts,
        2,
        composite.writeRef(),
        diagnostics.writeRef())));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(composite->link(linked.writeRef(), diagnostics.writeRef())));
    ComPtr<slang::IBlob> code;
    SlangResult result = linked->getTargetCode(0, code.writeRef(), diagnostics.writeRef());
    if (diagnostics)
        fprintf(stderr, "%s", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
    RefPtr<Program> program;
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(Program::load(
        code->getBufferPointer(),
        code->getBufferSize(),
        moduleName,
        program,
        error)));
    return program;
}

} // namespace Slang::HGVM
