#ifndef SLANG_PROXY_COMPILE_REQUEST_H
#define SLANG_PROXY_COMPILE_REQUEST_H

#include "proxy-base.h"
#include "proxy-macros.h"

#include "slang-com-helper.h"
#include "slang-deprecated.h"
#include "slang.h"

namespace SlangRecord
{
using namespace Slang;

class CompileRequestProxy : public ProxyBase<slang::ICompileRequest>
{
public:
    SLANG_COM_INTERFACE(
        0xf7695fe5,
        0xcfe4,
        0xf5a6,
        {0x37, 0x28, 0x13, 0xe4, 0xd5, 0xc6, 0xb7, 0x48})

    explicit CompileRequestProxy(slang::ICompileRequest* actual)
        : ProxyBase(actual)
    {
    }

    // Record addRef/release for lifetime tracking during replay
    PROXY_REFCOUNT_IMPL(CompileRequestProxy)

    SLANG_NO_THROW SlangResult SLANG_MCALL
    queryInterface(SlangUUID const& uuid, void** outObject) SLANG_OVERRIDE
    {
        if (!outObject) return SLANG_E_INVALID_ARG;

        if (uuid == CompileRequestProxy::getTypeGuid() ||
            uuid == slang::ICompileRequest::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<slang::ICompileRequest*>(this);
            return SLANG_OK;
        }
        if (uuid == ISlangUnknown::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<ISlangUnknown*>(static_cast<slang::ICompileRequest*>(this));
            return SLANG_OK;
        }
        return m_actual->queryInterface(uuid, outObject);
    }

    // ICompileRequest
    virtual SLANG_NO_THROW void SLANG_MCALL setFileSystem(ISlangFileSystem* fileSystem) override
    {
        SLANG_UNUSED(fileSystem);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setFileSystem");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setCompileFlags(SlangCompileFlags flags) override
    {
        SLANG_UNUSED(flags);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setCompileFlags");
    }

    virtual SLANG_NO_THROW SlangCompileFlags SLANG_MCALL getCompileFlags() override
    {
        RECORD_CALL();
        return getActual<slang::ICompileRequest>()->getCompileFlags();
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setDumpIntermediates(int enable) override
    {
        SLANG_UNUSED(enable);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setDumpIntermediates");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setDumpIntermediatePrefix(const char* prefix) override
    {
        SLANG_UNUSED(prefix);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setDumpIntermediatePrefix");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setLineDirectiveMode(SlangLineDirectiveMode mode) override
    {
        SLANG_UNUSED(mode);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setLineDirectiveMode");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setCodeGenTarget(SlangCompileTarget target) override
    {
        SLANG_UNUSED(target);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setCodeGenTarget");
    }

    virtual SLANG_NO_THROW int SLANG_MCALL addCodeGenTarget(SlangCompileTarget target) override
    {
        RECORD_CALL();
        RECORD_INPUT(target);
        auto result = getActual<slang::ICompileRequest>()->addCodeGenTarget(target);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetProfile(int targetIndex, SlangProfileID profile) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(profile);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setTargetProfile");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetFlags(int targetIndex, SlangTargetFlags flags) override
    {
        RECORD_CALL();
        RECORD_INPUT(targetIndex);
        RECORD_INPUT(flags);
        getActual<slang::ICompileRequest>()->setTargetFlags(targetIndex, flags);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetFloatingPointMode(int targetIndex, SlangFloatingPointMode mode) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(mode);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setTargetFloatingPointMode");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetMatrixLayoutMode(int targetIndex, SlangMatrixLayoutMode mode) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(mode);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setTargetMatrixLayoutMode");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setMatrixLayoutMode(SlangMatrixLayoutMode mode) override
    {
        SLANG_UNUSED(mode);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setMatrixLayoutMode");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setDebugInfoLevel(SlangDebugInfoLevel level) override
    {
        RECORD_CALL();
        RECORD_INPUT(level);
        getActual<slang::ICompileRequest>()->setDebugInfoLevel(level);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setOptimizationLevel(SlangOptimizationLevel level) override
    {
        RECORD_CALL();
        RECORD_INPUT(level);
        getActual<slang::ICompileRequest>()->setOptimizationLevel(level);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setOutputContainerFormat(SlangContainerFormat format) override
    {
        SLANG_UNUSED(format);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setOutputContainerFormat");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setPassThrough(SlangPassThrough passThrough) override
    {
        SLANG_UNUSED(passThrough);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setPassThrough");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setDiagnosticCallback(SlangDiagnosticCallback callback, void const* userData) override
    {
        SLANG_UNUSED(callback);
        SLANG_UNUSED(userData);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setDiagnosticCallback");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setWriter(SlangWriterChannel channel, ISlangWriter* writer) override
    {
        RECORD_CALL();
        int32_t channelInt = static_cast<int32_t>(channel);
        RECORD_INPUT(channelInt);
        // Note: writer is a callback interface provided by client, not wrapped
        getActual<slang::ICompileRequest>()->setWriter(channel, writer);
    }

    virtual SLANG_NO_THROW ISlangWriter* SLANG_MCALL getWriter(SlangWriterChannel channel) override
    {
        SLANG_UNUSED(channel);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::getWriter");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL addSearchPath(const char* searchDir) override
    {
        RECORD_CALL();
        RECORD_INPUT(searchDir);
        getActual<slang::ICompileRequest>()->addSearchPath(searchDir);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    addPreprocessorDefine(const char* key, const char* value) override
    {
        SLANG_UNUSED(key);
        SLANG_UNUSED(value);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::addPreprocessorDefine");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    processCommandLineArguments(char const* const* args, int argCount) override
    {
        RECORD_CALL();
        RECORD_INPUT(argCount);
        RECORD_INPUT_ARRAY(args, argCount);
        auto result = getActual<slang::ICompileRequest>()->processCommandLineArguments(args, argCount);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW int SLANG_MCALL
    addTranslationUnit(SlangSourceLanguage language, char const* name) override
    {
        SLANG_UNUSED(language);
        SLANG_UNUSED(name);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::addTranslationUnit");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setDefaultModuleName(const char* defaultModuleName) override
    {
        SLANG_UNUSED(defaultModuleName);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setDefaultModuleName");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL addTranslationUnitPreprocessorDefine(
        int translationUnitIndex,
        const char* key,
        const char* value) override
    {
        SLANG_UNUSED(translationUnitIndex);
        SLANG_UNUSED(key);
        SLANG_UNUSED(value);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::addTranslationUnitPreprocessorDefine");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    addTranslationUnitSourceFile(int translationUnitIndex, char const* path) override
    {
        SLANG_UNUSED(translationUnitIndex);
        SLANG_UNUSED(path);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::addTranslationUnitSourceFile");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL addTranslationUnitSourceString(
        int translationUnitIndex,
        char const* path,
        char const* source) override
    {
        SLANG_UNUSED(translationUnitIndex);
        SLANG_UNUSED(path);
        SLANG_UNUSED(source);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::addTranslationUnitSourceString");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    addLibraryReference(const char* basePath, const void* libData, size_t libDataSize) override
    {
        SLANG_UNUSED(basePath);
        SLANG_UNUSED(libData);
        SLANG_UNUSED(libDataSize);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::addLibraryReference");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL addTranslationUnitSourceStringSpan(
        int translationUnitIndex,
        char const* path,
        char const* sourceBegin,
        char const* sourceEnd) override
    {
        SLANG_UNUSED(translationUnitIndex);
        SLANG_UNUSED(path);
        SLANG_UNUSED(sourceBegin);
        SLANG_UNUSED(sourceEnd);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::addTranslationUnitSourceStringSpan");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL addTranslationUnitSourceBlob(
        int translationUnitIndex,
        char const* path,
        ISlangBlob* sourceBlob) override
    {
        SLANG_UNUSED(translationUnitIndex);
        SLANG_UNUSED(path);
        SLANG_UNUSED(sourceBlob);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::addTranslationUnitSourceBlob");
    }

    virtual SLANG_NO_THROW int SLANG_MCALL
    addEntryPoint(int translationUnitIndex, char const* name, SlangStage stage) override
    {
        SLANG_UNUSED(translationUnitIndex);
        SLANG_UNUSED(name);
        SLANG_UNUSED(stage);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::addEntryPoint");
    }

    virtual SLANG_NO_THROW int SLANG_MCALL addEntryPointEx(
        int translationUnitIndex,
        char const* name,
        SlangStage stage,
        int genericArgCount,
        char const** genericArgs) override
    {
        SLANG_UNUSED(translationUnitIndex);
        SLANG_UNUSED(name);
        SLANG_UNUSED(stage);
        SLANG_UNUSED(genericArgCount);
        SLANG_UNUSED(genericArgs);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::addEntryPointEx");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    setGlobalGenericArgs(int genericArgCount, char const** genericArgs) override
    {
        SLANG_UNUSED(genericArgCount);
        SLANG_UNUSED(genericArgs);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setGlobalGenericArgs");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    setTypeNameForGlobalExistentialTypeParam(int slotIndex, char const* typeName) override
    {
        SLANG_UNUSED(slotIndex);
        SLANG_UNUSED(typeName);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setTypeNameForGlobalExistentialTypeParam");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL setTypeNameForEntryPointExistentialTypeParam(
        int entryPointIndex,
        int slotIndex,
        char const* typeName) override
    {
        SLANG_UNUSED(entryPointIndex);
        SLANG_UNUSED(slotIndex);
        SLANG_UNUSED(typeName);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setTypeNameForEntryPointExistentialTypeParam");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setAllowGLSLInput(bool value) override
    {
        SLANG_UNUSED(value);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setAllowGLSLInput");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL compile() override
    {
        RECORD_CALL();
        auto result = getActual<slang::ICompileRequest>()->compile();
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW char const* SLANG_MCALL getDiagnosticOutput() override
    {
        RECORD_CALL();
        auto result = getActual<slang::ICompileRequest>()->getDiagnosticOutput();
        RECORD_INFO(result);
        return result;
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getDiagnosticOutputBlob(ISlangBlob** outBlob) override
    {
        SLANG_UNUSED(outBlob);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::getDiagnosticOutputBlob");
    }

    virtual SLANG_NO_THROW int SLANG_MCALL getDependencyFileCount() override
    {
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::getDependencyFileCount");
    }

    virtual SLANG_NO_THROW char const* SLANG_MCALL getDependencyFilePath(int index) override
    {
        SLANG_UNUSED(index);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::getDependencyFilePath");
    }

    virtual SLANG_NO_THROW int SLANG_MCALL getTranslationUnitCount() override
    {
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::getTranslationUnitCount");
    }

    virtual SLANG_NO_THROW char const* SLANG_MCALL getEntryPointSource(int entryPointIndex) override
    {
        SLANG_UNUSED(entryPointIndex);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::getEntryPointSource");
    }

    virtual SLANG_NO_THROW void const* SLANG_MCALL
    getEntryPointCode(int entryPointIndex, size_t* outSize) override
    {
        SLANG_UNUSED(entryPointIndex);
        SLANG_UNUSED(outSize);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::getEntryPointCode");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getEntryPointCodeBlob(int entryPointIndex, int targetIndex, ISlangBlob** outBlob) override
    {
        SLANG_UNUSED(entryPointIndex);
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outBlob);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::getEntryPointCodeBlob");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointHostCallable(
        int entryPointIndex,
        int targetIndex,
        ISlangSharedLibrary** outSharedLibrary) override
    {
        SLANG_UNUSED(entryPointIndex);
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outSharedLibrary);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::getEntryPointHostCallable");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getTargetCodeBlob(int targetIndex, ISlangBlob** outBlob) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outBlob);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::getTargetCodeBlob");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getTargetHostCallable(int targetIndex, ISlangSharedLibrary** outSharedLibrary) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outSharedLibrary);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::getTargetHostCallable");
    }

    virtual SLANG_NO_THROW void const* SLANG_MCALL getCompileRequestCode(size_t* outSize) override
    {
        SLANG_UNUSED(outSize);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::getCompileRequestCode");
    }

    virtual SLANG_NO_THROW ISlangMutableFileSystem* SLANG_MCALL
    getCompileRequestResultAsFileSystem() override
    {
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::getCompileRequestResultAsFileSystem");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getContainerCode(ISlangBlob** outBlob) override
    {
        SLANG_UNUSED(outBlob);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::getContainerCode");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    loadRepro(ISlangFileSystem* fileSystem, const void* data, size_t size) override
    {
        SLANG_UNUSED(fileSystem);
        SLANG_UNUSED(data);
        SLANG_UNUSED(size);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::loadRepro");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL saveRepro(ISlangBlob** outBlob) override
    {
        SLANG_UNUSED(outBlob);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::saveRepro");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL enableReproCapture() override
    {
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::enableReproCapture");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getProgram(slang::IComponentType** outProgram) override
    {
        SLANG_UNUSED(outProgram);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::getProgram");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getEntryPoint(SlangInt entryPointIndex, slang::IComponentType** outEntryPoint) override
    {
        SLANG_UNUSED(entryPointIndex);
        SLANG_UNUSED(outEntryPoint);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::getEntryPoint");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getModule(SlangInt translationUnitIndex, slang::IModule** outModule) override
    {
        SLANG_UNUSED(translationUnitIndex);
        SLANG_UNUSED(outModule);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::getModule");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getSession(slang::ISession** outSession) override
    {
        SLANG_UNUSED(outSession);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::getSession");
    }

    virtual SLANG_NO_THROW SlangReflection* SLANG_MCALL getReflection() override
    {
        RECORD_CALL();
        return getActual<slang::ICompileRequest>()->getReflection();
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setCommandLineCompilerMode() override
    {
        RECORD_CALL();
        getActual<slang::ICompileRequest>()->setCommandLineCompilerMode();
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    addTargetCapability(SlangInt targetIndex, SlangCapabilityID capability) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(capability);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::addTargetCapability");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getProgramWithEntryPoints(slang::IComponentType** outProgram) override
    {
        SLANG_UNUSED(outProgram);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::getProgramWithEntryPoints");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL isParameterLocationUsed(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        SlangParameterCategory category,
        SlangUInt spaceIndex,
        SlangUInt registerIndex,
        bool& outUsed) override
    {
        RECORD_CALL();
        RECORD_INPUT(entryPointIndex);
        RECORD_INPUT(targetIndex);
        RECORD_INPUT(category);
        RECORD_INPUT(spaceIndex);
        RECORD_INPUT(registerIndex);
        auto result = getActual<slang::ICompileRequest>()->isParameterLocationUsed(
            entryPointIndex, targetIndex, category, spaceIndex, registerIndex, outUsed);
        RECORD_INFO(outUsed);
        RECORD_RETURN(result);
        return result;
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetLineDirectiveMode(SlangInt targetIndex, SlangLineDirectiveMode mode) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(mode);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setTargetLineDirectiveMode");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetForceGLSLScalarBufferLayout(int targetIndex, bool forceScalarLayout) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(forceScalarLayout);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setTargetForceGLSLScalarBufferLayout");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    overrideDiagnosticSeverity(SlangInt messageID, SlangSeverity overrideSeverity) override
    {
        SLANG_UNUSED(messageID);
        SLANG_UNUSED(overrideSeverity);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::overrideDiagnosticSeverity");
    }

    virtual SLANG_NO_THROW SlangDiagnosticFlags SLANG_MCALL getDiagnosticFlags() override
    {
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::getDiagnosticFlags");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setDiagnosticFlags(SlangDiagnosticFlags flags) override
    {
        SLANG_UNUSED(flags);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setDiagnosticFlags");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setDebugInfoFormat(SlangDebugInfoFormat debugFormat) override
    {
        SLANG_UNUSED(debugFormat);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setDebugInfoFormat");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setEnableEffectAnnotations(bool value) override
    {
        SLANG_UNUSED(value);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setEnableEffectAnnotations");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setReportDownstreamTime(bool value) override
    {
        SLANG_UNUSED(value);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setReportDownstreamTime");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setReportPerfBenchmark(bool value) override
    {
        SLANG_UNUSED(value);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setReportPerfBenchmark");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setSkipSPIRVValidation(bool value) override
    {
        SLANG_UNUSED(value);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setSkipSPIRVValidation");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetUseMinimumSlangOptimization(int targetIndex, bool value) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(value);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setTargetUseMinimumSlangOptimization");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setIgnoreCapabilityCheck(bool value) override
    {
        SLANG_UNUSED(value);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setIgnoreCapabilityCheck");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getCompileTimeProfile(ISlangProfiler** compileTimeProfile, bool shouldClear) override
    {
        SLANG_UNUSED(compileTimeProfile);
        SLANG_UNUSED(shouldClear);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::getCompileTimeProfile");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetGenerateWholeProgram(int targetIndex, bool value) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(value);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setTargetGenerateWholeProgram");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetForceDXLayout(int targetIndex, bool value) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(value);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setTargetForceDXLayout");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetEmbedDownstreamIR(int targetIndex, bool value) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(value);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setTargetEmbedDownstreamIR");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetForceCLayout(int targetIndex, bool value) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(value);
        REPLAY_UNIMPLEMENTED_X("CompileRequestProxy::setTargetForceCLayout");
    }
};

} // namespace SlangRecord

#endif // SLANG_PROXY_COMPILE_REQUEST_H
