// slang-global-session.cpp
#include "slang-global-session.h"

#include "compiler-core/slang-artifact-desc-util.h"
#include "core/slang-archive-file-system.h"
#include "core/slang-performance-profiler.h"
#include "core/slang-type-convert-util.h"
#include "slang-check-impl.h"
#include "slang-compiler.h"
#include "slang-doc-ast.h"
#include "slang-doc-markdown-writer.h"
#include "slang-options.h"
#include "slang-parser.h"
#include "slang-serialize-ast.h"
#include "slang-serialize-container.h"
#include "slang-serialize-ir.h"

extern Slang::String get_slang_cuda_prelude();
extern Slang::String get_slang_cpp_prelude();
extern Slang::String get_slang_hlsl_prelude();

namespace Slang
{

void Session::init()
{
    SLANG_ASSERT(BaseTypeInfo::check());

#if SLANG_ENABLE_IR_BREAK_ALLOC
    // Read environment variable for IR debugging
    StringBuilder irBreakEnv;
    if (SLANG_SUCCEEDED(PlatformUtil::getEnvironmentVariable(
            UnownedStringSlice("SLANG_DEBUG_IR_BREAK"),
            irBreakEnv)))
    {
        String envValue = irBreakEnv.produceString();
        if (envValue.getLength())
        {
            _slangIRAllocBreak = stringToInt(envValue);
            _slangIRPrintStackAtBreak = true;
        }
    }
#endif

    _initCodeGenTransitionMap();

    ::memset(m_downstreamCompilerLocators, 0, sizeof(m_downstreamCompilerLocators));
    DownstreamCompilerUtil::setDefaultLocators(m_downstreamCompilerLocators);
    m_downstreamCompilerSet = new DownstreamCompilerSet;

    m_completionTokenName = getNamePool()->getName("#?");

    m_sharedLibraryLoader = DefaultSharedLibraryLoader::getSingleton();

    // Set up the command line options
    initCommandOptions(m_commandOptions);

    // Create the root AST builder that will be used when
    // loading the builtin modules, and which will serve as
    // the parent for the the AST builder of any linkages
    // created from this global session.
    //
    auto rootASTBuilder = new RootASTBuilder(this);
    m_rootASTBuilder = rootASTBuilder;

    // Make sure our source manager is initialized
    builtinSourceManager.initialize(nullptr, nullptr);

    // Built in linkage uses the built in builder
    m_builtinLinkage = new Linkage(this, rootASTBuilder, nullptr);
    m_builtinLinkage->m_optionSet.set(CompilerOptionName::DebugInformation, DebugInfoLevel::None);

    // Because the `Session` retains the builtin `Linkage`,
    // we need to make sure that the parent pointer inside
    // `Linkage` doesn't create a retain cycle.
    //
    // This operation ensures that the parent pointer will
    // just be a raw pointer, so that the builtin linkage
    // doesn't keep the parent session alive.
    //
    m_builtinLinkage->_stopRetainingParentSession();

    // Create scopes for various language builtins.
    //
    // TODO: load these on-demand to avoid parsing
    // the core module code for languages the user won't use.

    baseLanguageScope = rootASTBuilder->create<Scope>();

    // Will stay in scope as long as ASTBuilder
    baseModuleDecl =
        populateBaseLanguageModule(m_builtinLinkage->getASTBuilder(), baseLanguageScope);

    coreLanguageScope = rootASTBuilder->create<Scope>();
    coreLanguageScope->nextSibling = baseLanguageScope;

    hlslLanguageScope = rootASTBuilder->create<Scope>();
    hlslLanguageScope->nextSibling = coreLanguageScope;

    slangLanguageScope = rootASTBuilder->create<Scope>();
    slangLanguageScope->nextSibling = hlslLanguageScope;

    glslLanguageScope = rootASTBuilder->create<Scope>();
    glslLanguageScope->nextSibling = slangLanguageScope;

    glslModuleName = getNameObj("glsl");
    neuralModuleName = getNameObj("neural");

    {
        for (Index i = 0; i < Index(SourceLanguage::CountOf); ++i)
        {
            m_defaultDownstreamCompilers[i] = PassThroughMode::None;
        }
        m_defaultDownstreamCompilers[Index(SourceLanguage::C)] = PassThroughMode::GenericCCpp;
        m_defaultDownstreamCompilers[Index(SourceLanguage::CPP)] = PassThroughMode::GenericCCpp;
        m_defaultDownstreamCompilers[Index(SourceLanguage::CUDA)] = PassThroughMode::NVRTC;
    }

    // Set up default prelude code for target languages that need a prelude
    m_languagePreludes[Index(SourceLanguage::CUDA)] = get_slang_cuda_prelude();
    m_languagePreludes[Index(SourceLanguage::CPP)] = get_slang_cpp_prelude();
    m_languagePreludes[Index(SourceLanguage::HLSL)] = get_slang_hlsl_prelude();

    if (!spirvCoreGrammarInfo)
        spirvCoreGrammarInfo = SPIRVCoreGrammarInfo::getEmbeddedVersion();
}

Session::~Session()
{
    // Destroy the array of core (automatically-included) modules.
    //
    // TODO(tfoley): This code didn't have a comment clearly explaining
    // why this step is necessary, but the other line that used to be
    // here had a comment that expressed concern about the `SharedASTBuilder`
    // gettng destruted before things that refer to it. It is possible
    // that the underlying problem here is that the modules in the
    // `coreModules` array are owned by the builtin linkage, and Bad Things
    // would happen if the linkage gets destroyed while these modules
    // are still alive.
    //
    coreModules = decltype(coreModules)();
}

SharedASTBuilder* Session::getSharedASTBuilder()
{
    return getASTBuilder()->getSharedASTBuilder();
}

Module* Session::getBuiltinModule(slang::BuiltinModuleName name)
{
    auto info = getBuiltinModuleInfo(name);
    auto builtinLinkage = getBuiltinLinkage();
    auto moduleNameObj = builtinLinkage->getNamePool()->getName(info.name);
    RefPtr<Module> module;
    if (builtinLinkage->mapNameToLoadedModules.tryGetValue(moduleNameObj, module))
        return module.get();
    return nullptr;
}

void Session::_initCodeGenTransitionMap()
{
    // TODO(JS): Might want to do something about these in the future...

    // PassThroughMode getDownstreamCompilerRequiredForTarget(CodeGenTarget target);
    // SourceLanguage getDefaultSourceLanguageForDownstreamCompiler(PassThroughMode compiler);

    // Set up the default ways to do compilations between code gen targets
    auto& map = m_codeGenTransitionMap;

    // TODO(JS): There currently isn't a 'downstream compiler' for direct spirv output. If we did
    // it would presumably a transition from SlangIR to SPIRV.

    // For C and C++ we default to use the 'genericCCpp' compiler
    {
        const CodeGenTarget sources[] = {CodeGenTarget::CSource, CodeGenTarget::CPPSource};
        for (auto source : sources)
        {
            // We *don't* add a default for host callable, as we will determine what is suitable
            // depending on what is available. We prefer LLVM if that's available. If it's not we
            // can use generic C/C++ compiler

            map.addTransition(
                source,
                CodeGenTarget::ShaderSharedLibrary,
                PassThroughMode::GenericCCpp);
            map.addTransition(
                source,
                CodeGenTarget::HostSharedLibrary,
                PassThroughMode::GenericCCpp);
            map.addTransition(source, CodeGenTarget::HostExecutable, PassThroughMode::GenericCCpp);
            map.addTransition(source, CodeGenTarget::ObjectCode, PassThroughMode::GenericCCpp);
        }
    }


    // Add all the straightforward transitions
    map.addTransition(CodeGenTarget::CUDASource, CodeGenTarget::PTX, PassThroughMode::NVRTC);
    map.addTransition(CodeGenTarget::HLSL, CodeGenTarget::DXBytecode, PassThroughMode::Fxc);
    map.addTransition(CodeGenTarget::HLSL, CodeGenTarget::DXIL, PassThroughMode::Dxc);
    map.addTransition(CodeGenTarget::GLSL, CodeGenTarget::SPIRV, PassThroughMode::Glslang);
    map.addTransition(CodeGenTarget::Metal, CodeGenTarget::MetalLib, PassThroughMode::MetalC);
    map.addTransition(CodeGenTarget::WGSL, CodeGenTarget::WGSLSPIRV, PassThroughMode::Tint);
    // To assembly
    map.addTransition(CodeGenTarget::SPIRV, CodeGenTarget::SPIRVAssembly, PassThroughMode::Glslang);
    // We use glslang to turn SPIR-V into SPIR-V assembly.
    map.addTransition(
        CodeGenTarget::WGSLSPIRV,
        CodeGenTarget::WGSLSPIRVAssembly,
        PassThroughMode::Glslang);
    map.addTransition(CodeGenTarget::DXIL, CodeGenTarget::DXILAssembly, PassThroughMode::Dxc);
    map.addTransition(
        CodeGenTarget::DXBytecode,
        CodeGenTarget::DXBytecodeAssembly,
        PassThroughMode::Fxc);
    map.addTransition(
        CodeGenTarget::MetalLib,
        CodeGenTarget::MetalLibAssembly,
        PassThroughMode::MetalC);
}

void Session::addBuiltins(char const* sourcePath, char const* source)
{
    auto sourceBlob = StringBlob::moveCreate(String(source));

    // TODO(tfoley): Add ability to directly new builtins to the appropriate scope
    Module* module = nullptr;
    addBuiltinSource(coreLanguageScope, sourcePath, sourceBlob, module);
    if (module)
        coreModules.add(module);
}

void Session::setSharedLibraryLoader(ISlangSharedLibraryLoader* loader)
{
    // External API allows passing of nullptr to reset the loader
    loader = loader ? loader : DefaultSharedLibraryLoader::getSingleton();

    _setSharedLibraryLoader(loader);
}

ISlangSharedLibraryLoader* Session::getSharedLibraryLoader()
{
    return (m_sharedLibraryLoader == DefaultSharedLibraryLoader::getSingleton())
               ? nullptr
               : m_sharedLibraryLoader.get();
}

SlangResult Session::checkCompileTargetSupport(SlangCompileTarget inTarget)
{
    auto target = CodeGenTarget(inTarget);

    const PassThroughMode mode = getDownstreamCompilerRequiredForTarget(target);
    return (mode != PassThroughMode::None) ? checkPassThroughSupport(SlangPassThrough(mode))
                                           : SLANG_OK;
}

SlangResult Session::checkPassThroughSupport(SlangPassThrough inPassThrough)
{
    return checkExternalCompilerSupport(this, PassThroughMode(inPassThrough));
}

void Session::writeCoreModuleDoc(String config)
{
    ASTBuilder* astBuilder = getBuiltinLinkage()->getASTBuilder();
    SourceManager* sourceManager = getBuiltinSourceManager();

    DiagnosticSink sink(sourceManager, Lexer::sourceLocationLexer);

    List<String> docStrings;

    // For all the modules add their doc output to docStrings
    for (Module* m : coreModules)
    {
        RefPtr<ASTMarkup> markup(new ASTMarkup);
        ASTMarkupUtil::extract(m->getModuleDecl(), sourceManager, &sink, markup);

        DocMarkdownWriter writer(markup, astBuilder, &sink);
        auto rootPage = writer.writeAll(config.getUnownedSlice());
        File::writeAllText("toc.html", writer.writeTOC());
        rootPage->writeToDisk();
        rootPage->writeSummary(toSlice("summary.txt"));
    }
    ComPtr<ISlangBlob> diagnosticBlob;
    sink.getBlobIfNeeded(diagnosticBlob.writeRef());
    if (diagnosticBlob && diagnosticBlob->getBufferSize() != 0)
    {
        // Write the diagnostic blob to stdout.
        fprintf(stderr, "%s", (const char*)diagnosticBlob->getBufferPointer());
    }
}

const char* getBuiltinModuleNameStr(slang::BuiltinModuleName name)
{
    const char* result = nullptr;
    switch (name)
    {
    case slang::BuiltinModuleName::Core:
        result = "core";
        break;
    case slang::BuiltinModuleName::GLSL:
        result = "glsl";
        break;
    default:
        SLANG_UNEXPECTED("Unknown builtin module");
    }
    return result;
}

TypeCheckingCache* Session::getTypeCheckingCache()
{
    return static_cast<TypeCheckingCache*>(m_typeCheckingCache.get());
}

Session::BuiltinModuleInfo Session::getBuiltinModuleInfo(slang::BuiltinModuleName name)
{
    Session::BuiltinModuleInfo result;

    result.name = getBuiltinModuleNameStr(name);

    switch (name)
    {
    case slang::BuiltinModuleName::Core:
        result.languageScope = coreLanguageScope;
        break;
    case slang::BuiltinModuleName::GLSL:
        result.name = "glsl";
        result.languageScope = glslLanguageScope;
        break;
    default:
        SLANG_UNEXPECTED("Unknown builtin module");
    }
    return result;
}

SlangResult Session::compileCoreModule(slang::CompileCoreModuleFlags compileFlags)
{
    return compileBuiltinModule(slang::BuiltinModuleName::Core, compileFlags);
}

void Session::getBuiltinModuleSource(StringBuilder& sb, slang::BuiltinModuleName moduleName)
{
    switch (moduleName)
    {
    case slang::BuiltinModuleName::Core:
        sb << (const char*)getCoreLibraryCode()->getBufferPointer()
           << (const char*)getHLSLLibraryCode()->getBufferPointer()
           << (const char*)getAutodiffLibraryCode()->getBufferPointer();
        break;
    case slang::BuiltinModuleName::GLSL:
        sb << (const char*)getGLSLLibraryCode()->getBufferPointer();
        break;
    }
}

SlangResult Session::compileBuiltinModule(
    slang::BuiltinModuleName moduleName,
    slang::CompileCoreModuleFlags compileFlags)
{
    SLANG_AST_BUILDER_RAII(m_builtinLinkage->getASTBuilder());

#ifdef _DEBUG
    time_t beginTime = 0;
    if (moduleName == slang::BuiltinModuleName::Core)
    {
        // Print a message in debug builds to notice the user that compiling the core module
        // can take a while.
        time(&beginTime);
        fprintf(stderr, "Compiling core module on debug build, this can take a while.\n");
    }
#endif
    BuiltinModuleInfo builtinModuleInfo = getBuiltinModuleInfo(moduleName);
    auto moduleNameObj = m_builtinLinkage->getNamePool()->getName(builtinModuleInfo.name);
    if (m_builtinLinkage->mapNameToLoadedModules.tryGetValue(moduleNameObj))
    {
        // Already have the builtin module loaded
        return SLANG_FAIL;
    }

    StringBuilder moduleSrcBuilder;
    getBuiltinModuleSource(moduleSrcBuilder, moduleName);

    // TODO(JS): Could make this return a SlangResult as opposed to exception
    auto moduleSrcBlob = StringBlob::moveCreate(moduleSrcBuilder.produceString());
    Module* compiledModule = nullptr;
    addBuiltinSource(
        builtinModuleInfo.languageScope,
        builtinModuleInfo.name,
        moduleSrcBlob,
        compiledModule);

    if (moduleName == slang::BuiltinModuleName::Core)
    {
        // We need to retain this AST so that we can use it in other code
        // (Note that the `Scope` type does not retain the AST it points to)
        coreModules.add(compiledModule);
    }

    if (compileFlags & slang::CompileCoreModuleFlag::WriteDocumentation)
    {
        // Load config file first.
        String configText;
        if (SLANG_FAILED(File::readAllText("config.txt", configText)))
        {
            fprintf(
                stderr,
                "Error writing documentation: config file not found on current working "
                "directory.\n");
        }
        else
        {
            writeCoreModuleDoc(configText);
        }
    }

#ifdef _DEBUG
    if (moduleName == slang::BuiltinModuleName::Core)
    {
        time_t endTime;
        time(&endTime);
        fprintf(stderr, "Compiling core module took %.2f seconds.\n", difftime(endTime, beginTime));
    }
#endif
    return SLANG_OK;
}

SlangResult Session::loadCoreModule(const void* coreModule, size_t coreModuleSizeInBytes)
{
    return loadBuiltinModule(slang::BuiltinModuleName::Core, coreModule, coreModuleSizeInBytes);
}

SlangResult Session::loadBuiltinModule(
    slang::BuiltinModuleName moduleName,
    const void* moduleData,
    size_t sizeInBytes)
{
    SLANG_PROFILE;


    SLANG_AST_BUILDER_RAII(m_builtinLinkage->getASTBuilder());

    BuiltinModuleInfo builtinModuleInfo = getBuiltinModuleInfo(moduleName);
    auto nameObj = m_builtinLinkage->getNamePool()->getName(builtinModuleInfo.name);
    if (m_builtinLinkage->mapNameToLoadedModules.containsKey(nameObj))
    {
        // Already have a core module loaded
        return SLANG_FAIL;
    }

    // Make a file system to read it from
    ComPtr<ISlangFileSystemExt> fileSystem;
    SLANG_RETURN_ON_FAIL(loadArchiveFileSystem(moduleData, sizeInBytes, fileSystem));

    // Let's try loading serialized modules and adding them
    Module* module = nullptr;
    SLANG_RETURN_ON_FAIL(_readBuiltinModule(
        fileSystem,
        builtinModuleInfo.languageScope,
        builtinModuleInfo.name,
        module));

    if (moduleName == slang::BuiltinModuleName::Core)
    {
        // We need to retain this AST so that we can use it in other code
        // (Note that the `Scope` type does not retain the AST it points to)
        coreModules.add(module);
    }

    return SLANG_OK;
}

SlangResult Session::saveCoreModule(SlangArchiveType archiveType, ISlangBlob** outBlob)
{
    return saveBuiltinModule(slang::BuiltinModuleName::Core, archiveType, outBlob);
}

SlangResult Session::saveBuiltinModule(
    slang::BuiltinModuleName moduleTag,
    SlangArchiveType archiveType,
    ISlangBlob** outBlob)
{
    // If no builtin modules have been loaded, then there is
    // nothing to save, and we fail immediately.
    //
    if (m_builtinLinkage->mapNameToLoadedModules.getCount() == 0)
    {
        return SLANG_FAIL;
    }

    // The module will need to be looked up by its name, and
    // will also be serialized out to a path with a matching name.
    //
    BuiltinModuleInfo moduleInfo = getBuiltinModuleInfo(moduleTag);
    const char* moduleName = moduleInfo.name;

    // If we cannot find a loaded module in the linkage with
    // the appropriate name, then for some reason it hasn't
    // been loaded, and we fail.
    //
    RefPtr<Module> module;
    m_builtinLinkage->mapNameToLoadedModules.tryGetValue(
        getNameObj(UnownedStringSlice(moduleName)),
        module);
    if (!module)
    {
        return SLANG_FAIL;
    }

    // AST serialization needs access to an AST builder, so
    // we establish a current builder for the duration of
    // the serialization process.
    //
    SLANG_AST_BUILDER_RAII(m_builtinLinkage->getASTBuilder());

    // The serialized module will be represented as a logical
    // file in an archive, so we create a logical file system
    // to represent that archive.
    //
    ComPtr<ISlangMutableFileSystem> fileSystem;
    SLANG_RETURN_ON_FAIL(createArchiveFileSystem(archiveType, fileSystem));
    //
    // The created file system must support the `IArchiveFileSystem`
    // interface (since we created it with `createArchiveFileSystem`).
    //
    auto archiveFileSystem = as<IArchiveFileSystem>(fileSystem);
    if (!archiveFileSystem)
    {
        return SLANG_FAIL;
    }

    // The output file name that we'll write to in that file system
    // is just the builtin module name with a `.slang-module` suffix.
    //
    StringBuilder moduleFileName;
    moduleFileName << moduleName << ".slang-module";

    // The module serialization step has some options that we need
    // to configure appropriately.
    //
    SerialContainerUtil::WriteOptions options;
    //
    // We want builtin modules to be saved with their source location
    // information.
    //
    // And in order to work with source locations, the serialization
    // process will also need access to the source manager that
    // can translate locations into their humane format.
    //
    options.sourceManagerToUseWhenSerializingSourceLocs = m_builtinLinkage->getSourceManager();

    // At this point we can finally delegate down to the next level,
    // which handles the serialization of a Slang module into a
    // byte stream.
    //
    OwnedMemoryStream stream(FileAccess::Write);
    SLANG_RETURN_ON_FAIL(SerialContainerUtil::write(module, options, &stream));
    auto contents = stream.getContents();

    // Once the stream that represents the module has been written, we can
    // write it to a file in the logical file system.
    //
    // TODO(tfoley): why can't the file system let us open the file for output?
    //
    SLANG_RETURN_ON_FAIL(fileSystem->saveFile(
        moduleFileName.getBuffer(),
        contents.getBuffer(),
        contents.getCount()));

    // And finally, we can ask the archive file system to serialize itself
    // out as a blob of bytes, which yields the final serialized representation
    // of the module.
    //
    SLANG_RETURN_ON_FAIL(archiveFileSystem->storeArchive(
        // The `true` here indicates that the blob that gets created should own
        // its content, independent from the file system object itself; otherwise
        // the file system might return a blob that shares storage with itself.
        true,
        outBlob));

    return SLANG_OK;
}

SlangResult Session::_readBuiltinModule(
    ISlangFileSystem* fileSystem,
    Scope* scope,
    String moduleName,
    Module*& outModule)
{
    // Get the name of the module
    StringBuilder moduleFilename;
    moduleFilename << moduleName << ".slang-module";

    // Load it
    ComPtr<ISlangBlob> fileContents;
    SLANG_RETURN_ON_FAIL(fileSystem->loadFile(moduleFilename.getBuffer(), fileContents.writeRef()));

    RIFF::RootChunk const* rootChunk = RIFF::RootChunk::getFromBlob(fileContents);
    if (!rootChunk)
    {
        return SLANG_FAIL;
    }

    Linkage* linkage = getBuiltinLinkage();
    SourceManager* sourceManager = getBuiltinSourceManager();
    NamePool* sessionNamePool = &namePool;

    auto moduleChunk = ModuleChunk::find(rootChunk);
    if (!moduleChunk)
        return SLANG_FAIL;

    SHA1::Digest moduleDigest = moduleChunk->getDigest();

    auto irChunk = moduleChunk->findIR();
    if (!irChunk)
        return SLANG_FAIL;

    auto astChunk = moduleChunk->findAST();
    if (!astChunk)
        return SLANG_FAIL;

    // Source location information is stored as a distinct
    // chunk from the IR and AST, so we need to search for
    // that chunk and then set up the information for use
    // in the IR and AST deserialization (if we find anything).
    //
    RefPtr<SerialSourceLocReader> sourceLocReader;
    if (auto debugChunk = DebugChunk::find(moduleChunk))
    {
        SLANG_RETURN_ON_FAIL(
            readSourceLocationsFromDebugChunk(debugChunk, sourceManager, sourceLocReader));
    }

    // At this point we create the `Module` object that will
    // represent the builtin module we are reading, although
    // it is still possible that deserialization will fail
    // at one of the following steps.
    //
    auto astBuilder = linkage->getASTBuilder();
    RefPtr<Module> module(new Module(linkage, astBuilder));
    module->setName(moduleName);
    module->setDigest(moduleDigest);


    // Next, we set about deserializing the AST representation
    // of the module.
    //
    auto moduleDecl = readSerializedModuleAST(
        linkage,
        astBuilder,
        nullptr, // no sink
        fileContents,
        astChunk,
        sourceLocReader,
        SourceLoc());
    if (!moduleDecl)
    {
        return SLANG_FAIL;
    }
    moduleDecl->module = module;
    module->setModuleDecl(moduleDecl);

    // After the AST module has been read in, we next look
    // to deserialize the IR module.
    //
    RefPtr<IRModule> irModule;
    SLANG_RETURN_ON_FAIL(readSerializedModuleIR(irChunk, this, sourceLocReader, irModule));

    irModule->setName(module->getNameObj());
    module->setIRModule(irModule);

    // Put in the loaded module map
    linkage->mapNameToLoadedModules.add(sessionNamePool->getName(moduleName), module);


    // Add the resulting code to the appropriate scope
    if (!scope->containerDecl)
    {
        // We are the first chunk of code to be loaded for this scope
        scope->containerDecl = moduleDecl;
    }
    else
    {
        // We need to create a new scope to link into the whole thing
        auto subScope = linkage->getASTBuilder()->create<Scope>();
        subScope->containerDecl = moduleDecl;
        subScope->nextSibling = scope->nextSibling;
        scope->nextSibling = subScope;
    }

    outModule = module.get();

    return SLANG_OK;
}

SLANG_NO_THROW SlangResult SLANG_MCALL
Session::queryInterface(SlangUUID const& uuid, void** outObject)
{
    if (uuid == Session::getTypeGuid())
    {
        addReference();
        *outObject = static_cast<Session*>(this);
        return SLANG_OK;
    }

    if (uuid == ISlangUnknown::getTypeGuid() || uuid == IGlobalSession::getTypeGuid())
    {
        addReference();
        *outObject = static_cast<slang::IGlobalSession*>(this);
        return SLANG_OK;
    }

    return SLANG_E_NO_INTERFACE;
}

static size_t _getStructureSize(const uint8_t* src)
{
    size_t size = 0;
    ::memcpy(&size, src, sizeof(size_t));
    return size;
}

template<typename T>
static T makeFromSizeVersioned(const uint8_t* src)
{
    // The structure size must be size_t
    SLANG_COMPILE_TIME_ASSERT(sizeof(((T*)src)->structureSize) == sizeof(size_t));

    // The structureSize field *must* be the first element of T
    // Ideally would use SLANG_COMPILE_TIME_ASSERT, but that doesn't work on gcc.
    // Can't just assert, because determined to be a constant expression
    {
        auto offset = SLANG_OFFSET_OF(T, structureSize);
        SLANG_ASSERT(offset == 0);
        // Needed because offset is only 'used' by an assert
        SLANG_UNUSED(offset);
    }

    // The source size is held in the first element of T, and will be in the first bytes of src.
    const size_t srcSize = _getStructureSize(src);
    const size_t dstSize = sizeof(T);

    // If they are the same size, and appropriate alignment we can just cast and return
    if (srcSize == dstSize && (size_t(src) & (alignof(T) - 1)) == 0)
    {
        return *(const T*)src;
    }

    // Assumes T can default constructed sensibly
    T dst;

    // It's structure size should be setup and should be dstSize
    SLANG_ASSERT(dst.structureSize == dstSize);

    // The size to copy is the minimum on the two sizes
    const auto copySize = std::min(srcSize, dstSize);
    ::memcpy(&dst, src, copySize);

    // The final struct size is the destination size
    dst.structureSize = dstSize;

    return dst;
}

SLANG_NO_THROW SlangResult SLANG_MCALL
Session::createSession(slang::SessionDesc const& inDesc, slang::ISession** outSession)
{
    auto astBuilder = RefPtr(new ASTBuilder(m_rootASTBuilder, "Session::astBuilder"));
    slang::SessionDesc desc = makeFromSizeVersioned<slang::SessionDesc>((uint8_t*)&inDesc);

    RefPtr<Linkage> linkage = new Linkage(this, astBuilder, getBuiltinLinkage());

    if (desc.skipSPIRVValidation)
    {
        linkage->m_optionSet.set(CompilerOptionName::SkipSPIRVValidation, true);
    }

    {
        std::lock_guard<std::mutex> lock(m_typeCheckingCacheMutex);
        if (m_typeCheckingCache)
            linkage->m_typeCheckingCache =
                new TypeCheckingCache(*static_cast<TypeCheckingCache*>(m_typeCheckingCache.get()));
    }

    Int searchPathCount = desc.searchPathCount;
    for (Int ii = 0; ii < searchPathCount; ++ii)
    {
        linkage->addSearchPath(desc.searchPaths[ii]);
    }

    Int macroCount = desc.preprocessorMacroCount;
    for (Int ii = 0; ii < macroCount; ++ii)
    {
        auto& macro = desc.preprocessorMacros[ii];
        linkage->addPreprocessorDefine(macro.name, macro.value);
    }

    if (desc.fileSystem)
    {
        linkage->setFileSystem(desc.fileSystem);
    }

    if (desc.structureSize >= offsetof(slang::SessionDesc, enableEffectAnnotations))
    {
        linkage->m_optionSet.set(
            CompilerOptionName::EnableEffectAnnotations,
            desc.enableEffectAnnotations);
    }

    linkage->m_optionSet.load(desc.compilerOptionEntryCount, desc.compilerOptionEntries);

    if (!linkage->m_optionSet.hasOption(CompilerOptionName::MatrixLayoutColumn) &&
        !linkage->m_optionSet.hasOption(CompilerOptionName::MatrixLayoutRow))
        linkage->setMatrixLayoutMode(desc.defaultMatrixLayoutMode);

    {
        const Int targetCount = desc.targetCount;
        const uint8_t* targetDescPtr = reinterpret_cast<const uint8_t*>(desc.targets);
        for (Int ii = 0; ii < targetCount; ++ii, targetDescPtr += _getStructureSize(targetDescPtr))
        {
            const auto targetDesc = makeFromSizeVersioned<slang::TargetDesc>(targetDescPtr);
            linkage->addTarget(targetDesc);
        }
    }

    // If any target requires debug info, then we will need to enable debug info when lowering to
    // target-agnostic IR. The target-agnostic IR will only include debug info if the linkage IR
    // options specify that it should, so make sure the linkage debug info level is greater than or
    // equal to that of any target.
    DebugInfoLevel linkageDebugInfoLevel = linkage->m_optionSet.getDebugInfoLevel();
    for (auto target : linkage->targets)
        linkageDebugInfoLevel =
            Math::Max(linkageDebugInfoLevel, target->getOptionSet().getDebugInfoLevel());
    linkage->m_optionSet.set(CompilerOptionName::DebugInformation, linkageDebugInfoLevel);

    // Add any referenced modules to the linkage
    for (auto& option : linkage->m_optionSet.options)
    {
        if (option.key != CompilerOptionName::ReferenceModule)
            continue;
        for (auto& path : option.value)
        {
            DiagnosticSink sink;
            ComPtr<IArtifact> artifact;
            SlangResult result = createArtifactFromReferencedModule(
                path.stringValue,
                SourceLoc{},
                &sink,
                artifact.writeRef());
            if (SLANG_FAILED(result))
            {
                sink.diagnose(SourceLoc{}, Diagnostics::unableToReadFile, path.stringValue);
                return result;
            }
            linkage->m_libModules.add(artifact);
        }
    }

    *outSession = asExternal(linkage.detach());
    return SLANG_OK;
}

SLANG_NO_THROW SlangResult SLANG_MCALL
Session::createCompileRequest(slang::ICompileRequest** outCompileRequest)
{
    auto req = new EndToEndCompileRequest(this);

    // Give it a ref (for output)
    req->addRef();
    // Check it is what we think it should be
    SLANG_ASSERT(req->debugGetReferenceCount() == 1);

    *outCompileRequest = req;
    return SLANG_OK;
}

SLANG_NO_THROW SlangProfileID SLANG_MCALL Session::findProfile(char const* name)
{
    return SlangProfileID(Slang::Profile::lookUp(name).raw);
}

SLANG_NO_THROW SlangCapabilityID SLANG_MCALL Session::findCapability(char const* name)
{
    return SlangCapabilityID(Slang::findCapabilityName(UnownedTerminatedStringSlice(name)));
}

SLANG_NO_THROW void SLANG_MCALL
Session::setDownstreamCompilerPath(SlangPassThrough inPassThrough, char const* path)
{
    PassThroughMode passThrough = PassThroughMode(inPassThrough);
    SLANG_ASSERT(
        int(passThrough) > int(PassThroughMode::None) &&
        int(passThrough) < int(PassThroughMode::CountOf));

    if (m_downstreamCompilerPaths[int(passThrough)] != path)
    {
        // Make access redetermine compiler
        resetDownstreamCompiler(passThrough);
        // Set the path
        m_downstreamCompilerPaths[int(passThrough)] = path;
    }
}

SLANG_NO_THROW void SLANG_MCALL
Session::setDownstreamCompilerPrelude(SlangPassThrough inPassThrough, char const* prelude)
{
    PassThroughMode downstreamCompiler = PassThroughMode(inPassThrough);
    SLANG_ASSERT(
        int(downstreamCompiler) > int(PassThroughMode::None) &&
        int(downstreamCompiler) < int(PassThroughMode::CountOf));
    const SourceLanguage sourceLanguage =
        getDefaultSourceLanguageForDownstreamCompiler(downstreamCompiler);
    setLanguagePrelude(SlangSourceLanguage(sourceLanguage), prelude);
}

SLANG_NO_THROW void SLANG_MCALL
Session::getDownstreamCompilerPrelude(SlangPassThrough inPassThrough, ISlangBlob** outPrelude)
{
    PassThroughMode downstreamCompiler = PassThroughMode(inPassThrough);
    SLANG_ASSERT(
        int(downstreamCompiler) > int(PassThroughMode::None) &&
        int(downstreamCompiler) < int(PassThroughMode::CountOf));
    const SourceLanguage sourceLanguage =
        getDefaultSourceLanguageForDownstreamCompiler(downstreamCompiler);
    getLanguagePrelude(SlangSourceLanguage(sourceLanguage), outPrelude);
}

SLANG_NO_THROW void SLANG_MCALL
Session::setLanguagePrelude(SlangSourceLanguage inSourceLanguage, char const* prelude)
{
    SourceLanguage sourceLanguage = SourceLanguage(inSourceLanguage);
    SLANG_ASSERT(
        int(sourceLanguage) > int(SourceLanguage::Unknown) &&
        int(sourceLanguage) < int(SourceLanguage::CountOf));

    SLANG_ASSERT(sourceLanguage != SourceLanguage::Unknown);

    if (sourceLanguage != SourceLanguage::Unknown)
    {
        m_languagePreludes[int(sourceLanguage)] = prelude;
    }
}

SLANG_NO_THROW void SLANG_MCALL
Session::getLanguagePrelude(SlangSourceLanguage inSourceLanguage, ISlangBlob** outPrelude)
{
    SourceLanguage sourceLanguage = SourceLanguage(inSourceLanguage);

    *outPrelude = nullptr;
    if (sourceLanguage != SourceLanguage::Unknown)
    {
        SLANG_ASSERT(
            int(sourceLanguage) > int(SourceLanguage::Unknown) &&
            int(sourceLanguage) < int(SourceLanguage::CountOf));
        *outPrelude =
            Slang::StringUtil::createStringBlob(m_languagePreludes[int(sourceLanguage)]).detach();
    }
}

SLANG_NO_THROW const char* SLANG_MCALL Session::getBuildTagString()
{
    return ::Slang::getBuildTagString();
}

SLANG_NO_THROW SlangResult SLANG_MCALL Session::setDefaultDownstreamCompiler(
    SlangSourceLanguage sourceLanguage,
    SlangPassThrough defaultCompiler)
{
    if (DownstreamCompilerInfo::canCompile(defaultCompiler, sourceLanguage))
    {
        m_defaultDownstreamCompilers[int(sourceLanguage)] = PassThroughMode(defaultCompiler);
        return SLANG_OK;
    }
    return SLANG_FAIL;
}

SlangPassThrough SLANG_MCALL
Session::getDefaultDownstreamCompiler(SlangSourceLanguage inSourceLanguage)
{
    SLANG_ASSERT(inSourceLanguage >= 0 && inSourceLanguage < SLANG_SOURCE_LANGUAGE_COUNT_OF);
    auto sourceLanguage = SourceLanguage(inSourceLanguage);
    return SlangPassThrough(m_defaultDownstreamCompilers[int(sourceLanguage)]);
}

void Session::setDownstreamCompilerForTransition(
    SlangCompileTarget source,
    SlangCompileTarget target,
    SlangPassThrough compiler)
{
    if (compiler == SLANG_PASS_THROUGH_NONE)
    {
        // Removing the transition means a default can be used
        m_codeGenTransitionMap.removeTransition(CodeGenTarget(source), CodeGenTarget(target));
    }
    else
    {
        m_codeGenTransitionMap.addTransition(
            CodeGenTarget(source),
            CodeGenTarget(target),
            PassThroughMode(compiler));
    }
}

SlangPassThrough Session::getDownstreamCompilerForTransition(
    SlangCompileTarget inSource,
    SlangCompileTarget inTarget)
{
    const CodeGenTarget source = CodeGenTarget(inSource);
    const CodeGenTarget target = CodeGenTarget(inTarget);

    if (m_codeGenTransitionMap.hasTransition(source, target))
    {
        return (SlangPassThrough)m_codeGenTransitionMap.getTransition(source, target);
    }

    const auto desc = ArtifactDescUtil::makeDescForCompileTarget(inTarget);

    // Special case host-callable
    if ((desc.kind == ArtifactKind::HostCallable) &&
        (source == CodeGenTarget::CSource || source == CodeGenTarget::CPPSource))
    {
        // We prefer LLVM if it's available
        if (const auto llvm = getOrLoadDownstreamCompiler(PassThroughMode::LLVM, nullptr))
        {
            return SLANG_PASS_THROUGH_LLVM;
        }
    }

    // Use the legacy 'sourceLanguage' default mechanism.
    // This says nothing about the target type, so it is *assumed* the target type is possible
    // If not it will fail when trying to compile to an unknown target
    const SourceLanguage sourceLanguage =
        (SourceLanguage)TypeConvertUtil::getSourceLanguageFromTarget(inSource);
    if (sourceLanguage != SourceLanguage::Unknown)
    {
        return getDefaultDownstreamCompiler(SlangSourceLanguage(sourceLanguage));
    }

    // Unknwon
    return SLANG_PASS_THROUGH_NONE;
}

IDownstreamCompiler* Session::getDownstreamCompiler(CodeGenTarget source, CodeGenTarget target)
{
    PassThroughMode compilerType = (PassThroughMode)getDownstreamCompilerForTransition(
        SlangCompileTarget(source),
        SlangCompileTarget(target));
    return getOrLoadDownstreamCompiler(compilerType, nullptr);
}

SLANG_NO_THROW SlangResult SLANG_MCALL Session::setSPIRVCoreGrammar(char const* jsonPath)
{
    if (!jsonPath)
    {
        spirvCoreGrammarInfo = SPIRVCoreGrammarInfo::getEmbeddedVersion();
        SLANG_ASSERT(spirvCoreGrammarInfo);
    }
    else
    {
        SourceManager* sourceManager = getBuiltinSourceManager();
        SLANG_ASSERT(sourceManager);
        DiagnosticSink sink(sourceManager, Lexer::sourceLocationLexer);

        String contents;
        const auto readRes = File::readAllText(jsonPath, contents);
        if (SLANG_FAILED(readRes))
        {
            sink.diagnose(SourceLoc{}, Diagnostics::unableToReadFile, jsonPath);
            return readRes;
        }
        const auto pathInfo = PathInfo::makeFromString(jsonPath);
        const auto sourceFile = sourceManager->createSourceFileWithString(pathInfo, contents);
        const auto sourceView = sourceManager->createSourceView(sourceFile, nullptr, SourceLoc());
        spirvCoreGrammarInfo = SPIRVCoreGrammarInfo::loadFromJSON(*sourceView, sink);
    }
    return spirvCoreGrammarInfo ? SLANG_OK : SLANG_FAIL;
}

struct ParsedCommandLineData : public ISlangUnknown, public ComObject
{
    SLANG_COM_OBJECT_IUNKNOWN_ALL

    ISlangUnknown* getInterface(const Slang::Guid& guid)
    {
        if (guid == ISlangUnknown::getTypeGuid())
            return this;
        return nullptr;
    }
    List<SerializedOptionsData> options;
    List<slang::TargetDesc> targets;
};

SLANG_NO_THROW SlangResult SLANG_MCALL Session::parseCommandLineArguments(
    int argc,
    const char* const* argv,
    slang::SessionDesc* outDesc,
    ISlangUnknown** outAllocation)
{
    if (outDesc->structureSize < sizeof(slang::SessionDesc))
        return SLANG_E_BUFFER_TOO_SMALL;
    RefPtr<ParsedCommandLineData> outData = new ParsedCommandLineData();
    RefPtr<EndToEndCompileRequest> tempReq = new EndToEndCompileRequest(this);
    tempReq->processCommandLineArguments(argv, argc);
    outData->options.setCount(1 + tempReq->getLinkage()->targets.getCount());
    int optionDataIndex = 0;
    SerializedOptionsData& optionData = outData->options[optionDataIndex];
    optionDataIndex++;
    tempReq->getOptionSet().serialize(&optionData);
    tempReq->m_optionSetForDefaultTarget.serialize(&optionData);
    for (auto target : tempReq->getLinkage()->targets)
    {
        slang::TargetDesc tdesc;
        SerializedOptionsData& targetOptionData = outData->options[optionDataIndex];
        optionDataIndex++;
        tempReq->getTargetOptionSet(target).serialize(&targetOptionData);
        tdesc.compilerOptionEntryCount = (uint32_t)targetOptionData.entries.getCount();
        tdesc.compilerOptionEntries = targetOptionData.entries.getBuffer();
        tdesc.format = (SlangCompileTarget)target->getTarget();
        tdesc.profile = (SlangProfileID)target->getOptionSet().getProfile().raw;
        outData->targets.add(tdesc);
    }
    outDesc->compilerOptionEntryCount = (uint32_t)optionData.entries.getCount();
    outDesc->compilerOptionEntries = optionData.entries.getBuffer();
    outDesc->targetCount = outData->targets.getCount();
    outDesc->targets = outData->targets.getBuffer();
    *outAllocation = outData.get();
    outData->addRef();
    return SLANG_OK;
}

SLANG_NO_THROW SlangResult SLANG_MCALL
Session::getSessionDescDigest(slang::SessionDesc* sessionDesc, ISlangBlob** outBlob)
{
    ComPtr<slang::ISession> tempSession;
    createSession(*sessionDesc, tempSession.writeRef());
    auto linkage = static_cast<Linkage*>(tempSession.get());
    DigestBuilder<SHA1> digestBuilder;
    linkage->buildHash(digestBuilder, -1);
    auto blob = digestBuilder.finalize().toBlob();
    *outBlob = blob.detach();
    return SLANG_OK;
}

void Session::addBuiltinSource(
    Scope* scope,
    String const& path,
    ISlangBlob* sourceBlob,
    Module*& outModule)
{
    SourceManager* sourceManager = getBuiltinSourceManager();

    DiagnosticSink sink(sourceManager, Lexer::sourceLocationLexer);

    RefPtr<FrontEndCompileRequest> compileRequest =
        new FrontEndCompileRequest(m_builtinLinkage, nullptr, &sink);
    compileRequest->m_isCoreModuleCode = true;

    // Set the source manager on the sink
    sink.setSourceManager(sourceManager);
    // Make the linkage use the builtin source manager
    Linkage* linkage = compileRequest->getLinkage();
    linkage->setSourceManager(sourceManager);

    Name* moduleName = getNamePool()->getName(path);
    auto translationUnitIndex =
        compileRequest->addTranslationUnit(SourceLanguage::Slang, moduleName);

    compileRequest->addTranslationUnitSourceBlob(translationUnitIndex, path, sourceBlob);

    SlangResult res = compileRequest->executeActionsInner();
    if (SLANG_FAILED(res))
    {
        char const* diagnostics = sink.outputBuffer.getBuffer();
        fprintf(stderr, "%s", diagnostics);

        PlatformUtil::outputDebugMessage(diagnostics);

        SLANG_UNEXPECTED("error in Slang core module");
    }

    if (sink.outputBuffer.getLength())
    {
        char const* diagnostics = sink.outputBuffer.getBuffer();
        fprintf(stderr, "%s", diagnostics);

        PlatformUtil::outputDebugMessage(diagnostics);
        SLANG_UNEXPECTED("Compiling the core module should not yield any warnings");
    }

    // Extract the AST for the code we just parsed
    auto module = compileRequest->translationUnits[translationUnitIndex]->getModule();
    auto moduleDecl = module->getModuleDecl();

    // Extact documentation markup.
    ASTMarkup markup;
    ASTMarkupUtil::extract(moduleDecl, sourceManager, &sink, &markup);
    markup.attachToAST();

    // Put in the loaded module map
    linkage->mapNameToLoadedModules.add(moduleName, module);

    // Add the resulting code to the appropriate scope
    if (!scope->containerDecl)
    {
        // We are the first chunk of code to be loaded for this scope
        scope->containerDecl = moduleDecl;
    }
    else
    {
        // We need to create a new scope to link into the whole thing
        auto subScope = module->getASTBuilder()->create<Scope>();
        subScope->containerDecl = moduleDecl;
        subScope->nextSibling = scope->nextSibling;
        scope->nextSibling = subScope;
    }

    outModule = module;
}

SlangResult checkExternalCompilerSupport(Session* session, PassThroughMode passThrough)
{
    // Check if the type is supported on this compile
    if (passThrough == PassThroughMode::None)
    {
        // If no pass through -> that will always work!
        return SLANG_OK;
    }

    return session->getOrLoadDownstreamCompiler(passThrough, nullptr) ? SLANG_OK
                                                                      : SLANG_E_NOT_FOUND;
}

} // namespace Slang
