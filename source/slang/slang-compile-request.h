// slang-compile-request.h
#pragma once

//
// This file contains the `FrontEndCompileRequest` type
// and the types that it is built from (such as
// `TranslationUnitRequest`). These types are used
// whenever the Slang front-end is invoked to compile
// a module (or, in some cases, one or more modules)
// from source code to a checked AST and Slang IR.
//
// Note that the `EndToEndCompileRequest` type has its
// own header: `slang-end-to-end-request.h`.
//

#include "../compiler-core/slang-artifact.h"
#include "../compiler-core/slang-source-loc.h"
#include "../core/slang-smart-pointer.h"
#include "../core/slang-std-writers.h"
#include "slang-compiler-fwd.h"
#include "slang-diagnostics.h"
#include "slang-module.h"
#include "slang-preprocessor.h"
#include "slang-profile.h"
#include "slang-session.h"
#include "slang-translation-unit.h"

namespace Slang
{

/// Shared functionality between front- and back-end compile requests.
///
/// This is the base class for both `FrontEndCompileRequest` and
/// `BackEndCompileRequest`, and allows a small number of parts of
/// the compiler to be easily invocable from either front-end or
/// back-end work.
///
class CompileRequestBase : public RefObject
{
    // TODO: We really shouldn't need this type in the long run.
    // The few places that rely on it should be refactored to just
    // depend on the underlying information (a linkage and a diagnostic
    // sink) directly.
    //
    // The flags to control dumping and validation of IR should be
    // moved to some kind of shared settings/options `struct` that
    // both front-end and back-end requests can store.

public:
    Session* getSession();
    Linkage* getLinkage() { return m_linkage; }
    DiagnosticSink* getSink() { return m_sink; }
    SourceManager* getSourceManager() { return getLinkage()->getSourceManager(); }
    NamePool* getNamePool() { return getLinkage()->getNamePool(); }
    ISlangFileSystemExt* getFileSystemExt() { return getLinkage()->getFileSystemExt(); }
    SlangResult loadFile(String const& path, PathInfo& outPathInfo, ISlangBlob** outBlob)
    {
        return getLinkage()->loadFile(path, outPathInfo, outBlob);
    }

protected:
    CompileRequestBase(Linkage* linkage, DiagnosticSink* sink);

private:
    Linkage* m_linkage = nullptr;
    DiagnosticSink* m_sink = nullptr;
};

/// A request for the front-end to find and validate an entry-point function
struct FrontEndEntryPointRequest : RefObject
{
public:
    /// Create a request for an entry point.
    FrontEndEntryPointRequest(
        FrontEndCompileRequest* compileRequest,
        int translationUnitIndex,
        Name* name,
        Profile profile);

    /// Get the parent front-end compile request.
    FrontEndCompileRequest* getCompileRequest() { return m_compileRequest; }

    /// Get the translation unit that contains the entry point.
    TranslationUnitRequest* getTranslationUnit();

    /// Get the name of the entry point to find.
    Name* getName() { return m_name; }

    /// Get the stage that the entry point is to be compiled for
    Stage getStage() { return m_profile.getStage(); }

    /// Get the profile that the entry point is to be compiled for
    Profile getProfile() { return m_profile; }

    /// Get the index to the translation unit
    int getTranslationUnitIndex() const { return m_translationUnitIndex; }

private:
    // The parent compile request
    FrontEndCompileRequest* m_compileRequest;

    // The index of the translation unit that will hold the entry point
    int m_translationUnitIndex;

    // The name of the entry point function to look for
    Name* m_name;

    // The profile to compile for (including stage)
    Profile m_profile;
};

/// A request to compile source code to an AST + IR.
class FrontEndCompileRequest : public CompileRequestBase
{
public:
    /// Note that writers can be parsed as nullptr to disable output,
    /// and individual channels set to null to disable them
    FrontEndCompileRequest(Linkage* linkage, StdWriters* writers, DiagnosticSink* sink);

    int addEntryPoint(int translationUnitIndex, String const& name, Profile entryPointProfile);

    // Translation units we are being asked to compile
    List<RefPtr<TranslationUnitRequest>> translationUnits;

    // Additional modules that needs to be made visible to `import` while checking.
    const LoadedModuleDictionary* additionalLoadedModules = nullptr;

    RefPtr<TranslationUnitRequest> getTranslationUnit(UInt index)
    {
        return translationUnits[index];
    }

    // If true will serialize and de-serialize with debug information
    bool verifyDebugSerialization = false;

    CompilerOptionSet optionSet;

    List<RefPtr<FrontEndEntryPointRequest>> m_entryPointReqs;

    List<RefPtr<FrontEndEntryPointRequest>> const& getEntryPointReqs() { return m_entryPointReqs; }
    UInt getEntryPointReqCount() { return m_entryPointReqs.getCount(); }
    FrontEndEntryPointRequest* getEntryPointReq(UInt index) { return m_entryPointReqs[index]; }

    void parseTranslationUnit(TranslationUnitRequest* translationUnit);

    // Perform primary semantic checking on all
    // of the translation units in the program
    void checkAllTranslationUnits();

    void checkEntryPoints();

    void generateIR();

    SlangResult executeActionsInner();

    /// Add a translation unit to be compiled.
    ///
    /// @param language The source language that the translation unit will use (e.g.,
    /// `SourceLanguage::Slang`
    /// @param moduleName The name that will be used for the module compile from the translation
    /// unit.
    ///
    /// If moduleName is passed as nullptr a module name is generated.
    /// If all translation units in a compile request use automatically generated
    /// module names, then they are guaranteed not to conflict with one another.
    ///
    /// @return The zero-based index of the translation unit in this compile request.
    int addTranslationUnit(SourceLanguage language, Name* moduleName);

    int addTranslationUnit(TranslationUnitRequest* translationUnit);

    void addTranslationUnitSourceArtifact(int translationUnitIndex, IArtifact* sourceArtifact);

    void addTranslationUnitSourceBlob(
        int translationUnitIndex,
        String const& path,
        ISlangBlob* sourceBlob);

    void addTranslationUnitSourceFile(int translationUnitIndex, String const& path);

    /// Get a component type that represents the global scope of the compile request.
    ComponentType* getGlobalComponentType() { return m_globalComponentType; }

    /// Get a component type that represents the global scope of the compile request, plus the
    /// requested entry points.
    ComponentType* getGlobalAndEntryPointsComponentType()
    {
        return m_globalAndEntryPointsComponentType;
    }

    List<RefPtr<ComponentType>> const& getUnspecializedEntryPoints()
    {
        return m_unspecializedEntryPoints;
    }

    /// Does the code we are compiling represent part of the Slang core module?
    bool m_isCoreModuleCode = false;

    Name* m_defaultModuleName = nullptr;

    /// The irDumpOptions
    IRDumpOptions m_irDumpOptions;

    /// An "extra" entry point that was added via a library reference
    struct ExtraEntryPointInfo
    {
        Name* name;
        Profile profile;
        String mangledName;
    };

    /// A list of "extra" entry points added via a library reference
    List<ExtraEntryPointInfo> m_extraEntryPoints;

private:
    /// A component type that includes only the global scopes of the translation unit(s) that were
    /// compiled.
    RefPtr<ComponentType> m_globalComponentType;

    /// A component type that extends the global scopes with all of the entry points that were
    /// specified.
    RefPtr<ComponentType> m_globalAndEntryPointsComponentType;

    List<RefPtr<ComponentType>> m_unspecializedEntryPoints;

    RefPtr<StdWriters> m_writers;
};

/// Handlers for preprocessor callbacks to use when doing ordinary front-end compilation
struct FrontEndPreprocessorHandler : PreprocessorHandler
{
public:
    FrontEndPreprocessorHandler(
        Module* module,
        ASTBuilder* astBuilder,
        DiagnosticSink* sink,
        TranslationUnitRequest* translationUnit)
        : m_module(module)
        , m_astBuilder(astBuilder)
        , m_sink(sink)
        , m_translationUnit(translationUnit)
    {
    }

protected:
    Module* m_module;
    ASTBuilder* m_astBuilder;
    DiagnosticSink* m_sink;
    TranslationUnitRequest* m_translationUnit = nullptr;

    // The first task that this handler tries to deal with is
    // capturing all the files on which a module is dependent.
    //
    // That information is exposed through public APIs and used
    // by applications to decide when they need to "hot reload"
    // their shader code.
    //
    void handleFileDependency(SourceFile* sourceFile) SLANG_OVERRIDE
    {
        m_module->addFileDependency(sourceFile);
        m_translationUnit->addIncludedSourceFileIfNotExist(sourceFile);
    }

    // The second task that this handler deals with is detecting
    // whether any macro values were set in a given source file
    // that are semantically relevant to other stages of compilation.
    //
    void handleEndOfTranslationUnit(Preprocessor* preprocessor) SLANG_OVERRIDE
    {
        // We look at the preprocessor state after reading the entire
        // source file/string, in order to see if any macros have been
        // set that should be considered semantically relevant for
        // later stages of compilation.
        //
        // Note: Checking the macro environment *after* preprocessing is complete
        // means that we can treat macros introduced via `-D` options or the API
        // equivalently to macros introduced via `#define`s in user code.
        //
        // For now, the only case of semantically-relevant macros we need to worrry
        // about are the NVAPI macros used to establish the register/space to use.
        //
        static const char* kNVAPIRegisterMacroName = "NV_SHADER_EXTN_SLOT";
        static const char* kNVAPISpaceMacroName = "NV_SHADER_EXTN_REGISTER_SPACE";

        // For NVAPI use, the `NV_SHADER_EXTN_SLOT` macro is required to be defined.
        //
        String nvapiRegister;
        SourceLoc nvapiRegisterLoc;
        if (!SLANG_FAILED(findMacroValue(
                preprocessor,
                kNVAPIRegisterMacroName,
                nvapiRegister,
                nvapiRegisterLoc)))
        {
            // In contrast, NVAPI can be used without defining `NV_SHADER_EXTN_REGISTER_SPACE`,
            // which effectively defaults to `space0`.
            //
            String nvapiSpace = "space0";
            SourceLoc nvapiSpaceLoc;
            findMacroValue(preprocessor, kNVAPISpaceMacroName, nvapiSpace, nvapiSpaceLoc);

            // We are going to store the values of these macros on the AST-level `ModuleDecl`
            // so that they will be available to later processing stages.
            //
            auto moduleDecl = m_module->getModuleDecl();

            if (auto existingModifier = moduleDecl->findModifier<NVAPISlotModifier>())
            {
                // If there is already a modifier attached to the module (perhaps
                // because of preprocessing a different source file, or because
                // of settings established via command-line options), then we
                // need to validate that the values being set in this file
                // match those already set (or else there is likely to be
                // some kind of error in the user's code).
                //
                _validateNVAPIMacroMatch(
                    kNVAPIRegisterMacroName,
                    existingModifier->registerName,
                    nvapiRegister,
                    nvapiRegisterLoc);
                _validateNVAPIMacroMatch(
                    kNVAPISpaceMacroName,
                    existingModifier->spaceName,
                    nvapiSpace,
                    nvapiSpaceLoc);
            }
            else
            {
                // If there is no existing modifier on the module, then we
                // take responsibility for adding one, based on the macro
                // values we saw.
                //
                auto modifier = m_astBuilder->create<NVAPISlotModifier>();
                modifier->loc = nvapiRegisterLoc;
                modifier->registerName = nvapiRegister;
                modifier->spaceName = nvapiSpace;

                addModifier(moduleDecl, modifier);
            }
        }
    }

    /// Validate that a re-defintion of an NVAPI-related macro matches any previous definition
    void _validateNVAPIMacroMatch(
        char const* macroName,
        String const& existingValue,
        String const& newValue,
        SourceLoc loc)
    {
        if (existingValue != newValue)
        {
            m_sink->diagnose(
                loc,
                Diagnostics::nvapiMacroMismatch,
                macroName,
                existingValue,
                newValue);
        }
    }
};

} // namespace Slang
