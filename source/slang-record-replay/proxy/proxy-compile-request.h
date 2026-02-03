#ifndef SLANG_PROXY_COMPILE_REQUEST_H
#define SLANG_PROXY_COMPILE_REQUEST_H

#include "proxy-base.h"

#include "../../core/slang-smart-pointer.h"
#include "slang-com-helper.h"
#include "slang-deprecated.h"
#include "slang.h"

namespace SlangRecord
{
using namespace Slang;

class CompileRequestProxy : public slang::ICompileRequest,
                            public RefObject,
                            public ProxyBase
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

    SLANG_REF_OBJECT_IUNKNOWN_ALL
    ISlangUnknown* getInterface(const Guid& guid);

    // ICompileRequest
    virtual SLANG_NO_THROW void SLANG_MCALL setFileSystem(ISlangFileSystem* fileSystem) override
    {
        SLANG_UNUSED(fileSystem);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setFileSystem");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setCompileFlags(SlangCompileFlags flags) override
    {
        SLANG_UNUSED(flags);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setCompileFlags");
    }

    virtual SLANG_NO_THROW SlangCompileFlags SLANG_MCALL getCompileFlags() override
    {
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::getCompileFlags");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setDumpIntermediates(int enable) override
    {
        SLANG_UNUSED(enable);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setDumpIntermediates");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setDumpIntermediatePrefix(const char* prefix) override
    {
        SLANG_UNUSED(prefix);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setDumpIntermediatePrefix");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setLineDirectiveMode(SlangLineDirectiveMode mode) override
    {
        SLANG_UNUSED(mode);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setLineDirectiveMode");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setCodeGenTarget(SlangCompileTarget target) override
    {
        SLANG_UNUSED(target);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setCodeGenTarget");
    }

    virtual SLANG_NO_THROW int SLANG_MCALL addCodeGenTarget(SlangCompileTarget target) override
    {
        SLANG_UNUSED(target);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::addCodeGenTarget");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetProfile(int targetIndex, SlangProfileID profile) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(profile);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setTargetProfile");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetFlags(int targetIndex, SlangTargetFlags flags) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(flags);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setTargetFlags");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetFloatingPointMode(int targetIndex, SlangFloatingPointMode mode) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(mode);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setTargetFloatingPointMode");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetMatrixLayoutMode(int targetIndex, SlangMatrixLayoutMode mode) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(mode);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setTargetMatrixLayoutMode");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setMatrixLayoutMode(SlangMatrixLayoutMode mode) override
    {
        SLANG_UNUSED(mode);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setMatrixLayoutMode");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setDebugInfoLevel(SlangDebugInfoLevel level) override
    {
        SLANG_UNUSED(level);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setDebugInfoLevel");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setOptimizationLevel(SlangOptimizationLevel level) override
    {
        SLANG_UNUSED(level);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setOptimizationLevel");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setOutputContainerFormat(SlangContainerFormat format) override
    {
        SLANG_UNUSED(format);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setOutputContainerFormat");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setPassThrough(SlangPassThrough passThrough) override
    {
        SLANG_UNUSED(passThrough);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setPassThrough");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setDiagnosticCallback(SlangDiagnosticCallback callback, void const* userData) override
    {
        SLANG_UNUSED(callback);
        SLANG_UNUSED(userData);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setDiagnosticCallback");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setWriter(SlangWriterChannel channel, ISlangWriter* writer) override
    {
        SLANG_UNUSED(channel);
        SLANG_UNUSED(writer);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setWriter");
    }

    virtual SLANG_NO_THROW ISlangWriter* SLANG_MCALL getWriter(SlangWriterChannel channel) override
    {
        SLANG_UNUSED(channel);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::getWriter");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL addSearchPath(const char* searchDir) override
    {
        SLANG_UNUSED(searchDir);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::addSearchPath");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    addPreprocessorDefine(const char* key, const char* value) override
    {
        SLANG_UNUSED(key);
        SLANG_UNUSED(value);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::addPreprocessorDefine");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    processCommandLineArguments(char const* const* args, int argCount) override
    {
        SLANG_UNUSED(args);
        SLANG_UNUSED(argCount);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::processCommandLineArguments");
    }

    virtual SLANG_NO_THROW int SLANG_MCALL
    addTranslationUnit(SlangSourceLanguage language, char const* name) override
    {
        SLANG_UNUSED(language);
        SLANG_UNUSED(name);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::addTranslationUnit");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setDefaultModuleName(const char* defaultModuleName) override
    {
        SLANG_UNUSED(defaultModuleName);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setDefaultModuleName");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL addTranslationUnitPreprocessorDefine(
        int translationUnitIndex,
        const char* key,
        const char* value) override
    {
        SLANG_UNUSED(translationUnitIndex);
        SLANG_UNUSED(key);
        SLANG_UNUSED(value);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::addTranslationUnitPreprocessorDefine");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    addTranslationUnitSourceFile(int translationUnitIndex, char const* path) override
    {
        SLANG_UNUSED(translationUnitIndex);
        SLANG_UNUSED(path);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::addTranslationUnitSourceFile");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL addTranslationUnitSourceString(
        int translationUnitIndex,
        char const* path,
        char const* source) override
    {
        SLANG_UNUSED(translationUnitIndex);
        SLANG_UNUSED(path);
        SLANG_UNUSED(source);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::addTranslationUnitSourceString");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    addLibraryReference(const char* basePath, const void* libData, size_t libDataSize) override
    {
        SLANG_UNUSED(basePath);
        SLANG_UNUSED(libData);
        SLANG_UNUSED(libDataSize);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::addLibraryReference");
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
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::addTranslationUnitSourceStringSpan");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL addTranslationUnitSourceBlob(
        int translationUnitIndex,
        char const* path,
        ISlangBlob* sourceBlob) override
    {
        SLANG_UNUSED(translationUnitIndex);
        SLANG_UNUSED(path);
        SLANG_UNUSED(sourceBlob);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::addTranslationUnitSourceBlob");
    }

    virtual SLANG_NO_THROW int SLANG_MCALL
    addEntryPoint(int translationUnitIndex, char const* name, SlangStage stage) override
    {
        SLANG_UNUSED(translationUnitIndex);
        SLANG_UNUSED(name);
        SLANG_UNUSED(stage);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::addEntryPoint");
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
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::addEntryPointEx");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    setGlobalGenericArgs(int genericArgCount, char const** genericArgs) override
    {
        SLANG_UNUSED(genericArgCount);
        SLANG_UNUSED(genericArgs);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setGlobalGenericArgs");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    setTypeNameForGlobalExistentialTypeParam(int slotIndex, char const* typeName) override
    {
        SLANG_UNUSED(slotIndex);
        SLANG_UNUSED(typeName);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setTypeNameForGlobalExistentialTypeParam");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL setTypeNameForEntryPointExistentialTypeParam(
        int entryPointIndex,
        int slotIndex,
        char const* typeName) override
    {
        SLANG_UNUSED(entryPointIndex);
        SLANG_UNUSED(slotIndex);
        SLANG_UNUSED(typeName);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setTypeNameForEntryPointExistentialTypeParam");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setAllowGLSLInput(bool value) override
    {
        SLANG_UNUSED(value);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setAllowGLSLInput");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL compile() override
    {
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::compile");
    }

    virtual SLANG_NO_THROW char const* SLANG_MCALL getDiagnosticOutput() override
    {
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::getDiagnosticOutput");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getDiagnosticOutputBlob(ISlangBlob** outBlob) override
    {
        SLANG_UNUSED(outBlob);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::getDiagnosticOutputBlob");
    }

    virtual SLANG_NO_THROW int SLANG_MCALL getDependencyFileCount() override
    {
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::getDependencyFileCount");
    }

    virtual SLANG_NO_THROW char const* SLANG_MCALL getDependencyFilePath(int index) override
    {
        SLANG_UNUSED(index);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::getDependencyFilePath");
    }

    virtual SLANG_NO_THROW int SLANG_MCALL getTranslationUnitCount() override
    {
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::getTranslationUnitCount");
    }

    virtual SLANG_NO_THROW char const* SLANG_MCALL getEntryPointSource(int entryPointIndex) override
    {
        SLANG_UNUSED(entryPointIndex);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::getEntryPointSource");
    }

    virtual SLANG_NO_THROW void const* SLANG_MCALL
    getEntryPointCode(int entryPointIndex, size_t* outSize) override
    {
        SLANG_UNUSED(entryPointIndex);
        SLANG_UNUSED(outSize);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::getEntryPointCode");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getEntryPointCodeBlob(int entryPointIndex, int targetIndex, ISlangBlob** outBlob) override
    {
        SLANG_UNUSED(entryPointIndex);
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outBlob);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::getEntryPointCodeBlob");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointHostCallable(
        int entryPointIndex,
        int targetIndex,
        ISlangSharedLibrary** outSharedLibrary) override
    {
        SLANG_UNUSED(entryPointIndex);
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outSharedLibrary);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::getEntryPointHostCallable");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getTargetCodeBlob(int targetIndex, ISlangBlob** outBlob) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outBlob);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::getTargetCodeBlob");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getTargetHostCallable(int targetIndex, ISlangSharedLibrary** outSharedLibrary) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outSharedLibrary);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::getTargetHostCallable");
    }

    virtual SLANG_NO_THROW void const* SLANG_MCALL getCompileRequestCode(size_t* outSize) override
    {
        SLANG_UNUSED(outSize);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::getCompileRequestCode");
    }

    virtual SLANG_NO_THROW ISlangMutableFileSystem* SLANG_MCALL
    getCompileRequestResultAsFileSystem() override
    {
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::getCompileRequestResultAsFileSystem");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getContainerCode(ISlangBlob** outBlob) override
    {
        SLANG_UNUSED(outBlob);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::getContainerCode");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    loadRepro(ISlangFileSystem* fileSystem, const void* data, size_t size) override
    {
        SLANG_UNUSED(fileSystem);
        SLANG_UNUSED(data);
        SLANG_UNUSED(size);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::loadRepro");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL saveRepro(ISlangBlob** outBlob) override
    {
        SLANG_UNUSED(outBlob);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::saveRepro");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL enableReproCapture() override
    {
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::enableReproCapture");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getProgram(slang::IComponentType** outProgram) override
    {
        SLANG_UNUSED(outProgram);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::getProgram");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getEntryPoint(SlangInt entryPointIndex, slang::IComponentType** outEntryPoint) override
    {
        SLANG_UNUSED(entryPointIndex);
        SLANG_UNUSED(outEntryPoint);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::getEntryPoint");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getModule(SlangInt translationUnitIndex, slang::IModule** outModule) override
    {
        SLANG_UNUSED(translationUnitIndex);
        SLANG_UNUSED(outModule);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::getModule");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getSession(slang::ISession** outSession) override
    {
        SLANG_UNUSED(outSession);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::getSession");
    }

    virtual SLANG_NO_THROW SlangReflection* SLANG_MCALL getReflection() override
    {
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::getReflection");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setCommandLineCompilerMode() override
    {
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setCommandLineCompilerMode");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    addTargetCapability(SlangInt targetIndex, SlangCapabilityID capability) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(capability);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::addTargetCapability");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getProgramWithEntryPoints(slang::IComponentType** outProgram) override
    {
        SLANG_UNUSED(outProgram);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::getProgramWithEntryPoints");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL isParameterLocationUsed(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        SlangParameterCategory category,
        SlangUInt spaceIndex,
        SlangUInt registerIndex,
        bool& outUsed) override
    {
        SLANG_UNUSED(entryPointIndex);
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(category);
        SLANG_UNUSED(spaceIndex);
        SLANG_UNUSED(registerIndex);
        SLANG_UNUSED(outUsed);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::isParameterLocationUsed");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetLineDirectiveMode(SlangInt targetIndex, SlangLineDirectiveMode mode) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(mode);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setTargetLineDirectiveMode");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetForceGLSLScalarBufferLayout(int targetIndex, bool forceScalarLayout) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(forceScalarLayout);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setTargetForceGLSLScalarBufferLayout");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    overrideDiagnosticSeverity(SlangInt messageID, SlangSeverity overrideSeverity) override
    {
        SLANG_UNUSED(messageID);
        SLANG_UNUSED(overrideSeverity);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::overrideDiagnosticSeverity");
    }

    virtual SLANG_NO_THROW SlangDiagnosticFlags SLANG_MCALL getDiagnosticFlags() override
    {
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::getDiagnosticFlags");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setDiagnosticFlags(SlangDiagnosticFlags flags) override
    {
        SLANG_UNUSED(flags);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setDiagnosticFlags");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setDebugInfoFormat(SlangDebugInfoFormat debugFormat) override
    {
        SLANG_UNUSED(debugFormat);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setDebugInfoFormat");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setEnableEffectAnnotations(bool value) override
    {
        SLANG_UNUSED(value);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setEnableEffectAnnotations");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setReportDownstreamTime(bool value) override
    {
        SLANG_UNUSED(value);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setReportDownstreamTime");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setReportPerfBenchmark(bool value) override
    {
        SLANG_UNUSED(value);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setReportPerfBenchmark");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setSkipSPIRVValidation(bool value) override
    {
        SLANG_UNUSED(value);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setSkipSPIRVValidation");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetUseMinimumSlangOptimization(int targetIndex, bool value) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(value);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setTargetUseMinimumSlangOptimization");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setIgnoreCapabilityCheck(bool value) override
    {
        SLANG_UNUSED(value);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setIgnoreCapabilityCheck");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getCompileTimeProfile(ISlangProfiler** compileTimeProfile, bool shouldClear) override
    {
        SLANG_UNUSED(compileTimeProfile);
        SLANG_UNUSED(shouldClear);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::getCompileTimeProfile");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetGenerateWholeProgram(int targetIndex, bool value) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(value);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setTargetGenerateWholeProgram");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetForceDXLayout(int targetIndex, bool value) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(value);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setTargetForceDXLayout");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetEmbedDownstreamIR(int targetIndex, bool value) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(value);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setTargetEmbedDownstreamIR");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetForceCLayout(int targetIndex, bool value) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(value);
        SLANG_UNIMPLEMENTED_X("CompileRequestProxy::setTargetForceCLayout");
    }
};

} // namespace SlangRecord

#endif // SLANG_PROXY_COMPILE_REQUEST_H
