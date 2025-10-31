// slang-session.cpp
#include "slang-session.h"

#include "../core/slang-shared-library.h"
#include "compiler-core/slang-artifact-util.h"
#include "slang-check-impl.h"
#include "slang-compiler.h"
#include "slang-lower-to-ir.h"
#include "slang-mangle.h"
#include "slang-options.h"
#include "slang-parser.h"
#include "slang-preprocessor.h"
#include "slang-serialize-ast.h"
#include "slang-serialize-container.h"
#include "slang-serialize-ir.h"
#include "slang-standard-module-config.h"

namespace Slang
{

// Helper function to find the neural.slang module path
static String getStandardModuleDirPath()
{
    // Get the path of the currently loaded libslang.so/slang.dll by using a known exported symbol
    String libslangPath =
        SharedLibraryUtils::getSharedLibraryFileName((void*)slang_createGlobalSession);
    if (libslangPath.getLength() == 0)
        return String();

    // Get the directory containing libslang.so/slang.dll
    String libslangDir = Path::getParentDirectory(libslangPath);
    if (libslangDir.getLength() == 0)
        return String();

    // TODO: Change this to SLANG_STANDARD_MODULE_DIR_NAME directory if we add more standard modules
    String stdModuleDirPath = Path::combine(libslangDir, SLANG_STANDARD_MODULE_DIR_NAME);
    return stdModuleDirPath;
}

static String findStandardModulePath(String const& stdModuleDirPath, String const& moduleName)
{
    // The neural module is always in the same directory as libslang.so/slang.dll
    // e.g., bin/slang-standard-module/ on Windows, lib/slang-standard-module/ on Linux/Mac
    String stdModulePath = Path::combine(stdModuleDirPath, moduleName + ".slang-module");

    if (File::exists(stdModulePath))
        return stdModulePath;

    return String();
}

Linkage::Linkage(Session* session, ASTBuilder* astBuilder, Linkage* builtinLinkage)
    : m_session(session)
    , m_retainedSession(session)
    , m_sourceManager(&m_defaultSourceManager)
    , m_astBuilder(astBuilder)
    , m_cmdLineContext(new CommandLineContext())
    , m_stringSlicePool(StringSlicePool::Style::Default)
{
    namePool = session->getNamePool();

    m_defaultSourceManager.initialize(session->getBuiltinSourceManager(), nullptr);

    setFileSystem(nullptr);

    // Copy of the built in linkages modules
    if (builtinLinkage)
    {
        for (const auto& nameToMod : builtinLinkage->mapNameToLoadedModules)
            mapNameToLoadedModules.add(nameToMod);
    }

    m_semanticsForReflection = new SharedSemanticsContext(this, nullptr, nullptr);
}

SharedSemanticsContext* Linkage::getSemanticsForReflection()
{
    return m_semanticsForReflection.get();
}

ISlangUnknown* Linkage::getInterface(const Guid& guid)
{
    if (guid == ISlangUnknown::getTypeGuid() || guid == ISession::getTypeGuid())
        return asExternal(this);

    return nullptr;
}

Linkage::~Linkage()
{
    // Upstream type checking cache.
    if (m_typeCheckingCache)
    {
        auto globalSession = getSessionImpl();
        std::lock_guard<std::mutex> lock(globalSession->m_typeCheckingCacheMutex);
        if (!globalSession->m_typeCheckingCache ||
            globalSession->getTypeCheckingCache()->resolvedOperatorOverloadCache.getCount() <
                getTypeCheckingCache()->resolvedOperatorOverloadCache.getCount())
        {
            globalSession->m_typeCheckingCache = m_typeCheckingCache;
            getTypeCheckingCache()->version++;
        }
        destroyTypeCheckingCache();
    }
}

SearchDirectoryList& Linkage::getSearchDirectories()
{
    auto list = m_optionSet.getArray(CompilerOptionName::Include);
    if (list.getCount() != searchDirectoryCache.searchDirectories.getCount())
    {
        searchDirectoryCache.searchDirectories.clear();
        for (auto dir : list)
            searchDirectoryCache.searchDirectories.add(SearchDirectory(dir.stringValue));
    }
    return searchDirectoryCache;
}

TypeCheckingCache* Linkage::getTypeCheckingCache()
{
    if (!m_typeCheckingCache)
    {
        m_typeCheckingCache = new TypeCheckingCache();
    }
    return static_cast<TypeCheckingCache*>(m_typeCheckingCache.get());
}

void Linkage::destroyTypeCheckingCache()
{
    m_typeCheckingCache = nullptr;
}

SLANG_NO_THROW slang::IGlobalSession* SLANG_MCALL Linkage::getGlobalSession()
{
    return asExternal(getSessionImpl());
}

void Linkage::addTarget(slang::TargetDesc const& desc)
{
    SLANG_AST_BUILDER_RAII(getASTBuilder());

    auto targetIndex = addTarget(CodeGenTarget(desc.format));
    auto target = targets[targetIndex];

    auto& optionSet = target->getOptionSet();
    optionSet.inheritFrom(m_optionSet);

    optionSet.set(CompilerOptionName::FloatingPointMode, FloatingPointMode(desc.floatingPointMode));
    optionSet.addTargetFlags(desc.flags);
    optionSet.setProfile(Profile(desc.profile));
    optionSet.set(CompilerOptionName::LineDirectiveMode, LineDirectiveMode(desc.lineDirectiveMode));
    optionSet.set(CompilerOptionName::GLSLForceScalarLayout, desc.forceGLSLScalarBufferLayout);

    CompilerOptionSet targetOptions;
    targetOptions.load(desc.compilerOptionEntryCount, desc.compilerOptionEntries);
    optionSet.overrideWith(targetOptions);
}

SLANG_NO_THROW slang::IModule* SLANG_MCALL
Linkage::loadModule(const char* moduleName, slang::IBlob** outDiagnostics)
{
    SLANG_AST_BUILDER_RAII(getASTBuilder());

    DiagnosticSink sink(getSourceManager(), Lexer::sourceLocationLexer);
    applySettingsToDiagnosticSink(&sink, &sink, m_optionSet);

    if (isInLanguageServer())
    {
        sink.setFlags(DiagnosticSink::Flag::HumaneLoc | DiagnosticSink::Flag::LanguageServer);
    }

    try
    {
        auto name = getNamePool()->getName(moduleName);

        auto module = findOrImportModule(name, SourceLoc(), &sink);
        sink.getBlobIfNeeded(outDiagnostics);

        return asExternal(module);
    }
    catch (const AbortCompilationException& e)
    {
        outputExceptionDiagnostic(e, sink, outDiagnostics);
        return nullptr;
    }
    catch (const Exception& e)
    {
        outputExceptionDiagnostic(e, sink, outDiagnostics);
        return nullptr;
    }
    catch (...)
    {
        outputExceptionDiagnostic(sink, outDiagnostics);
        return nullptr;
    }
}

slang::IModule* Linkage::loadModuleFromBlob(
    const char* moduleName,
    const char* path,
    slang::IBlob* source,
    ModuleBlobType blobType,
    slang::IBlob** outDiagnostics)
{
    SLANG_AST_BUILDER_RAII(getASTBuilder());

    DiagnosticSink sink(getSourceManager(), Lexer::sourceLocationLexer);
    applySettingsToDiagnosticSink(&sink, &sink, m_optionSet);

    if (isInLanguageServer())
    {
        sink.setFlags(DiagnosticSink::Flag::HumaneLoc | DiagnosticSink::Flag::LanguageServer);
    }


    try
    {
        auto getDigestStr = [](auto x)
        {
            DigestBuilder<SHA1> digestBuilder;
            digestBuilder.append(x);
            return digestBuilder.finalize().toString();
        };

        String moduleNameStr = moduleName;
        if (!moduleName)
            moduleNameStr = getDigestStr(source);

        auto name = getNamePool()->getName(moduleNameStr);
        RefPtr<LoadedModule> loadedModule;
        if (mapNameToLoadedModules.tryGetValue(name, loadedModule))
        {
            return loadedModule;
        }
        String pathStr = path;
        if (pathStr.getLength() == 0)
        {
            // If path is empty, use a digest from source as path.
            pathStr = getDigestStr(source);
        }
        auto pathInfo = PathInfo::makeFromString(pathStr);
        if (File::exists(pathStr))
        {
            String cannonicalPath;
            if (SLANG_SUCCEEDED(Path::getCanonical(pathStr, cannonicalPath)))
            {
                pathInfo = PathInfo::makeNormal(pathStr, cannonicalPath);
            }
        }
        RefPtr<Module> module =
            loadModuleImpl(name, pathInfo, source, SourceLoc(), &sink, nullptr, blobType);
        sink.getBlobIfNeeded(outDiagnostics);
        return asExternal(module.get());
    }
    catch (const AbortCompilationException& e)
    {
        outputExceptionDiagnostic(e, sink, outDiagnostics);
        return nullptr;
    }
    catch (const Exception& e)
    {
        outputExceptionDiagnostic(e, sink, outDiagnostics);
        return nullptr;
    }
    catch (...)
    {
        outputExceptionDiagnostic(sink, outDiagnostics);
        return nullptr;
    }
}

SLANG_NO_THROW slang::IModule* SLANG_MCALL Linkage::loadModuleFromSource(
    const char* moduleName,
    const char* path,
    slang::IBlob* source,
    slang::IBlob** outDiagnostics)
{
    return loadModuleFromBlob(moduleName, path, source, ModuleBlobType::Source, outDiagnostics);
}

SLANG_NO_THROW slang::IModule* SLANG_MCALL Linkage::loadModuleFromSourceString(
    const char* moduleName,
    const char* path,
    const char* source,
    slang::IBlob** outDiagnostics)
{
    auto sourceBlob = StringBlob::create(UnownedStringSlice(source));
    return loadModuleFromSource(moduleName, path, sourceBlob.get(), outDiagnostics);
}

SLANG_NO_THROW slang::IModule* SLANG_MCALL Linkage::loadModuleFromIRBlob(
    const char* moduleName,
    const char* path,
    slang::IBlob* source,
    slang::IBlob** outDiagnostics)
{
    return loadModuleFromBlob(moduleName, path, source, ModuleBlobType::IR, outDiagnostics);
}

SLANG_NO_THROW SlangResult SLANG_MCALL Linkage::loadModuleInfoFromIRBlob(
    slang::IBlob* source,
    SlangInt& outModuleVersion,
    const char*& outModuleCompilerVersion,
    const char*& outModuleName)
{
    // We start by reading the content of the file as
    // an in-memory RIFF container.
    //
    auto rootChunk = RIFF::RootChunk::getFromBlob(source);
    if (!rootChunk)
    {
        return SLANG_FAIL;
    }

    auto moduleChunk = ModuleChunk::find(rootChunk);
    if (!moduleChunk)
    {
        return SLANG_FAIL;
    }

    auto irChunk = moduleChunk->findIR();
    if (!irChunk)
    {
        return SLANG_FAIL;
    }

    RefPtr<IRModule> irModule;
    String compilerVersion;
    UInt version;
    String name;
    SLANG_RETURN_ON_FAIL(readSerializedModuleInfo(irChunk, compilerVersion, version, name));
    const auto compilerVersionSlice = m_stringSlicePool.addAndGetSlice(compilerVersion);
    const auto nameSlice = m_stringSlicePool.addAndGetSlice(name);
    outModuleCompilerVersion = compilerVersionSlice.begin();
    outModuleName = nameSlice.begin();
    outModuleVersion = SlangInt(version);

    return SLANG_OK;
}

SLANG_NO_THROW SlangResult SLANG_MCALL Linkage::createCompositeComponentType(
    slang::IComponentType* const* componentTypes,
    SlangInt componentTypeCount,
    slang::IComponentType** outCompositeComponentType,
    ISlangBlob** outDiagnostics)
{
    if (outCompositeComponentType == nullptr)
        return SLANG_E_INVALID_ARG;

    SLANG_AST_BUILDER_RAII(getASTBuilder());

    // Attempting to create a "composite" of just one component type should
    // just return the component type itself, to avoid redundant work.
    //
    if (componentTypeCount == 1)
    {
        auto componentType = componentTypes[0];
        componentType->addRef();
        *outCompositeComponentType = componentType;
        return SLANG_OK;
    }

    DiagnosticSink sink(getSourceManager(), Lexer::sourceLocationLexer);
    applySettingsToDiagnosticSink(&sink, &sink, m_optionSet);

    List<RefPtr<ComponentType>> childComponents;
    for (Int cc = 0; cc < componentTypeCount; ++cc)
    {
        childComponents.add(asInternal(componentTypes[cc]));
    }

    RefPtr<ComponentType> composite = CompositeComponentType::create(this, childComponents);

    sink.getBlobIfNeeded(outDiagnostics);

    *outCompositeComponentType = asExternal(composite.detach());
    return SLANG_OK;
}

SLANG_NO_THROW slang::TypeReflection* SLANG_MCALL Linkage::specializeType(
    slang::TypeReflection* inUnspecializedType,
    slang::SpecializationArg const* specializationArgs,
    SlangInt specializationArgCount,
    ISlangBlob** outDiagnostics)
{
    SLANG_AST_BUILDER_RAII(getASTBuilder());

    auto unspecializedType = asInternal(inUnspecializedType);

    List<Type*> typeArgs;

    for (Int ii = 0; ii < specializationArgCount; ++ii)
    {
        auto& arg = specializationArgs[ii];
        if (arg.kind != slang::SpecializationArg::Kind::Type)
            return nullptr;

        typeArgs.add(asInternal(arg.type));
    }

    DiagnosticSink sink(getSourceManager(), Lexer::sourceLocationLexer);
    auto specializedType =
        specializeType(unspecializedType, typeArgs.getCount(), typeArgs.getBuffer(), &sink);
    sink.getBlobIfNeeded(outDiagnostics);

    return asExternal(specializedType);
}

DeclRef<GenericDecl> getGenericParentDeclRef(
    ASTBuilder* astBuilder,
    SemanticsVisitor* visitor,
    DeclRef<Decl> declRef)
{
    // Create substituted parent decl ref.
    auto decl = declRef.getDecl();

    while (decl && !as<GenericDecl>(decl))
    {
        decl = decl->parentDecl;
    }

    if (!decl)
    {
        // No generic parent
        return DeclRef<GenericDecl>();
    }

    auto genericDecl = as<GenericDecl>(decl);
    auto genericDeclRef =
        createDefaultSubstitutionsIfNeeded(astBuilder, visitor, DeclRef(genericDecl))
            .as<GenericDecl>();
    return substituteDeclRef(SubstitutionSet(declRef), astBuilder, genericDeclRef)
        .as<GenericDecl>();
}

bool Linkage::isSpecialized(DeclRef<Decl> declRef)
{
    // For now, we only support two 'states': fully applied or not at all.
    // If we add support for partial specialization, we will need to update this logic.
    //
    // If it's not specialized, then declRef will be the one with default substitutions.
    //
    SemanticsVisitor visitor(getSemanticsForReflection());

    auto decl = declRef.getDecl();
    while (decl && !as<GenericDecl>(decl))
    {
        decl = decl->parentDecl;
    }

    if (!decl)
        return true; // no generics => always specialized

    auto defaultArgs = getDefaultSubstitutionArgs(getASTBuilder(), &visitor, as<GenericDecl>(decl));
    auto currentArgs =
        SubstitutionSet(declRef).findGenericAppDeclRef(as<GenericDecl>(decl))->getArgs();

    if (defaultArgs.getCount() != currentArgs.getCount()) // should really never happen.
        return true;

    for (Index i = 0; i < defaultArgs.getCount(); ++i)
    {
        if (defaultArgs[i] != currentArgs[i])
            return true;
    }

    return false;
}

bool isFuncGeneric(DeclRef<Decl> declRef)
{
    if (auto funcDecl = as<FuncDecl>(declRef.getDecl()))
    {
        if (funcDecl->parentDecl && as<GenericDecl>(funcDecl->parentDecl))
        {
            return true;
        }
    }

    return false;
}

DeclRef<Decl> Linkage::specializeWithArgTypes(
    Expr* funcExpr,
    List<Type*> argTypes,
    DiagnosticSink* sink)
{
    SemanticsVisitor visitor(getSemanticsForReflection());
    SemanticsVisitor::ExprLocalScope scope;
    visitor = visitor.withSink(sink).withExprLocalScope(&scope);

    SLANG_AST_BUILDER_RAII(getASTBuilder());

    if (auto declRefFuncExpr = as<DeclRefExpr>(funcExpr))
    {
        if (isFuncGeneric(declRefFuncExpr->declRef) && !isSpecialized(declRefFuncExpr->declRef))
        {
            if (auto genericDeclRef = getGenericParentDeclRef(
                    getCurrentASTBuilder(),
                    &visitor,
                    declRefFuncExpr->declRef))
            {
                auto genericDeclRefExpr = getCurrentASTBuilder()->create<DeclRefExpr>();
                genericDeclRefExpr->declRef = genericDeclRef;
                funcExpr = genericDeclRefExpr;
            }
        }
    }

    List<Expr*> argExprs;
    for (SlangInt aa = 0; aa < argTypes.getCount(); ++aa)
    {
        auto argType = argTypes[aa];

        // Create an 'empty' expr with the given type. Ideally, the expression itself should not
        // matter only its checked type.
        //
        auto argExpr = getCurrentASTBuilder()->create<VarExpr>();
        argExpr->type = argType;
        argExpr->type.isLeftValue = true;
        argExprs.add(argExpr);
    }

    // Construct invoke expr.
    auto invokeExpr = getCurrentASTBuilder()->create<InvokeExpr>();
    invokeExpr->functionExpr = funcExpr;
    invokeExpr->arguments = argExprs;

    auto checkedInvokeExpr = visitor.CheckInvokeExprWithCheckedOperands(invokeExpr);

    return as<DeclRefExpr>(as<InvokeExpr>(checkedInvokeExpr)->functionExpr)->declRef;
}


DeclRef<Decl> Linkage::specializeGeneric(
    DeclRef<Decl> declRef,
    List<Expr*> argExprs,
    DiagnosticSink* sink)
{
    SLANG_AST_BUILDER_RAII(getASTBuilder());
    SLANG_ASSERT(declRef);

    SemanticsVisitor visitor(getSemanticsForReflection());
    visitor = visitor.withSink(sink);

    auto genericDeclRef = getGenericParentDeclRef(getASTBuilder(), &visitor, declRef);

    DeclRefExpr* declRefExpr = getASTBuilder()->create<DeclRefExpr>();
    declRefExpr->declRef = genericDeclRef;

    GenericAppExpr* genericAppExpr = getASTBuilder()->create<GenericAppExpr>();
    genericAppExpr->functionExpr = declRefExpr;
    genericAppExpr->arguments = argExprs;

    auto specializedDeclRef =
        as<DeclRefExpr>(visitor.checkGenericAppWithCheckedArgs(genericAppExpr))->declRef;

    return specializedDeclRef;
}

SLANG_NO_THROW slang::TypeLayoutReflection* SLANG_MCALL Linkage::getTypeLayout(
    slang::TypeReflection* inType,
    SlangInt targetIndex,
    slang::LayoutRules rules,
    ISlangBlob** outDiagnostics)
{
    SLANG_AST_BUILDER_RAII(getASTBuilder());

    auto type = asInternal(inType);

    if (targetIndex < 0 || targetIndex >= targets.getCount())
        return nullptr;

    auto target = targets[targetIndex];

    // TODO: We need a way to pass through the layout rules
    // that the user requested (e.g., constant buffers vs.
    // structured buffer rules). Right now the API only
    // exposes a single case, so this isn't a big deal.
    //
    SLANG_UNUSED(rules);

    auto typeLayout = target->getTypeLayout(type, rules);

    // TODO: We currently don't have a path for capturing
    // errors that occur during layout (e.g., types that
    // are invalid because of target-specific layout constraints).
    //
    SLANG_UNUSED(outDiagnostics);

    return asExternal(typeLayout);
}

SLANG_NO_THROW slang::TypeReflection* SLANG_MCALL Linkage::getContainerType(
    slang::TypeReflection* inType,
    slang::ContainerType containerType,
    ISlangBlob** outDiagnostics)
{
    SLANG_AST_BUILDER_RAII(getASTBuilder());

    auto type = asInternal(inType);

    Type* containerTypeReflection = nullptr;
    ContainerTypeKey key = {inType, containerType};
    if (!m_containerTypes.tryGetValue(key, containerTypeReflection))
    {
        switch (containerType)
        {
        case slang::ContainerType::ConstantBuffer:
            {
                SemanticsVisitor visitor(getSemanticsForReflection());
                auto layoutType = getASTBuilder()->getDefaultLayoutType();
                Type* cbType = visitor.getConstantBufferType(type, layoutType);
                containerTypeReflection = cbType;
            }
            break;
        case slang::ContainerType::ParameterBlock:
            {
                ParameterBlockType* pbType = getASTBuilder()->getParameterBlockType(type);
                containerTypeReflection = pbType;
            }
            break;
        case slang::ContainerType::StructuredBuffer:
            {
                HLSLStructuredBufferType* sbType = getASTBuilder()->getStructuredBufferType(type);
                containerTypeReflection = sbType;
            }
            break;
        case slang::ContainerType::UnsizedArray:
            {
                ArrayExpressionType* arrType = getASTBuilder()->getArrayType(type, nullptr);
                containerTypeReflection = arrType;
            }
            break;
        default:
            containerTypeReflection = type;
            break;
        }

        m_containerTypes.add(key, containerTypeReflection);
    }

    SLANG_UNUSED(outDiagnostics);

    return asExternal(containerTypeReflection);
}

SLANG_NO_THROW slang::TypeReflection* SLANG_MCALL Linkage::getDynamicType()
{
    SLANG_AST_BUILDER_RAII(getASTBuilder());

    return asExternal(getASTBuilder()->getSharedASTBuilder()->getDynamicType());
}

SLANG_NO_THROW SlangResult SLANG_MCALL
Linkage::getTypeRTTIMangledName(slang::TypeReflection* type, ISlangBlob** outNameBlob)
{
    SLANG_AST_BUILDER_RAII(getASTBuilder());

    auto internalType = asInternal(type);
    if (auto declRefType = as<DeclRefType>(internalType))
    {
        auto name = getMangledName(m_astBuilder, declRefType->getDeclRef());
        Slang::ComPtr<ISlangBlob> blob = Slang::StringUtil::createStringBlob(name);
        *outNameBlob = blob.detach();
        return SLANG_OK;
    }
    return SLANG_FAIL;
}

SLANG_NO_THROW SlangResult SLANG_MCALL Linkage::getTypeConformanceWitnessMangledName(
    slang::TypeReflection* type,
    slang::TypeReflection* interfaceType,
    ISlangBlob** outNameBlob)
{
    SLANG_AST_BUILDER_RAII(getASTBuilder());

    auto subType = asInternal(type);
    auto supType = asInternal(interfaceType);
    auto name = getMangledNameForConformanceWitness(m_astBuilder, subType, supType);
    Slang::ComPtr<ISlangBlob> blob = Slang::StringUtil::createStringBlob(name);
    *outNameBlob = blob.detach();
    return SLANG_OK;
}

SLANG_NO_THROW SlangResult SLANG_MCALL Linkage::getTypeConformanceWitnessSequentialID(
    slang::TypeReflection* type,
    slang::TypeReflection* interfaceType,
    uint32_t* outId)
{
    SLANG_AST_BUILDER_RAII(getASTBuilder());

    auto subType = asInternal(type);
    auto supType = asInternal(interfaceType);

    if (!subType || !supType)
        return SLANG_FAIL;

    auto name = getMangledNameForConformanceWitness(m_astBuilder, subType, supType);
    auto interfaceName = getMangledTypeName(m_astBuilder, supType);
    uint32_t resultIndex = 0;
    if (mapMangledNameToRTTIObjectIndex.tryGetValue(name, resultIndex))
    {
        if (outId)
            *outId = resultIndex;
        return SLANG_OK;
    }
    auto idAllocator = mapInterfaceMangledNameToSequentialIDCounters.tryGetValue(interfaceName);
    if (!idAllocator)
    {
        mapInterfaceMangledNameToSequentialIDCounters[interfaceName] = 0;
        idAllocator = mapInterfaceMangledNameToSequentialIDCounters.tryGetValue(interfaceName);
    }
    resultIndex = (*idAllocator);
    ++(*idAllocator);
    mapMangledNameToRTTIObjectIndex[name] = resultIndex;
    if (outId)
        *outId = resultIndex;
    return SLANG_OK;
}

SLANG_NO_THROW SlangResult SLANG_MCALL Linkage::getDynamicObjectRTTIBytes(
    slang::TypeReflection* type,
    slang::TypeReflection* interfaceType,
    uint32_t* outBuffer,
    uint32_t bufferSize)
{
    // Slang RTTI header format:
    // byte 0-7: pointer to RTTI struct describing the type. (not used for now, set to 1 for valid
    // types, and 0 to represent null).
    // byte 8-11: 32-bit sequential ID of the type conformance witness.
    // byte 12-15: unused.

    if (bufferSize < 16)
        return SLANG_E_BUFFER_TOO_SMALL;

    SLANG_AST_BUILDER_RAII(getASTBuilder());

    SLANG_RETURN_ON_FAIL(getTypeConformanceWitnessSequentialID(type, interfaceType, outBuffer + 2));

    // Make the RTTI part non zero.
    outBuffer[0] = 1;

    return SLANG_OK;
}

SLANG_NO_THROW SlangResult SLANG_MCALL Linkage::createTypeConformanceComponentType(
    slang::TypeReflection* type,
    slang::TypeReflection* interfaceType,
    slang::ITypeConformance** outConformanceComponentType,
    SlangInt conformanceIdOverride,
    ISlangBlob** outDiagnostics)
{
    if (outConformanceComponentType == nullptr)
        return SLANG_E_INVALID_ARG;

    SLANG_AST_BUILDER_RAII(getASTBuilder());

    RefPtr<TypeConformance> result;
    DiagnosticSink sink;
    applySettingsToDiagnosticSink(&sink, &sink, m_optionSet);

    try
    {
        SemanticsVisitor visitor(getSemanticsForReflection());
        visitor = visitor.withSink(&sink);

        auto witness = visitor.isSubtype(
            (Slang::Type*)type,
            (Slang::Type*)interfaceType,
            IsSubTypeOptions::None);
        if (auto subtypeWitness = as<SubtypeWitness>(witness))
        {
            result = new TypeConformance(this, subtypeWitness, conformanceIdOverride, &sink);
        }
    }
    catch (...)
    {
    }
    sink.getBlobIfNeeded(outDiagnostics);
    bool success = (result != nullptr);
    *outConformanceComponentType = result.detach();
    return success ? SLANG_OK : SLANG_FAIL;
}

SLANG_NO_THROW SlangResult SLANG_MCALL
Linkage::createCompileRequest(SlangCompileRequest** outCompileRequest)
{
    auto compileRequest = new EndToEndCompileRequest(this);
    compileRequest->addRef();
    *outCompileRequest = asExternal(compileRequest);
    return SLANG_OK;
}

SLANG_NO_THROW SlangInt SLANG_MCALL Linkage::getLoadedModuleCount()
{
    return loadedModulesList.getCount();
}

SLANG_NO_THROW slang::IModule* SLANG_MCALL Linkage::getLoadedModule(SlangInt index)
{
    if (index >= 0 && index < loadedModulesList.getCount())
        return loadedModulesList[index].get();
    return nullptr;
}

void Linkage::buildHash(DigestBuilder<SHA1>& builder, SlangInt targetIndex)
{
    // Add the Slang compiler version to the hash
    auto version = String(getBuildTagString());
    builder.append(version);

    // Add compiler options, including search path, preprocessor includes, etc.
    m_optionSet.buildHash(builder);

    auto addTargetDigest = [&](TargetRequest* targetReq)
    {
        targetReq->getOptionSet().buildHash(builder);

        const PassThroughMode passThroughMode =
            getDownstreamCompilerRequiredForTarget(targetReq->getTarget());
        const SourceLanguage sourceLanguage =
            getDefaultSourceLanguageForDownstreamCompiler(passThroughMode);

        // Add prelude for the given downstream compiler.
        ComPtr<ISlangBlob> prelude;
        getGlobalSession()->getLanguagePrelude(
            (SlangSourceLanguage)sourceLanguage,
            prelude.writeRef());
        if (prelude)
        {
            builder.append(prelude);
        }

        // TODO: Downstream compilers (specifically dxc) can currently #include additional
        // dependencies. This is currently the case for NVAPI headers included in the prelude. These
        // dependencies are currently not picked up by the shader cache which is a significant
        // issue. This can only be fixed by running the preprocessor in the slang compiler so dxc
        // (or any other downstream compiler for that matter) isn't resolving any includes
        // implicitly.

        // Add the downstream compiler version (if it exists) to the hash
        auto downstreamCompiler =
            getSessionImpl()->getOrLoadDownstreamCompiler(passThroughMode, nullptr);
        if (downstreamCompiler)
        {
            ComPtr<ISlangBlob> versionString;
            if (SLANG_SUCCEEDED(downstreamCompiler->getVersionString(versionString.writeRef())))
            {
                builder.append(versionString);
            }
        }
    };

    // Add the target specified by targetIndex
    if (targetIndex == -1)
    {
        // -1 means all targets.
        for (auto targetReq : targets)
        {
            addTargetDigest(targetReq);
        }
    }
    else
    {
        auto targetReq = targets[targetIndex];
        addTargetDigest(targetReq);
    }
}

SlangResult Linkage::addSearchPath(char const* path)
{
    m_optionSet.add(CompilerOptionName::Include, String(path));
    return SLANG_OK;
}

SlangResult Linkage::addPreprocessorDefine(char const* name, char const* value)
{
    CompilerOptionValue val;
    val.kind = CompilerOptionValueKind::String;
    val.stringValue = name;
    val.stringValue2 = value;
    m_optionSet.add(CompilerOptionName::MacroDefine, val);
    return SLANG_OK;
}

SlangResult Linkage::setMatrixLayoutMode(SlangMatrixLayoutMode mode)
{
    m_optionSet.setMatrixLayoutMode((MatrixLayoutMode)mode);
    return SLANG_OK;
}

SlangResult Linkage::loadFile(String const& path, PathInfo& outPathInfo, ISlangBlob** outBlob)
{
    outPathInfo.type = PathInfo::Type::Unknown;

    SLANG_RETURN_ON_FAIL(m_fileSystemExt->loadFile(path.getBuffer(), outBlob));

    ComPtr<ISlangBlob> uniqueIdentity;
    // Get the unique identity
    if (SLANG_FAILED(
            m_fileSystemExt->getFileUniqueIdentity(path.getBuffer(), uniqueIdentity.writeRef())))
    {
        // We didn't get a unique identity, so go with just a found path
        outPathInfo.type = PathInfo::Type::FoundPath;
        outPathInfo.foundPath = path;
    }
    else
    {
        outPathInfo = PathInfo::makeNormal(path, StringUtil::getString(uniqueIdentity));
    }
    return SLANG_OK;
}

Expr* Linkage::parseTermString(String typeStr, Scope* scope)
{
    // Create a SourceManager on the stack, so any allocations for 'SourceFile'/'SourceView' etc
    // will be cleaned up
    SourceManager localSourceManager;
    localSourceManager.initialize(getSourceManager(), nullptr);

    Slang::SourceFile* srcFile =
        localSourceManager.createSourceFileWithString(PathInfo::makeTypeParse(), typeStr);

    // We'll use a temporary diagnostic sink
    DiagnosticSink sink(&localSourceManager, nullptr);

    // RAII type to make make sure current SourceManager is restored after parse.
    // Use RAII - to make sure everything is reset even if an exception is thrown.
    struct ScopeReplaceSourceManager
    {
        ScopeReplaceSourceManager(Linkage* linkage, SourceManager* replaceManager)
            : m_linkage(linkage), m_originalSourceManager(linkage->getSourceManager())
        {
            linkage->setSourceManager(replaceManager);
        }

        ~ScopeReplaceSourceManager() { m_linkage->setSourceManager(m_originalSourceManager); }

    private:
        Linkage* m_linkage;
        SourceManager* m_originalSourceManager;
    };

    // We need to temporarily replace the SourceManager for this CompileRequest
    ScopeReplaceSourceManager scopeReplaceSourceManager(this, &localSourceManager);

    SourceLanguage sourceLanguage = SourceLanguage::Slang;
    SlangLanguageVersion languageVersion = m_optionSet.getLanguageVersion();

    auto tokens = preprocessSource(
        srcFile,
        &sink,
        nullptr,
        Dictionary<String, String>(),
        this,
        sourceLanguage,
        languageVersion);

    if (sourceLanguage == SourceLanguage::Unknown)
        sourceLanguage = SourceLanguage::Slang;

    return parseTermFromSourceFile(
        getASTBuilder(),
        tokens,
        &sink,
        scope,
        getNamePool(),
        sourceLanguage);
}

UInt Linkage::addTarget(CodeGenTarget target)
{
    RefPtr<TargetRequest> targetReq = new TargetRequest(this, target);

    Index result = targets.getCount();
    targets.add(targetReq);
    return UInt(result);
}

void Linkage::loadParsedModule(
    RefPtr<FrontEndCompileRequest> compileRequest,
    RefPtr<TranslationUnitRequest> translationUnit,
    Name* name,
    const PathInfo& pathInfo)
{
    // Note: we add the loaded module to our name->module listing
    // before doing semantic checking, so that if it tries to
    // recursively `import` itself, we can detect it.
    //
    RefPtr<Module> loadedModule = translationUnit->getModule();

    // Get a path
    String mostUniqueIdentity = pathInfo.getMostUniqueIdentity();
    SLANG_ASSERT(mostUniqueIdentity.getLength() > 0);

    mapPathToLoadedModule.add(mostUniqueIdentity, loadedModule);
    mapNameToLoadedModules.add(name, loadedModule);

    auto sink = translationUnit->compileRequest->getSink();

    int errorCountBefore = sink->getErrorCount();
    int errorCountAfter;
    try
    {
        compileRequest->checkAllTranslationUnits();
    }
    catch (...)
    {
        mapPathToLoadedModule.remove(mostUniqueIdentity);
        mapNameToLoadedModules.remove(name);
        throw;
    }
    errorCountAfter = sink->getErrorCount();
    if (isInLanguageServer())
    {
        // Don't generate IR as language server.
        // This means that we currently cannot report errors that are detected during IR passes.
        // Ideally we want to run those passes, but that is too risky for what it is worth right
        // now.
    }
    else
    {
        if (errorCountAfter != errorCountBefore)
        {
            // There must have been an error in the loaded module.
            // Remove from maps if there were errors during semantic checking
            mapPathToLoadedModule.remove(mostUniqueIdentity);
            mapNameToLoadedModules.remove(name);
        }
        else
        {
            // If we didn't run into any errors, then try to generate
            // IR code for the imported module.
            if (errorCountAfter == 0)
            {
                loadedModule->setIRModule(
                    generateIRForTranslationUnit(getASTBuilder(), translationUnit));
            }
        }
    }
    loadedModulesList.add(loadedModule);
}

RefPtr<Module> Linkage::findOrLoadSerializedModuleForModuleLibrary(
    ISlangBlob* blobHoldingSerializedData,
    ModuleChunk const* moduleChunk,
    RIFF::ListChunk const* libraryChunk,
    DiagnosticSink* sink)
{
    RefPtr<Module> resultModule;

    // We will attempt things in a few different steps, trying to
    // decode as little of the serialized module as necessary at
    // each step, so that we don't waste time on the heavyweight
    // stuff when we didn't need to.
    //
    // The first step is to simply decode the module name, and
    // see if we have a already loaded a matching module.

    auto moduleName = getNamePool()->getName(moduleChunk->getName());
    if (mapNameToLoadedModules.tryGetValue(moduleName, resultModule))
        return resultModule;

    // It is possible that the module has been loaded, but somehow
    // under a different name, so next we decode the list of file
    // paths that the module depends on, and then rely on the assumption
    // that the first of those paths represents the file for the module
    // itself to detect if we've already loaded a module from that
    // path.
    //
    // Note: While this is a distasteful assumption to make, it is
    // one that gets made in several parts of the compiler codebase
    // already. It isn't something that can be fixed in just one
    // place at this point.

    auto fileDependenciesList = moduleChunk->getFileDependencies();
    auto firstFileDependencyChunk = fileDependenciesList.getFirst();
    if (!firstFileDependencyChunk)
        return nullptr;

    auto modulePathInfo = PathInfo::makePath(firstFileDependencyChunk->getValue());
    if (mapPathToLoadedModule.tryGetValue(modulePathInfo.getMostUniqueIdentity(), resultModule))
        return resultModule;

    // If we failed to find a previously-loaded module, then we
    // will go ahead and load the module from the serialized form.
    //
    PathInfo filePathInfo;
    return loadSerializedModule(
        moduleName,
        modulePathInfo,
        blobHoldingSerializedData,
        moduleChunk,
        libraryChunk,
        SourceLoc(),
        sink);
}

RefPtr<Module> Linkage::loadSerializedModule(
    Name* moduleName,
    const PathInfo& moduleFilePathInfo,
    ISlangBlob* blobHoldingSerializedData,
    ModuleChunk const* moduleChunk,
    RIFF::ListChunk const* containerChunk,
    SourceLoc const& requestingLoc,
    DiagnosticSink* sink)
{
    auto astBuilder = getASTBuilder();
    SLANG_AST_BUILDER_RAII(astBuilder);

    auto module = RefPtr(new Module(this, astBuilder));
    module->setName(moduleName);

    // Just as if we were processing an `import` declaration in
    // source code, we will track the fact that this serialized
    // modlue is (effectively) being imported, so that we can
    // diagnose anything troublesome, like an attempt at a
    // recursive import.
    //
    ModuleBeingImportedRAII moduleBeingImported(this, module, moduleName, requestingLoc);

    // We will register the module in our data structures to
    // track loaded modules, and then remove it in the case
    // where there is some kind of failure.
    //
    String mostUniqueIdentity = moduleFilePathInfo.getMostUniqueIdentity();
    SLANG_ASSERT(mostUniqueIdentity.getLength() > 0);

    mapPathToLoadedModule.add(mostUniqueIdentity, module);
    mapNameToLoadedModules.add(moduleName, module);
    try
    {
        if (SLANG_FAILED(loadSerializedModuleContents(
                module,
                moduleFilePathInfo,
                blobHoldingSerializedData,
                moduleChunk,
                containerChunk,
                sink)))
        {
            mapPathToLoadedModule.remove(mostUniqueIdentity);
            mapNameToLoadedModules.remove(moduleName);
            return nullptr;
        }

        loadedModulesList.add(module);
        return module;
    }
    catch (...)
    {
        mapPathToLoadedModule.remove(mostUniqueIdentity);
        mapNameToLoadedModules.remove(moduleName);
        throw;
    }
}

RefPtr<Module> Linkage::loadBinaryModuleImpl(
    Name* moduleName,
    const PathInfo& moduleFilePathInfo,
    ISlangBlob* moduleFileContents,
    SourceLoc const& requestingLoc,
    DiagnosticSink* sink)
{
    auto astBuilder = getASTBuilder();
    SLANG_AST_BUILDER_RAII(astBuilder);

    // We start by reading the content of the file as
    // an in-memory RIFF container.
    //
    auto rootChunk = RIFF::RootChunk::getFromBlob(moduleFileContents);
    if (!rootChunk)
    {
        return nullptr;
    }

    auto moduleChunk = ModuleChunk::find(rootChunk);
    if (!moduleChunk)
    {
        return nullptr;
    }

    // Next, we attempt to check if the binary module is up to
    // date with the compilation options in use as well as
    // the contents of all the files its compilation depended
    // on (as determined by its hash).
    //
    String mostUniqueIdentity = moduleFilePathInfo.getMostUniqueIdentity();
    SLANG_ASSERT(mostUniqueIdentity.getLength() > 0);
    if (m_optionSet.getBoolOption(CompilerOptionName::UseUpToDateBinaryModule))
    {
        if (!isBinaryModuleUpToDate(moduleFilePathInfo.foundPath, moduleChunk))
        {
            return nullptr;
        }
    }

    // If everything seems reasonable, then we will go ahead and load
    // the module more completely from that serialized representation.
    //
    RefPtr<Module> module = loadSerializedModule(
        moduleName,
        moduleFilePathInfo,
        moduleFileContents,
        moduleChunk,
        rootChunk,
        requestingLoc,
        sink);

    return module;
}

void Linkage::_diagnoseErrorInImportedModule(DiagnosticSink* sink)
{
    for (auto info = m_modulesBeingImported; info; info = info->next)
    {
        sink->diagnose(info->importLoc, Diagnostics::errorInImportedModule, info->name);
    }
    if (!isInLanguageServer())
    {
        sink->diagnose(SourceLoc(), Diagnostics::compilationCeased);
    }
}

RefPtr<Module> Linkage::loadModuleImpl(
    Name* moduleName,
    const PathInfo& modulePathInfo,
    ISlangBlob* moduleBlob,
    SourceLoc const& requestingLoc,
    DiagnosticSink* sink,
    const LoadedModuleDictionary* additionalLoadedModules,
    ModuleBlobType blobType)
{
    switch (blobType)
    {
    case ModuleBlobType::IR:
        return loadBinaryModuleImpl(moduleName, modulePathInfo, moduleBlob, requestingLoc, sink);

    case ModuleBlobType::Source:
        return loadSourceModuleImpl(
            moduleName,
            modulePathInfo,
            moduleBlob,
            requestingLoc,
            sink,
            additionalLoadedModules);

    default:
        SLANG_UNEXPECTED("unknown module blob type");
        UNREACHABLE_RETURN(nullptr);
    }
}

RefPtr<Module> Linkage::loadSourceModuleImpl(
    Name* name,
    const PathInfo& filePathInfo,
    ISlangBlob* sourceBlob,
    SourceLoc const& srcLoc,
    DiagnosticSink* sink,
    const LoadedModuleDictionary* additionalLoadedModules)
{
    RefPtr<FrontEndCompileRequest> frontEndReq = new FrontEndCompileRequest(this, nullptr, sink);

    frontEndReq->additionalLoadedModules = additionalLoadedModules;

    RefPtr<TranslationUnitRequest> translationUnit = new TranslationUnitRequest(frontEndReq);
    translationUnit->compileRequest = frontEndReq;
    translationUnit->setModuleName(name);
    Stage impliedStage;
    translationUnit->sourceLanguage = SourceLanguage::Slang;

    // If we are loading from a file with apparaent glsl extension,
    // set the source language to GLSL to enable GLSL compatibility mode.
    if ((SourceLanguage)findSourceLanguageFromPath(filePathInfo.getName(), impliedStage) ==
        SourceLanguage::GLSL)
    {
        translationUnit->sourceLanguage = SourceLanguage::GLSL;
    }

    frontEndReq->addTranslationUnit(translationUnit);

    auto module = translationUnit->getModule();

    ModuleBeingImportedRAII moduleBeingImported(this, module, name, srcLoc);

    // Create an artifact for the source
    auto sourceArtifact = ArtifactUtil::createArtifact(
        ArtifactDesc::make(ArtifactKind::Source, ArtifactPayload::Slang, ArtifactStyle::Unknown));

    if (sourceBlob)
    {
        // If the user has already provided a source blob, use that.
        sourceArtifact->addRepresentation(
            new SourceBlobWithPathInfoArtifactRepresentation(filePathInfo, sourceBlob));
    }
    else if (
        filePathInfo.type == PathInfo::Type::Normal ||
        filePathInfo.type == PathInfo::Type::FoundPath)
    {
        // Create with the 'friendly' name
        // We create that it was loaded from the file system
        sourceArtifact->addRepresentation(new ExtFileArtifactRepresentation(
            filePathInfo.foundPath.getUnownedSlice(),
            getFileSystemExt()));
    }
    else
    {
        return nullptr;
    }

    translationUnit->addSourceArtifact(sourceArtifact);

    if (SLANG_FAILED(translationUnit->requireSourceFiles()))
    {
        // Some problem accessing source files
        return nullptr;
    }
    int errorCountBefore = sink->getErrorCount();
    frontEndReq->parseTranslationUnit(translationUnit);
    int errorCountAfter = sink->getErrorCount();

    if (errorCountAfter != errorCountBefore && !isInLanguageServer())
    {
        _diagnoseErrorInImportedModule(sink);
        // Something went wrong during the parsing, so we should bail out.
        return nullptr;
    }

    try
    {
        loadParsedModule(frontEndReq, translationUnit, name, filePathInfo);
    }
    catch (const Slang::AbortCompilationException&)
    {
        // Something is fatally wrong, we should return nullptr.
        module = nullptr;
    }
    errorCountAfter = sink->getErrorCount();

    if (errorCountAfter != errorCountBefore && !isInLanguageServer())
    {
        // If something is fatally wrong, we want to report
        // the diagnostic even if we are in language server
        // and processing a different module.
        _diagnoseErrorInImportedModule(sink);
        // Something went wrong during the parsing, so we should bail out.
        return nullptr;
    }

    if (!module)
        return nullptr;

    module->setPathInfo(filePathInfo);
    return module;
}

bool Linkage::isBeingImported(Module* module)
{
    for (auto ii = m_modulesBeingImported; ii; ii = ii->next)
    {
        if (module == ii->module)
            return true;
    }
    return false;
}

// Derive a file name for the module, by taking the given
// identifier, replacing all occurrences of `_` with `-`,
// and then appending `.slang`.
//
// For example, `foo_bar` becomes `foo-bar.slang`.
String getFileNameFromModuleName(Name* name, bool translateUnderScore)
{
    String fileName;
    if (!getText(name).getUnownedSlice().endsWithCaseInsensitive(".slang"))
    {
        StringBuilder sb;
        for (auto c : getText(name))
        {
            if (translateUnderScore && c == '_')
                c = '-';

            sb.append(c);
        }
        sb.append(".slang");
        fileName = sb.produceString();
    }
    else
    {
        fileName = getText(name);
    }
    return fileName;
}

RefPtr<Module> Linkage::findOrImportModule(
    Name* moduleName,
    SourceLoc const& requestingLoc,
    DiagnosticSink* sink,
    const LoadedModuleDictionary* loadedModules)
{
    // Have we already loaded a module matching this name?
    //
    RefPtr<LoadedModule> previouslyLoadedModule;
    if (mapNameToLoadedModules.tryGetValue(moduleName, previouslyLoadedModule))
    {
        // If the map shows a null module having been loaded,
        // then that means there was a prior load attempt,
        // but it failed, so we won't bother trying again.
        //
        if (!previouslyLoadedModule)
            return nullptr;

        // If state shows us that the module is already being
        // imported deeper on the call stack, then we've
        // hit a recursive case, and that is an error.
        //
        if (isBeingImported(previouslyLoadedModule))
        {
            // We seem to be in the middle of loading this module
            sink->diagnose(requestingLoc, Diagnostics::recursiveModuleImport, moduleName);
            return nullptr;
        }

        return previouslyLoadedModule;
    }

    // If the user is providing an additional list of loaded modules, we find
    // if the module being imported is in that list. This allows a translation
    // unit to use previously checked translation units in the same
    // FrontEndCompileRequest.
    {
        Module* previouslyLoadedLocalModule = nullptr;
        if (loadedModules && loadedModules->tryGetValue(moduleName, previouslyLoadedLocalModule))
        {
            return previouslyLoadedLocalModule;
        }
    }

    // If the name being requested matches the name of a built-in module,
    // then we will special-case the process by loading that builtin
    // module directly.
    //
    // TODO: right now this logic is only considering the built-in `glsl`
    // module, but it should probably be generalized so that we can more
    // easily support having multiple built-in modules rather than just
    // putting everything into `core`.
    //
    if (moduleName == getSessionImpl()->glslModuleName)
    {
        // This is a builtin glsl module, just load it from embedded definition.
        auto glslModule = getSessionImpl()->getBuiltinModule(slang::BuiltinModuleName::GLSL);
        if (!glslModule)
        {
            // Note: the way this logic is currently written, if the built-in
            // `glsl` module fails to load, then we will *not* fall back to
            // searching for a user-defined module in a file like `glsl.slang`.
            //
            // It is unclear if this should be the default behavior or not.
            // Should built-in modules be prioritized over user modules?
            // Should built-in modules shadow user modules, even when the
            // built-in module fails to load, for some reason?
            //
            sink->diagnose(requestingLoc, Diagnostics::glslModuleNotAvailable, moduleName);
        }
        return glslModule;
    }

    // We are going to use a loop to search for a suitable file to
    // load the module from, to account for a few key choices:
    //
    // * We can both load modules from a source `.slang` file,
    //   or from a binary `.slang-module` file.
    //
    // * For a variety of reasons, the `import` logic has historically
    //   translated underscores in a module name into dashes (so that
    //   `import my_module` will look for `my-module.slang`), and we
    //   try to support both that convention as well as a convention
    //   that preserves underscores.
    //
    // To try to keep this logic as orthogonal as possible, we first
    // construct lists of the options we want to iterate over, and
    // then do the actual loop later.

    ShortList<ModuleBlobType, 2> typesToTry;
    if (isInLanguageServer())
    {
        // When in language server, we always prefer to use source module if it is available.
        typesToTry.add(ModuleBlobType::Source);
        typesToTry.add(ModuleBlobType::IR);
    }
    else
    {
        // Look for a precompiled module first, if not exist, load from source.
        typesToTry.add(ModuleBlobType::IR);
        typesToTry.add(ModuleBlobType::Source);
    }

    // We will always search for a file name that directly matches the
    // module name as written first, and then search for one with
    // underscores replaced by dashes. The latter is the original
    // behavior that `import` provided, but it seems safest to prefer
    // the exact name spelled in the user's code when there might
    // actually be ambiguity.
    //
    auto defaultSourceFileName = getFileNameFromModuleName(moduleName, false);
    auto alternativeSourceFileName = getFileNameFromModuleName(moduleName, true);
    String sourceFileNamesToTry[] = {defaultSourceFileName, alternativeSourceFileName};

    // We are going to look for the candidate file using the same
    // logic that would be used for a preprocessor `#include`,
    // so we set up the necessary state.
    //
    IncludeSystem includeSystem(&getSearchDirectories(), getFileSystemExt(), getSourceManager());

    // Just like with a `#include`, the search will take into
    // account the path to the file where the request to import
    // this module came from (e.g. the source file with the
    // `import` declaration), if such a path is available.
    //
    PathInfo requestingPathInfo =
        getSourceManager()->getPathInfo(requestingLoc, SourceLocType::Actual);

    for (auto type : typesToTry)
    {
        for (auto sourceFileName : sourceFileNamesToTry)
        {
            // The `sourceFileName` will have the `.slang` extension,
            // so if we are looking for a binary module, we need
            // to change the extension we will look for.
            //
            String fileName;
            switch (type)
            {
            case ModuleBlobType::Source:
                fileName = sourceFileName;
                break;

            case ModuleBlobType::IR:
                fileName = Path::replaceExt(sourceFileName, "slang-module");
                break;
            }

            // We now search for a file matching the desired name,
            // using the same logic as for a `#include`.
            //
            // TODO: We might want to consider how to handle the case
            // of an `import` with a relative path a little specially,
            // since it could in theory be possible for two `.slang`
            // files with the same base name to exist in different
            // directories in a project, and we'd want file-relative
            // `import`s to work for each, without having either one
            // be able to "claim" the bare identifier of the base
            // name for itself.
            //
            PathInfo filePathInfo;
            if (SLANG_FAILED(
                    includeSystem.findFile(fileName, requestingPathInfo.foundPath, filePathInfo)))
            {
                // If we failed to find the file at this step, we
                // will continue the search for our other options.
                //
                continue;
            }

            // We will *again* search for a previously loaded module.
            //
            // It is possible that the same file will have been loaded
            // as a module under two different module names. The easiest
            // way for this to happen is if there are `import` declarations
            // using both the underscore and dash conventions (e.g., both
            // `import "my-module.slang"` and `import my_module`).
            //
            // This case may also arise if one file `import`s a module using
            // just an identifier for its name, but another `import`s it
            // using a path (e.g., `import "subdir/file.slang"`).
            //
            // No matter how the situation arises, we only want to have one
            // copy of the "same" module loaded at a given time, so we
            // will re-use the existing module if we find one here.
            //
            if (mapPathToLoadedModule.tryGetValue(
                    filePathInfo.getMostUniqueIdentity(),
                    previouslyLoadedModule))
            {
                // TODO: If we find a previously-loaded module at this step,
                // then we should probably register that module under the
                // given `moduleName` in the map of loaded modules, so
                // that subsequent `import`s using the same form will find it.
                //
                return previouslyLoadedModule;
            }

            // Now we try to load the content of the file.
            //
            // If for some reason we could find a file at the
            // given path, but for some reason couldn't *open*
            // and *read* it, then we continue the search
            // using whatever other candidate file names are left.
            //
            ComPtr<ISlangBlob> fileContents;
            if (SLANG_FAILED(includeSystem.loadFile(filePathInfo, fileContents)))
            {
                continue;
            }

            // If we found a real file and were able to load its contents,
            // then we'll go ahead and try to load a module from it,
            // whether by compiling it or decoding the binary.
            //
            auto module = loadModuleImpl(
                moduleName,
                filePathInfo,
                fileContents,
                requestingLoc,
                sink,
                loadedModules,
                type);

            // If the attempt to load the module from the given path
            // was successful, we go ahead and use it, without trying
            // out any other options.
            //
            if (module)
                return module;
        }
    }

    // Fallback: If the normal search failed, we will just search the whatever modules
    // from our standard module search path
    auto standardModuleDirPath = getStandardModuleDirPath();
    if (standardModuleDirPath.getLength() > 0)
    {
        String standardModulePath = findStandardModulePath(standardModuleDirPath, moduleName->text);
        if (standardModulePath.getLength() > 0)
        {
            // Found standard module, load it directly
            ComPtr<ISlangBlob> fileContents;
            SlangResult result = getFileSystemExt()->loadFile(
                standardModulePath.getBuffer(),
                fileContents.writeRef());
            if (SLANG_SUCCEEDED(result))
            {
                auto pathInfo = PathInfo::makeFromString(standardModulePath);
                RefPtr<Module> module = loadModuleImpl(
                    moduleName,
                    pathInfo,
                    fileContents,
                    requestingLoc,
                    sink,
                    nullptr,
                    ModuleBlobType::IR);
                if (module)
                {
                    if (auto irModule = module->getIRModule())
                    {
                        if (irModule->getModuleInst()
                                ->findDecoration<IRExperimentalModuleDecoration>() &&
                            !m_optionSet.getBoolOption(CompilerOptionName::ExperimentalFeature))
                        {
                            sink->diagnose(
                                requestingLoc,
                                Diagnostics::needToEnableExperimentFeature,
                                moduleName);
                        }
                    }
                    return module;
                }
            }
        }
    }

    // If we tried out all of our candidate file names
    // and failed with each of them, then we diagnose
    // an error based on the original *source* file
    // name.
    //
    // TODO: this should really be an error message
    // that clearly states something like "no file
    // suitable for module `whatever` was found
    // and loaded.
    //
    // Ideally that error message would include whatever
    // of the candidate file names from the loop above
    // got furthest along in the process (or just a
    // list of the file names that were tried, if
    // nothing was even found via the include system).
    //
    sink->diagnose(requestingLoc, Diagnostics::cannotOpenFile, defaultSourceFileName);

    // If the attempt to import the module failed, then
    // we will stick a null pointer into the map of loaded
    // modules, so that subsequent attempts to load a module
    // with this name will return null without having to
    // go through all the above steps yet again.
    //
    mapNameToLoadedModules[moduleName] = nullptr;
    return nullptr;
}

SourceFile* Linkage::loadSourceFile(String pathFrom, String path)
{
    IncludeSystem includeSystem(&getSearchDirectories(), getFileSystemExt(), getSourceManager());
    ComPtr<slang::IBlob> blob;
    PathInfo pathInfo;
    SLANG_RETURN_NULL_ON_FAIL(includeSystem.findFile(path, pathFrom, pathInfo));
    SourceFile* sourceFile = nullptr;
    SLANG_RETURN_NULL_ON_FAIL(includeSystem.loadFile(pathInfo, blob, sourceFile));
    return sourceFile;
}

// Check if a serialized module is up-to-date with current compiler options and source files.
bool Linkage::isBinaryModuleUpToDate(String fromPath, RIFF::ListChunk const* baseChunk)
{
    auto moduleChunk = ModuleChunk::find(baseChunk);
    if (!moduleChunk)
        return false;

    SHA1::Digest existingDigest = moduleChunk->getDigest();

    DigestBuilder<SHA1> digestBuilder;
    auto version = String(getBuildTagString());
    digestBuilder.append(version);
    m_optionSet.buildHash(digestBuilder);

    // Find the canonical path of the directory containing the module source file.
    String moduleSrcPath = "";

    auto dependencyChunks = moduleChunk->getFileDependencies();
    if (auto firstDependencyChunk = dependencyChunks.getFirst())
    {
        moduleSrcPath = firstDependencyChunk->getValue();

        IncludeSystem includeSystem(
            &getSearchDirectories(),
            getFileSystemExt(),
            getSourceManager());
        PathInfo modulePathInfo;
        if (SLANG_SUCCEEDED(includeSystem.findFile(moduleSrcPath, fromPath, modulePathInfo)))
        {
            moduleSrcPath = modulePathInfo.foundPath;
            Path::getCanonical(moduleSrcPath, moduleSrcPath);
        }
    }

    for (auto dependencyChunk : dependencyChunks)
    {
        auto file = dependencyChunk->getValue();
        auto sourceFile = loadSourceFile(fromPath, file);
        if (!sourceFile)
        {
            // If we cannot find the source file from `fromPath`,
            // try again from the module's source file path.
            if (dependencyChunks.getFirst())
                sourceFile = loadSourceFile(moduleSrcPath, file);
        }
        if (!sourceFile)
            return false;
        digestBuilder.append(sourceFile->getDigest());
    }
    return digestBuilder.finalize() == existingDigest;
}

SLANG_NO_THROW bool SLANG_MCALL
Linkage::isBinaryModuleUpToDate(const char* modulePath, slang::IBlob* binaryModuleBlob)
{
    auto rootChunk = RIFF::RootChunk::getFromBlob(binaryModuleBlob);
    if (!rootChunk)
        return false;
    return isBinaryModuleUpToDate(modulePath, rootChunk);
}

SourceFile* Linkage::findFile(Name* name, SourceLoc loc, IncludeSystem& outIncludeSystem)
{
    auto impl = [&](bool translateUnderScore) -> SourceFile*
    {
        auto fileName = getFileNameFromModuleName(name, translateUnderScore);

        // Next, try to find the file of the given name,
        // using our ordinary include-handling logic.

        auto& searchDirs = getSearchDirectories();
        outIncludeSystem = IncludeSystem(&searchDirs, getFileSystemExt(), getSourceManager());

        // Get the original path info
        PathInfo pathIncludedFromInfo = getSourceManager()->getPathInfo(loc, SourceLocType::Actual);
        PathInfo filePathInfo;

        ComPtr<ISlangBlob> fileContents;

        // We have to load via the found path - as that is how file was originally loaded
        if (SLANG_FAILED(
                outIncludeSystem.findFile(fileName, pathIncludedFromInfo.foundPath, filePathInfo)))
        {
            return nullptr;
        }
        // Otherwise, try to load it.
        SourceFile* sourceFile;
        if (SLANG_FAILED(outIncludeSystem.loadFile(filePathInfo, fileContents, sourceFile)))
        {
            return nullptr;
        }
        return sourceFile;
    };
    if (auto rs = impl(false))
        return rs;
    return impl(true);
}

Linkage::IncludeResult Linkage::findAndIncludeFile(
    Module* module,
    TranslationUnitRequest* translationUnit,
    Name* name,
    SourceLoc const& loc,
    DiagnosticSink* sink)
{
    IncludeResult result;
    result.fileDecl = nullptr;
    result.isNew = false;

    IncludeSystem includeSystem;
    auto sourceFile = findFile(name, loc, includeSystem);
    if (!sourceFile)
    {
        sink->diagnose(loc, Diagnostics::cannotOpenFile, getText(name));
        return result;
    }

    // If the file has already been included, don't need to do anything further.
    if (auto existingFileDecl = module->getIncludedSourceFileMap().tryGetValue(sourceFile))
    {
        result.fileDecl = *existingFileDecl;
        result.isNew = false;
        return result;
    }

    if (isInLanguageServer())
    {
        // HACK: When in language server mode, we will always load the currently opend file as a
        // fresh module even if some previously opened file already references the current file via
        // `import` or `include`. see comments in `WorkspaceVersion::getOrLoadModule()` for the
        // reason behind this. An undesired outcome of this decision is that we could endup
        // including the currently opened file itself via chain of `__include`s because the
        // currently opened file will not have a true unique file system identity that allows it to
        // be deduplicated correct. Therefore we insert a hack logic here to detect re-inclusion by
        // just the file path. We can clean up this hack by making the language server truly support
        // incremental checking so we can reuse the previously loaded module instead of needing to
        // always start with a fresh copy.
        //
        for (auto file : translationUnit->getSourceFiles())
        {
            if (file->getPathInfo().hasFoundPath() &&
                Path::equals(file->getPathInfo().foundPath, sourceFile->getPathInfo().foundPath))
                return result;
        }
    }

    module->addFileDependency(sourceFile);

    // Create a transparent FileDecl to hold all children from the included file.
    auto fileDecl = module->getASTBuilder()->create<FileDecl>();
    fileDecl->nameAndLoc.name = name;
    fileDecl->parentDecl = module->getModuleDecl();
    module->getIncludedSourceFileMap().add(sourceFile, fileDecl);

    FrontEndPreprocessorHandler preprocessorHandler(
        module,
        module->getASTBuilder(),
        sink,
        translationUnit);
    auto combinedPreprocessorDefinitions = translationUnit->getCombinedPreprocessorDefinitions();
    SourceLanguage sourceLanguage = translationUnit->sourceLanguage;
    SlangLanguageVersion slangLanguageVersion = module->getModuleDecl()->languageVersion;
    auto tokens = preprocessSource(
        sourceFile,
        sink,
        &includeSystem,
        combinedPreprocessorDefinitions,
        this,
        sourceLanguage,
        slangLanguageVersion,
        &preprocessorHandler);

    if (sourceLanguage == SourceLanguage::Unknown)
        sourceLanguage = translationUnit->sourceLanguage;

    if (slangLanguageVersion != module->getModuleDecl()->languageVersion)
    {
        sink->diagnose(
            tokens.begin()->getLoc(),
            Diagnostics::languageVersionDiffersFromIncludingModule);
    }

    auto outerScope = module->getModuleDecl()->ownedScope;
    parseSourceFile(
        module->getASTBuilder(),
        translationUnit,
        sourceLanguage,
        tokens,
        sink,
        outerScope,
        fileDecl);

    module->getModuleDecl()->addMember(fileDecl);

    result.fileDecl = fileDecl;
    result.isNew = true;
    return result;
}

void Linkage::setFileSystem(ISlangFileSystem* inFileSystem)
{
    // Set the fileSystem
    m_fileSystem = inFileSystem;

    // Release what's there
    m_fileSystemExt.setNull();

    // If nullptr passed in set up default
    if (inFileSystem == nullptr)
    {
        m_fileSystemExt = new Slang::CacheFileSystem(Slang::OSFileSystem::getExtSingleton());
    }
    else
    {
        if (auto cacheFileSystem = as<CacheFileSystem>(inFileSystem))
        {
            m_fileSystemExt = cacheFileSystem;
        }
        else
        {
            if (m_requireCacheFileSystem)
            {
                m_fileSystemExt = new Slang::CacheFileSystem(inFileSystem);
            }
            else
            {
                // See if we have the full ISlangFileSystemExt interface, if we do just use it
                inFileSystem->queryInterface(SLANG_IID_PPV_ARGS(m_fileSystemExt.writeRef()));

                // If not wrap with CacheFileSystem that emulates ISlangFileSystemExt from the
                // ISlangFileSystem interface
                if (!m_fileSystemExt)
                {
                    // Construct a wrapper to emulate the extended interface behavior
                    m_fileSystemExt = new Slang::CacheFileSystem(m_fileSystem);
                }
            }
        }
    }

    // If requires a cache file system, check that it does have one
    SLANG_ASSERT(m_requireCacheFileSystem == false || as<CacheFileSystem>(m_fileSystemExt));

    // Set the file system used on the source manager
    getSourceManager()->setFileSystemExt(m_fileSystemExt);
}

SlangResult Linkage::loadSerializedModuleContents(
    Module* module,
    const PathInfo& moduleFilePathInfo,
    ISlangBlob* blobHoldingSerializedData,
    ModuleChunk const* moduleChunk,
    RIFF::ListChunk const* containerChunk,
    DiagnosticSink* sink)
{
    // At this point we've dealt with basically all of
    // the formalities, and we just need to get down
    // to the real work of decoding the information
    // in the `moduleChunk`.

    //
    // TODO(tfoley): The fact that a separate `containerChunk` is getting
    // passed in here is entirely byproduct of the support for "module libraries"
    // that can (in principle) contain multiple serialized modules. When
    // things are serialized in the "container" representation used for
    // a module library, there is a single `DebugChunk` as a child of
    // the container, with all of the `ModuleChunk`s sharing that debug info.
    //
    // In contrast, the more typical kind of serialized module that the compiler
    // produces serializes a single `ModuleChunk`, and the `DebugChunk` is
    // one of its direct children. Thus there are currently two different
    // locations where debug information might be found.
    //
    // Prior to the change where we navigate the serialized RIFF hierarchy
    // in memory without copying it, this issue was addressed by having
    // the subroutine that looked for a `DebugChunk` start at the `ModuleChunk`
    // and work its way up through the hierarchy using parent pointers that
    // were created as part of RIFF loading. When navigating the RIFF in-place
    // we don't have such parent pointers.
    //
    // As a short-term solution, we should deprecate and remove the support
    // for "module libraries" so that the code doesn't have to handle two
    // different layouts.
    //
    // In the longer term, we should be making some conscious design decisions
    // around how we want to organize the top-level structure of our serialized
    // intermediate/output formats, since there's quite a mix of different
    // approaches currently in use.
    //

    auto sourceManager = getSourceManager();
    RefPtr<SerialSourceLocReader> sourceLocReader;
    if (auto debugChunk = DebugChunk::find(moduleChunk, containerChunk))
    {
        SLANG_RETURN_ON_FAIL(
            readSourceLocationsFromDebugChunk(debugChunk, sourceManager, sourceLocReader));
    }

    auto astChunk = moduleChunk->findAST();
    if (!astChunk)
        return SLANG_FAIL;

    auto irChunk = moduleChunk->findIR();
    if (!irChunk)
        return SLANG_FAIL;

    auto astBuilder = getASTBuilder();
    auto session = getSessionImpl();

    // For the purposes of any modules referenced
    // by the module we're about to decode, we will
    // construct a source location that represents
    // the module itself (if possible).
    //
    // TODO(tfoley): This logic seems like overkill, given
    // that many (most? all?) control-flow paths that can
    // reach this routine will have already found a `SourceFile`
    // to represent the module, as part of even getting the
    // `moduleFilePathInfo` to pass in
    //
    // The approach here is more or less exactly copied
    // from what the old `SerialContainerUtil::read` function
    // used to do, with the hopes that it will as many tests
    // passing as possible.
    //
    // Down the line somebody should scrutinize all of this
    // kind of logic in the compiler codebase, because there
    // is something that feels unclean about how paths are being handled.
    //
    SourceLoc serializedModuleLoc;
    {
        auto sourceFile =
            sourceManager->findSourceFileByPathRecursively(moduleFilePathInfo.foundPath);
        if (!sourceFile)
        {
            sourceFile = sourceManager->createSourceFileWithString(moduleFilePathInfo, String());
            sourceManager->addSourceFile(moduleFilePathInfo.getMostUniqueIdentity(), sourceFile);
        }
        auto sourceView =
            sourceManager->createSourceView(sourceFile, &moduleFilePathInfo, SourceLoc());
        serializedModuleLoc = sourceView->getRange().begin;
    }

    auto moduleDecl = readSerializedModuleAST(
        this,
        astBuilder,
        sink,
        blobHoldingSerializedData,
        astChunk,
        sourceLocReader,
        serializedModuleLoc);
    if (!moduleDecl)
        return SLANG_FAIL;
    module->setModuleDecl(moduleDecl);

    RefPtr<IRModule> irModule;
    SLANG_RETURN_ON_FAIL(readSerializedModuleIR(irChunk, session, sourceLocReader, irModule));
    module->setIRModule(irModule);

    // The handling of file dependencies is complicated, because of
    // the way that the encoding logic tried to make all of the
    // paths be relative to the primary source file for the module.
    //
    // We end up needing to undo some amount of that work here.
    //

    module->clearFileDependency();
    String moduleSourcePath = moduleFilePathInfo.foundPath;
    bool isFirst = true;
    for (auto depenencyFileChunk : moduleChunk->getFileDependencies())
    {
        auto encodedDependencyFilePath = depenencyFileChunk->getValue();

        auto sourceFile = loadSourceFile(moduleFilePathInfo.foundPath, encodedDependencyFilePath);
        if (isFirst)
        {
            // The first file is the source for the main module file.
            // We store the module path as the basis for finding the remaining
            // dependent files.
            if (sourceFile)
                moduleSourcePath = sourceFile->getPathInfo().foundPath;
            isFirst = false;
        }
        // If we cannot find the dependent file directly, try to find
        // it relative to the module source path.
        if (!sourceFile)
        {
            sourceFile = loadSourceFile(moduleSourcePath, encodedDependencyFilePath);
        }
        if (sourceFile)
        {
            module->addFileDependency(sourceFile);
        }
    }
    module->setPathInfo(moduleFilePathInfo);
    module->setDigest(moduleChunk->getDigest());
    module->_collectShaderParams();
    module->_discoverEntryPoints(sink, targets);

    // Hook up fileDecl's scope to module's scope.
    for (auto fileDecl : moduleDecl->getDirectMemberDeclsOfType<FileDecl>())
    {
        addSiblingScopeForContainerDecl(m_astBuilder, moduleDecl->ownedScope, fileDecl);
    }
    if (sink->getErrorCount() != 0)
        return SLANG_FAIL;
    return SLANG_OK;
}

void Linkage::setRequireCacheFileSystem(bool requireCacheFileSystem)
{
    if (requireCacheFileSystem == m_requireCacheFileSystem)
    {
        return;
    }

    ComPtr<ISlangFileSystem> scopeFileSystem(m_fileSystem);
    m_requireCacheFileSystem = requireCacheFileSystem;

    setFileSystem(scopeFileSystem);
}

RefPtr<Module> findOrImportModule(
    Linkage* linkage,
    Name* name,
    SourceLoc const& loc,
    DiagnosticSink* sink,
    const LoadedModuleDictionary* loadedModules)
{
    return linkage->findOrImportModule(name, loc, sink, loadedModules);
}

Type* checkProperType(Linkage* linkage, TypeExp typeExp, DiagnosticSink* sink);


} // namespace Slang
