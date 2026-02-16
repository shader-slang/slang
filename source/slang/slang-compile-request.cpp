// slang-compile-request.cpp
#include "slang-compile-request.h"

#include "../core/slang-performance-profiler.h"
#include "compiler-core/slang-artifact-desc-util.h"
#include "compiler-core/slang-artifact-util.h"
#include "slang-ast-dump.h"
#include "slang-check-impl.h"
#include "slang-compiler.h"
#include "slang-emit-source-writer.h"
#include "slang-lower-to-ir.h"
#include "slang-parser.h"
#include "slang-serialize-container.h"

namespace Slang
{
//
// FrontEndEntryPointRequest
//

FrontEndEntryPointRequest::FrontEndEntryPointRequest(
    FrontEndCompileRequest* compileRequest,
    int translationUnitIndex,
    Name* name,
    Profile profile)
    : m_compileRequest(compileRequest)
    , m_translationUnitIndex(translationUnitIndex)
    , m_name(name)
    , m_profile(profile)
{
}

TranslationUnitRequest* FrontEndEntryPointRequest::getTranslationUnit()
{
    return getCompileRequest()->translationUnits[m_translationUnitIndex];
}

//
// CompileRequestBase
//

CompileRequestBase::CompileRequestBase(Linkage* linkage, DiagnosticSink* sink)
    : m_linkage(linkage), m_sink(sink)
{
}

Session* CompileRequestBase::getSession()
{
    return getLinkage()->getSessionImpl();
}

//
// FrontEndCompileRequest
//

FrontEndCompileRequest::FrontEndCompileRequest(
    Linkage* linkage,
    StdWriters* writers,
    DiagnosticSink* sink)
    : CompileRequestBase(linkage, sink), m_writers(writers)
{
    optionSet.inheritFrom(linkage->m_optionSet);
}

// Holds the hierarchy of views, the children being views that were 'initiated' (have an initiating
// SourceLoc) in the parent.
typedef Dictionary<SourceView*, List<SourceView*>> ViewInitiatingHierarchy;

// Calculate the hierarchy from the sourceManager
static void _calcViewInitiatingHierarchy(
    SourceManager* sourceManager,
    ViewInitiatingHierarchy& outHierarchy)
{
    const List<SourceView*> emptyList;
    outHierarchy.clear();

    // Iterate over all managers
    for (SourceManager* curManager = sourceManager; curManager;
         curManager = curManager->getParent())
    {
        // Iterate over all views
        for (SourceView* view : curManager->getSourceViews())
        {
            if (view->getInitiatingSourceLoc().isValid())
            {
                // Look up the view it came from
                SourceView* parentView =
                    sourceManager->findSourceViewRecursively(view->getInitiatingSourceLoc());
                if (parentView)
                {
                    List<SourceView*>& children = outHierarchy.getOrAddValue(parentView, emptyList);
                    // It shouldn't have already been added
                    SLANG_ASSERT(children.indexOf(view) < 0);
                    children.add(view);
                }
            }
        }
    }

    // Order all the children, by their raw SourceLocs. This is desirable, so that a trivial
    // traversal will traverse children in the order they are initiated in the parent source. This
    // assumes they increase in SourceLoc implies an later within a source file - this is true
    // currently.
    for (auto& [_, value] : outHierarchy)
    {
        value.sort(
            [](SourceView* a, SourceView* b) -> bool {
                return a->getInitiatingSourceLoc().getRaw() < b->getInitiatingSourceLoc().getRaw();
            });
    }
}

// Given a source file, find the view that is the initial SourceView use of the source. It must have
// an initiating SourceLoc that is not valid.
static SourceView* _findInitialSourceView(SourceFile* sourceFile)
{
    // TODO(JS):
    // This might be overkill - presumably the SourceView would belong to the same manager as it's
    // SourceFile? That is not enforced by the SourceManager in any way though so we just search all
    // managers, and all views.
    for (SourceManager* sourceManager = sourceFile->getSourceManager(); sourceManager;
         sourceManager = sourceManager->getParent())
    {
        for (SourceView* view : sourceManager->getSourceViews())
        {
            if (view->getSourceFile() == sourceFile && !view->getInitiatingSourceLoc().isValid())
            {
                return view;
            }
        }
    }

    return nullptr;
}

static void _outputInclude(SourceFile* sourceFile, Index depth, DiagnosticSink* sink)
{
    StringBuilder buf;

    for (Index i = 0; i < depth; ++i)
    {
        buf << "  ";
    }

    // Output the found path for now
    // TODO(JS). We could use the verbose paths flag to control what path is output -> as it may be
    // useful to output the full path for example

    const PathInfo& pathInfo = sourceFile->getPathInfo();
    buf << "'" << pathInfo.foundPath << "'";

    // TODO(JS)?
    // You might want to know where this include was from.
    // If I output this though there will be a problem... as the indenting won't be clearly shown.
    // Perhaps I output in two sections, one the hierarchy and the other the locations of the
    // includes?

    sink->diagnose(SourceLoc(), Diagnostics::includeOutput, buf);
}

static void _outputIncludesRec(
    SourceView* sourceView,
    Index depth,
    ViewInitiatingHierarchy& hierarchy,
    DiagnosticSink* sink)
{
    SourceFile* sourceFile = sourceView->getSourceFile();
    const PathInfo& pathInfo = sourceFile->getPathInfo();

    switch (pathInfo.type)
    {
    case PathInfo::Type::TokenPaste:
    case PathInfo::Type::CommandLine:
    case PathInfo::Type::TypeParse:
        {
            // If any of these types we don't output
            return;
        }
    default:
        break;
    }

    // Okay output this file at the current depth
    _outputInclude(sourceFile, depth, sink);

    // Now recurse to all of the children at the next depth
    List<SourceView*>* children = hierarchy.tryGetValue(sourceView);
    if (children)
    {
        for (SourceView* child : *children)
        {
            _outputIncludesRec(child, depth + 1, hierarchy, sink);
        }
    }
}

static void _outputPreprocessorTokens(const TokenList& toks, ISlangWriter* writer)
{
    if (writer == nullptr)
    {
        return;
    }

    StringBuilder buf;
    for (const auto& tok : toks)
    {
        buf << tok.getContent();
        // We'll separate tokens with space for now
        buf.appendChar(' ');
    }

    buf.appendChar('\n');

    writer->write(buf.getBuffer(), buf.getLength());
}

static void _outputIncludes(
    const List<SourceFile*>& sourceFiles,
    SourceManager* sourceManager,
    DiagnosticSink* sink)
{
    // Set up the hierarchy to know how all the source views relate. This could be argued as
    // overkill, but makes recursive output pretty simple
    ViewInitiatingHierarchy hierarchy;
    _calcViewInitiatingHierarchy(sourceManager, hierarchy);

    // For all the source files
    for (SourceFile* sourceFile : sourceFiles)
    {
        if (sourceFile->isIncludedFile())
            continue;

        // Find an initial view (this is the view of this file, that doesn't have an initiating loc)
        SourceView* sourceView = _findInitialSourceView(sourceFile);
        if (!sourceView)
        {
            // Okay, didn't find one, so just output the file
            _outputInclude(sourceFile, 0, sink);
        }
        else
        {
            // Output from this view recursively
            _outputIncludesRec(sourceView, 0, hierarchy, sink);
        }
    }
}

void FrontEndCompileRequest::parseTranslationUnit(TranslationUnitRequest* translationUnit)
{
    SLANG_PROFILE;
    if (translationUnit->isChecked)
        return;

    auto linkage = getLinkage();

    SLANG_AST_BUILDER_RAII(linkage->getASTBuilder());

    // TODO(JS): NOTE! Here we are using the searchDirectories on the linkage. This is because
    // currently the API only allows the setting search paths on linkage.
    //
    // Here we should probably be using the searchDirectories on the FrontEndCompileRequest.
    // If searchDirectories.parent pointed to the one in the Linkage would mean linkage paths
    // would be checked too (after those on the FrontEndCompileRequest).
    IncludeSystem includeSystem(
        &linkage->getSearchDirectories(),
        linkage->getFileSystemExt(),
        linkage->getSourceManager());

    auto combinedPreprocessorDefinitions = translationUnit->getCombinedPreprocessorDefinitions();

    auto module = translationUnit->getModule();

    ASTBuilder* astBuilder = module->getASTBuilder();

    ModuleDecl* translationUnitSyntax = astBuilder->create<ModuleDecl>();

    translationUnitSyntax->nameAndLoc.name = translationUnit->moduleName;
    translationUnitSyntax->module = module;
    module->setModuleDecl(translationUnitSyntax);

    // When compiling a module of code that belongs to the Slang
    // core module, we add a modifier to the module to act
    // as a marker, so that downstream code can detect declarations
    // that came from the core module (by walking up their
    // chain of ancestors and looking for the marker), and treat
    // them differently from user declarations.
    //
    // We are adding the marker here, before we even parse the
    // code in the module, in case the subsequent steps would
    // like to treat the core module differently. Alternatively
    // we could pass down the `m_isStandardLibraryCode` flag to
    // these passes.
    //
    if (m_isCoreModuleCode)
    {
        translationUnitSyntax->modifiers.first = astBuilder->create<FromCoreModuleModifier>();
    }

    // We use a custom handler for preprocessor callbacks, to
    // ensure that relevant state that is only visible during
    // preprocessoing can be communicated to later phases of
    // compilation.
    //
    FrontEndPreprocessorHandler preprocessorHandler(module, astBuilder, getSink(), translationUnit);

    for (auto sourceFile : translationUnit->getSourceFiles())
    {
        module->getIncludedSourceFileMap().addIfNotExists(sourceFile, nullptr);
    }

    // For a new translation unit, we need to reset the WarningStateTracker
    // to avoid pragma state pollution from previously parsed modules.
    // This is only done for the first file of the translation unit.
    // Subsequent files (if any) in the same translation unit, as well as
    // files included via __include during semantic checking, will reuse
    // the tracker to preserve pragma states within the module.
    getSink()->setSourceWarningStateTracker(nullptr);

    for (auto sourceFile : translationUnit->getSourceFiles())
    {
        SourceLanguage sourceLanguage = translationUnit->sourceLanguage;
        SlangLanguageVersion languageVersion =
            translationUnit->compileRequest->optionSet.getLanguageVersion();
        auto tokens = preprocessSource(
            sourceFile,
            getSink(),
            &includeSystem,
            combinedPreprocessorDefinitions,
            getLinkage(),
            sourceLanguage,
            languageVersion,
            &preprocessorHandler);

        translationUnitSyntax->languageVersion = languageVersion;

        if (sourceLanguage == SourceLanguage::Unknown)
            sourceLanguage = translationUnit->sourceLanguage;

        Scope* languageScope = nullptr;
        switch (sourceLanguage)
        {
        case SourceLanguage::HLSL:
            languageScope = getSession()->hlslLanguageScope;
            break;
        case SourceLanguage::GLSL:
            languageScope = getSession()->glslLanguageScope;
            break;
        case SourceLanguage::Slang:
        default:
            languageScope = getSession()->slangLanguageScope;
            break;
        }

        if (optionSet.getBoolOption(CompilerOptionName::OutputIncludes))
        {
            _outputIncludes(
                translationUnit->getSourceFiles(),
                getSink()->getSourceManager(),
                getSink());
        }

        if (optionSet.getBoolOption(CompilerOptionName::PreprocessorOutput))
        {
            if (m_writers)
            {
                _outputPreprocessorTokens(
                    tokens,
                    m_writers->getWriter(SLANG_WRITER_CHANNEL_STD_OUTPUT));
            }
            // If we output the preprocessor output then we are done doing anything else
            return;
        }

        parseSourceFile(
            astBuilder,
            translationUnit,
            sourceLanguage,
            tokens,
            getSink(),
            languageScope,
            translationUnitSyntax);

        // Let's try dumping

        if (optionSet.getBoolOption(CompilerOptionName::DumpAst))
        {
            StringBuilder buf;
            SourceWriter writer(linkage->getSourceManager(), LineDirectiveMode::None, nullptr);

            ASTDumpUtil::dump(
                translationUnit->getModuleDecl(),
                ASTDumpUtil::Style::Flat,
                0,
                &writer);

            const String& path = sourceFile->getPathInfo().foundPath;
            if (path.getLength())
            {
                String fileName = Path::getFileNameWithoutExt(path);
                fileName.append(".slang-ast");

                File::writeAllText(fileName, writer.getContent());
            }
        }

#if 0
            // Test serialization
            {
                ASTSerialTestUtil::testSerialize(translationUnit->getModuleDecl(), getSession()->getNamePool(), getLinkage()->getASTBuilder()->getSharedASTBuilder(), getSourceManager());
            }
#endif
    }
}

void FrontEndCompileRequest::checkAllTranslationUnits()
{
    SLANG_PROFILE;

    LoadedModuleDictionary loadedModules;
    if (additionalLoadedModules)
        loadedModules = *additionalLoadedModules;

    // Iterate over all translation units and
    // apply the semantic checking logic.
    for (auto& translationUnit : translationUnits)
    {
        if (translationUnit->isChecked)
            continue;

        checkTranslationUnit(translationUnit.Ptr(), loadedModules);

        // Add the checked module to list of loadedModules so that they can be
        // discovered by `findOrImportModule` when processing future `import` decls.
        // TODO: this does not handle the case where a translation unit to discover
        // another translation unit added later to the compilation request.
        // We should output an error message when we detect such a case, or support
        // this scenario with a recursive style checking.
        loadedModules.add(translationUnit->moduleName, translationUnit->getModule());
    }
    checkEntryPoints();
}

void FrontEndCompileRequest::generateIR()
{
    SLANG_PROFILE;
    SLANG_AST_BUILDER_RAII(getLinkage()->getASTBuilder());

    // Our task in this function is to generate IR code
    // for all of the declarations in the translation
    // units that were loaded.

    // Each translation unit is its own little world
    // for code generation (we are not trying to
    // replicate the GLSL linkage model), and so
    // we will generate IR for each (if needed)
    // in isolation.
    for (auto& translationUnit : translationUnits)
    {
        // Skip if the module is precompiled.
        if (translationUnit->getModule()->getIRModule())
            continue;

        // We want to only run generateIRForTranslationUnit once here. This is for two side effects:
        // * it can dump ir
        // * it can generate diagnostics

        /// Generate IR for translation unit.
        RefPtr<IRModule> irModule(
            generateIRForTranslationUnit(getLinkage()->getASTBuilder(), translationUnit));

        if (verifyDebugSerialization)
        {
            SerialContainerUtil::WriteOptions options;

            options.sourceManagerToUseWhenSerializingSourceLocs = getSourceManager();

            // Verify debug information
            if (SLANG_FAILED(
                    SerialContainerUtil::verifyIRSerialize(irModule, getSession(), options)))
            {
                getSink()->diagnose(
                    irModule->getModuleInst()->sourceLoc,
                    Diagnostics::serialDebugVerificationFailed);
            }
        }

        // Set the module on the translation unit
        translationUnit->getModule()->setIRModule(irModule);
    }
}

SlangResult FrontEndCompileRequest::executeActionsInner()
{
    SLANG_PROFILE_SECTION(frontEndExecute);
    SLANG_AST_BUILDER_RAII(getLinkage()->getASTBuilder());

    for (TranslationUnitRequest* translationUnit : translationUnits)
    {
        // Make sure SourceFile representation is available for all translationUnits
        SLANG_RETURN_ON_FAIL(translationUnit->requireSourceFiles());
    }


    // Parse everything from the input files requested
    for (TranslationUnitRequest* translationUnit : translationUnits)
    {
        parseTranslationUnit(translationUnit);
    }

    if (optionSet.getBoolOption(CompilerOptionName::PreprocessorOutput))
    {
        // If doing pre-processor output, then we are done
        return SLANG_OK;
    }

    if (getSink()->getErrorCount() != 0)
        return SLANG_FAIL;

    // Perform semantic checking on the whole collection
    {
        SLANG_PROFILE_SECTION(SemanticChecking);
        checkAllTranslationUnits();
    }

    if (getSink()->getErrorCount() != 0)
        return SLANG_FAIL;

    // After semantic checking is performed we can try and output doc information for this
    if (optionSet.getBoolOption(CompilerOptionName::Doc))
    {
        // TODO: implement the logic to output generated documents to target directory/zip file.
    }

    // Look up all the entry points that are expected,
    // and use them to populate the `program` member.
    //
    m_globalComponentType = createUnspecializedGlobalComponentType(this);
    if (getSink()->getErrorCount() != 0)
        return SLANG_FAIL;

    m_globalAndEntryPointsComponentType =
        createUnspecializedGlobalAndEntryPointsComponentType(this, m_unspecializedEntryPoints);
    if (getSink()->getErrorCount() != 0)
        return SLANG_FAIL;

    // We always generate IR for all the translation units.
    //
    // TODO: We may eventually have a mode where we skip
    // IR codegen and only produce an AST (e.g., for use when
    // debugging problems in the parser or semantic checking),
    // but for now there are no cases where not having IR
    // makes sense.
    //
    generateIR();
    if (getSink()->getErrorCount() != 0)
        return SLANG_FAIL;

    // Do parameter binding generation, for each compilation target.
    //
    for (auto targetReq : getLinkage()->targets)
    {
        auto targetProgram = m_globalAndEntryPointsComponentType->getTargetProgram(targetReq);
        targetProgram->getOrCreateLayout(getSink());
        targetProgram->getOrCreateIRModuleForLayout(getSink());
    }
    if (getSink()->getErrorCount() != 0)
        return SLANG_FAIL;

    return SLANG_OK;
}

int FrontEndCompileRequest::addTranslationUnit(SourceLanguage language, Name* moduleName)
{
    RefPtr<TranslationUnitRequest> translationUnit = new TranslationUnitRequest(this);
    translationUnit->compileRequest = this;
    translationUnit->sourceLanguage = SourceLanguage(language);

    translationUnit->setModuleName(moduleName);
    return addTranslationUnit(translationUnit);
}

int FrontEndCompileRequest::addTranslationUnit(TranslationUnitRequest* translationUnit)
{
    Index result = translationUnits.getCount();
    translationUnits.add(translationUnit);
    return (int)result;
}

void FrontEndCompileRequest::addTranslationUnitSourceArtifact(
    int translationUnitIndex,
    IArtifact* sourceArtifact)
{
    auto translationUnit = translationUnits[translationUnitIndex];

    // Add the source file
    translationUnit->addSourceArtifact(sourceArtifact);

    if (!translationUnit->moduleName)
    {
        translationUnit->setModuleName(
            getNamePool()->getName(Path::getFileNameWithoutExt(sourceArtifact->getName())));
    }
    if (translationUnit->module->getFilePath() == nullptr)
        translationUnit->module->setPathInfo(PathInfo::makePath(sourceArtifact->getName()));
}

void FrontEndCompileRequest::addTranslationUnitSourceBlob(
    int translationUnitIndex,
    String const& path,
    ISlangBlob* sourceBlob)
{
    auto translationUnit = translationUnits[translationUnitIndex];
    auto sourceDesc =
        ArtifactDescUtil::makeDescForSourceLanguage(asExternal(translationUnit->sourceLanguage));

    auto artifact = ArtifactUtil::createArtifact(sourceDesc, path.getBuffer());
    artifact->addRepresentationUnknown(sourceBlob);

    addTranslationUnitSourceArtifact(translationUnitIndex, artifact);
}

void FrontEndCompileRequest::addTranslationUnitSourceFile(
    int translationUnitIndex,
    String const& path)
{
    // TODO: We need to consider whether a relative `path` should cause
    // us to look things up using the registered search paths.
    //
    // This behavior wouldn't make sense for command-line invocations
    // of `slangc`, but at least one API user wondered by the search
    // paths were not taken into account by this function.
    //

    auto fileSystemExt = getLinkage()->getFileSystemExt();
    auto translationUnit = getTranslationUnit(translationUnitIndex);

    auto sourceDesc =
        ArtifactDescUtil::makeDescForSourceLanguage(asExternal(translationUnit->sourceLanguage));

    auto sourceArtifact = ArtifactUtil::createArtifact(sourceDesc, path.getBuffer());

    auto extRep = new ExtFileArtifactRepresentation(path.getUnownedSlice(), fileSystemExt);
    sourceArtifact->addRepresentation(extRep);

    SlangResult existsRes = SLANG_OK;

    // If we require caching, we demand it's loaded here.
    //
    // In practice this probably means repro capture is enabled. So we want to
    // load the blob such that it's in the cache, even if it doesn't actually
    // have to be loaded for the compilation.
    if (getLinkage()->m_requireCacheFileSystem)
    {
        ComPtr<ISlangBlob> blob;
        // If we can load the blob, then it exists
        existsRes = sourceArtifact->loadBlob(ArtifactKeep::Yes, blob.writeRef());
    }
    else
    {
        existsRes = sourceArtifact->exists() ? SLANG_OK : SLANG_E_NOT_FOUND;
    }

    if (SLANG_FAILED(existsRes))
    {
        // Emit a diagnostic!
        getSink()->diagnose(SourceLoc(), Diagnostics::cannotOpenFile, path);
        return;
    }

    addTranslationUnitSourceArtifact(translationUnitIndex, sourceArtifact);
}

int FrontEndCompileRequest::addEntryPoint(
    int translationUnitIndex,
    String const& name,
    Profile entryPointProfile)
{
    auto translationUnitReq = translationUnits[translationUnitIndex];

    Index result = m_entryPointReqs.getCount();

    RefPtr<FrontEndEntryPointRequest> entryPointReq = new FrontEndEntryPointRequest(
        this,
        translationUnitIndex,
        getNamePool()->getName(name),
        entryPointProfile);

    m_entryPointReqs.add(entryPointReq);
    //    translationUnitReq->entryPoints.add(entryPointReq);

    return int(result);
}

int EndToEndCompileRequest::addEntryPoint(
    int translationUnitIndex,
    String const& name,
    Profile entryPointProfile,
    List<String> const& genericTypeNames)
{
    getFrontEndReq()->addEntryPoint(translationUnitIndex, name, entryPointProfile);

    EntryPointInfo entryPointInfo;
    for (auto typeName : genericTypeNames)
        entryPointInfo.specializationArgStrings.add(typeName);

    Index result = m_entryPoints.getCount();
    m_entryPoints.add(_Move(entryPointInfo));
    return (int)result;
}

} // namespace Slang
