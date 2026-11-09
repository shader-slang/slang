// hgbc-compiler.cpp
#include "slang-hgvm-compiler.h"

#include "core/slang-io.h"

namespace Slang::HGVM
{
using namespace Slang;

namespace
{

String getSourceDirectory(const char* sourcePath)
{
    String directory = Path::getParentDirectory(sourcePath);
    return directory.getLength() ? directory : ".";
}

void appendDiagnostics(slang::IBlob* diagnostics, String& outDiagnostics)
{
    if (!diagnostics || !diagnostics->getBufferSize())
        return;
    outDiagnostics.append(
        static_cast<const char*>(diagnostics->getBufferPointer()),
        diagnostics->getBufferSize());
}

} // namespace

struct HGBCCompiler::Impl
{
    ComPtr<slang::IGlobalSession> globalSession;
    ComPtr<ISlangFileSystemExt> fileSystem;
    Dictionary<String, ComPtr<slang::ISession>> sessions;
};

HGBCCompiler::HGBCCompiler(slang::IGlobalSession* globalSession, ISlangFileSystemExt* fileSystem)
    : m_impl(new Impl)
{
    m_impl->globalSession = globalSession;
    m_impl->fileSystem = fileSystem;
}

HGBCCompiler::~HGBCCompiler() = default;

SlangResult HGBCCompiler::compile(
    const char* sourcePath,
    const char* entryPointName,
    const char* const* additionalSearchPaths,
    SlangInt additionalSearchPathCount,
    SlangDebugInfoLevel debugInfoLevel,
    SlangDiagnosticColor diagnosticColor,
    ComPtr<slang::IBlob>& outCode,
    String& outDiagnostics,
    bool reportCheckpointIntermediates)
{
    outCode = nullptr;
    outDiagnostics = "";
    if (!m_impl->globalSession || !sourcePath || !sourcePath[0] ||
        (entryPointName && !entryPointName[0]) || additionalSearchPathCount < 0)
    {
        return SLANG_E_INVALID_ARG;
    }

    String canonicalPath;
    SlangResult pathResult;
    if (m_impl->fileSystem)
    {
        ComPtr<ISlangBlob> path;
        pathResult = m_impl->fileSystem->getPath(PathKind::Canonical, sourcePath, path.writeRef());
        if (SLANG_SUCCEEDED(pathResult))
            canonicalPath = static_cast<const char*>(path->getBufferPointer());
    }
    else
    {
        pathResult = Path::getCanonical(sourcePath, canonicalPath);
    }
    if (SLANG_FAILED(pathResult))
    {
        outDiagnostics = String("Cannot resolve Slang source path: ") + sourcePath;
        return SLANG_FAIL;
    }
    sourcePath = canonicalPath.getBuffer();
    String sourceDirectory = getSourceDirectory(sourcePath);
    List<String> searchPathStorage;
    searchPathStorage.add(sourceDirectory);
    for (SlangInt i = 0; i < additionalSearchPathCount; ++i)
    {
        if (!additionalSearchPaths || !additionalSearchPaths[i])
            return SLANG_E_INVALID_ARG;
        searchPathStorage.add(additionalSearchPaths[i]);
    }

    // A Slang session owns its module cache, so its key includes every setting that affects module
    // loading or generated HGBC. Tests with the same source directory and include paths then reuse
    // imported demo libraries while tests in unrelated directories cannot collide by module name.
    StringBuilder sessionKey;
    sessionKey << int(debugInfoLevel) << ":" << int(diagnosticColor) << ":"
               << reportCheckpointIntermediates;
    for (const String& searchPath : searchPathStorage)
        sessionKey << ":" << searchPath.getLength() << ":" << searchPath;
    String sessionKeyString = sessionKey.produceString();

    slang::ISession* session = nullptr;
    if (auto cachedSession = m_impl->sessions.tryGetValue(sessionKeyString))
    {
        session = cachedSession->get();
    }
    else
    {
        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_HGBC;
        slang::SessionDesc sessionDesc = {};
        sessionDesc.fileSystem = m_impl->fileSystem;
        sessionDesc.targetCount = 1;
        sessionDesc.targets = &targetDesc;
        slang::CompilerOptionEntry compilerOptions[4] = {};
        compilerOptions[0].name = slang::CompilerOptionName::DiagnosticColor;
        compilerOptions[0].value.kind = slang::CompilerOptionValueKind::Int;
        compilerOptions[0].value.intValue0 = diagnosticColor;
        compilerOptions[1].name = slang::CompilerOptionName::DebugInformation;
        compilerOptions[1].value.kind = slang::CompilerOptionValueKind::Int;
        compilerOptions[1].value.intValue0 = debugInfoLevel;
        compilerOptions[2].name = slang::CompilerOptionName::ExperimentalFeature;
        compilerOptions[2].value.kind = slang::CompilerOptionValueKind::Int;
        compilerOptions[2].value.intValue0 = 1;
        compilerOptions[3].name = slang::CompilerOptionName::ReportCheckpointIntermediates;
        compilerOptions[3].value.kind = slang::CompilerOptionValueKind::Int;
        compilerOptions[3].value.intValue0 = reportCheckpointIntermediates;
        sessionDesc.compilerOptionEntries = compilerOptions;
        sessionDesc.compilerOptionEntryCount = SLANG_COUNT_OF(compilerOptions);

        List<const char*> searchPaths;
        for (const String& searchPath : searchPathStorage)
            searchPaths.add(searchPath.getBuffer());
        sessionDesc.searchPathCount = searchPaths.getCount();
        sessionDesc.searchPaths = searchPaths.getBuffer();

        ComPtr<slang::ISession> newSession;
        SLANG_RETURN_ON_FAIL(
            m_impl->globalSession->createSession(sessionDesc, newSession.writeRef()));
        session = newSession;
        m_impl->sessions.add(std::move(sessionKeyString), std::move(newSession));
    }

    String moduleName = Path::getFileNameWithoutExt(sourcePath);
    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module(session->loadModuleFromSource(
        moduleName.getBuffer(),
        sourcePath,
        nullptr,
        diagnostics.writeRef()));
    appendDiagnostics(diagnostics, outDiagnostics);
    if (!module)
        return SLANG_FAIL;

    List<slang::IComponentType*> components;
    List<ComPtr<slang::IEntryPoint>> entries;
    components.add(module);
    SlangResult result = SLANG_OK;
    if (entryPointName)
    {
        ComPtr<slang::IEntryPoint> entryPoint;
        diagnostics = nullptr;
        result = module->findAndCheckEntryPoint(
            entryPointName,
            SLANG_STAGE_DISPATCH,
            entryPoint.writeRef(),
            diagnostics.writeRef());
        appendDiagnostics(diagnostics, outDiagnostics);
        if (SLANG_FAILED(result))
            return result;

        if (!slang_isHostEntryPoint(entryPoint))
        {
            outDiagnostics.append(
                "The selected entry point is not marked [host] and cannot use HGVM. "
                "Use -legacy to run it with the legacy interpreter.\n");
            return SLANG_FAIL;
        }

        entries.add(entryPoint);
        components.add(entryPoint);
    }
    else
    {
        for (SlangInt32 i = 0; i < module->getDefinedEntryPointCount(); ++i)
        {
            ComPtr<slang::IEntryPoint> entryPoint;
            SLANG_RETURN_ON_FAIL(module->getDefinedEntryPoint(i, entryPoint.writeRef()));
            if (!slang_isHostEntryPoint(entryPoint))
                continue;
            if (entryPoint->getSpecializationParamCount())
            {
                outDiagnostics = "Generic [host] entry points require explicit specialization.\n";
                return SLANG_E_NOT_IMPLEMENTED;
            }
            entries.add(entryPoint);
            components.add(entryPoint);
        }
        if (entries.getCount() == 0)
        {
            outDiagnostics = "The source module has no [host] entry points.\n";
            return SLANG_FAIL;
        }
    }

    ComPtr<slang::IComponentType> composite;
    diagnostics = nullptr;
    result = session->createCompositeComponentType(
        components.getBuffer(),
        components.getCount(),
        composite.writeRef(),
        diagnostics.writeRef());
    appendDiagnostics(diagnostics, outDiagnostics);
    if (SLANG_FAILED(result))
        return result;

    ComPtr<slang::IComponentType> linkedProgram;
    diagnostics = nullptr;
    result = composite->link(linkedProgram.writeRef(), diagnostics.writeRef());
    appendDiagnostics(diagnostics, outDiagnostics);
    if (SLANG_FAILED(result))
        return result;

    diagnostics = nullptr;
    result = linkedProgram->getTargetCode(0, outCode.writeRef(), diagnostics.writeRef());
    appendDiagnostics(diagnostics, outDiagnostics);
    return result;
}

SlangResult compileHGBCProgram(
    const char* sourcePath,
    const char* entryPointName,
    const char* const* additionalSearchPaths,
    SlangInt additionalSearchPathCount,
    SlangDebugInfoLevel debugInfoLevel,
    SlangDiagnosticColor diagnosticColor,
    ComPtr<slang::IBlob>& outCode,
    String& outDiagnostics,
    bool reportCheckpointIntermediates)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_RETURN_ON_FAIL(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()));
    RefPtr<HGBCCompiler> compiler = new HGBCCompiler(globalSession);
    return compiler->compile(
        sourcePath,
        entryPointName,
        additionalSearchPaths,
        additionalSearchPathCount,
        debugInfoLevel,
        diagnosticColor,
        outCode,
        outDiagnostics,
        reportCheckpointIntermediates);
}

} // namespace Slang::HGVM
