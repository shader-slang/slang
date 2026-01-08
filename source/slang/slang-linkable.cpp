// slang-linkable.cpp
#include "slang-linkable.h"

#include "compiler-core/slang-artifact-container-util.h"
#include "compiler-core/slang-artifact-desc-util.h"
#include "compiler-core/slang-artifact-impl.h"
#include "core/slang-char-util.h"
#include "core/slang-memory-file-system.h"
#include "slang-check-impl.h"
#include "slang-compiler.h"
#include "slang-lookup.h"
#include "slang-mangle.h"

namespace Slang
{

//
// ModuleDependencyList
//

void ModuleDependencyList::addDependency(Module* module)
{
    // If we depend on a module, then we depend on everything it depends on.
    //
    // Note: We are processing these sub-depenencies before adding the
    // `module` itself, so that in the common case a module will always
    // appear *after* everything it depends on.
    //
    // However, this rule is being violated in the compiler right now because
    // the modules for hte top-level translation units of a compile request
    // will be added to the list first (using `addLeafDependency`) to
    // maintain compatibility with old behavior. This may be fixed later.
    //
    for (auto subDependency : module->getModuleDependencyList())
    {
        _addDependency(subDependency);
    }
    _addDependency(module);
}

void ModuleDependencyList::addLeafDependency(Module* module)
{
    _addDependency(module);
}

void ModuleDependencyList::_addDependency(Module* module)
{
    if (m_moduleSet.contains(module))
        return;

    m_moduleList.add(module);
    m_moduleSet.add(module);
}

//
// FileDependencyList
//

void FileDependencyList::addDependency(SourceFile* sourceFile)
{
    if (m_fileSet.contains(sourceFile))
        return;

    m_fileList.add(sourceFile);
    m_fileSet.add(sourceFile);
}

void FileDependencyList::addDependency(Module* module)
{
    for (SourceFile* sourceFile : module->getFileDependencyList())
    {
        addDependency(sourceFile);
    }
}

//
// ComponentType
//

ComponentType::ComponentType(Linkage* linkage)
    : m_linkage(linkage)
{
}

ComponentType* asInternal(slang::IComponentType* inComponentType)
{
    // Note: we use a `queryInterface` here instead of just a `static_cast`
    // to ensure that the `IComponentType` we get is the preferred/canonical
    // one, which shares its address with the `ComponentType`.
    //
    // TODO: An alternative choice here would be to have a "magic" IID that
    // we pass into `queryInterface` that returns the `ComponentType` directly
    // (without even `addRef`-ing it).
    //
    ComPtr<slang::IComponentType> componentType;
    inComponentType->queryInterface(SLANG_IID_PPV_ARGS(componentType.writeRef()));
    return static_cast<ComponentType*>(componentType.get());
}

ISlangUnknown* ComponentType::getInterface(Guid const& guid)
{
    if (guid == ISlangUnknown::getTypeGuid() || guid == slang::IComponentType::getTypeGuid())
    {
        return static_cast<slang::IComponentType*>(this);
    }
    if (guid == IModulePrecompileService_Experimental::getTypeGuid())
        return static_cast<slang::IModulePrecompileService_Experimental*>(this);
    if (guid == IComponentType2::getTypeGuid())
        return static_cast<slang::IComponentType2*>(this);
    return nullptr;
}

SLANG_NO_THROW slang::ISession* SLANG_MCALL ComponentType::getSession()
{
    return m_linkage;
}

SLANG_NO_THROW slang::ProgramLayout* SLANG_MCALL
ComponentType::getLayout(Int targetIndex, slang::IBlob** outDiagnostics)
{
    auto linkage = getLinkage();
    if (targetIndex < 0 || targetIndex >= linkage->targets.getCount())
        return nullptr;
    auto target = linkage->targets[targetIndex];

    DiagnosticSink sink(linkage->getSourceManager(), Lexer::sourceLocationLexer);
    auto programLayout = getTargetProgram(target)->getOrCreateLayout(&sink);
    sink.getBlobIfNeeded(outDiagnostics);

    return asExternal(programLayout);
}

static ICastable* _findDiagnosticRepresentation(IArtifact* artifact)
{
    if (auto rep = findAssociatedRepresentation<IArtifactDiagnostics>(artifact))
    {
        return rep;
    }

    for (auto associated : artifact->getAssociated())
    {
        if (isDerivedFrom(associated->getDesc().payload, ArtifactPayload::Diagnostics))
        {
            return associated;
        }
    }
    return nullptr;
}

static IArtifact* _findObfuscatedSourceMap(IArtifact* artifact)
{
    // If we find any obfuscated source maps, we are done
    for (auto associated : artifact->getAssociated())
    {
        const auto desc = associated->getDesc();

        if (isDerivedFrom(desc.payload, ArtifactPayload::SourceMap) &&
            isDerivedFrom(desc.style, ArtifactStyle::Obfuscated))
        {
            return associated;
        }
    }
    return nullptr;
}

SLANG_NO_THROW SlangResult SLANG_MCALL ComponentType::getResultAsFileSystem(
    SlangInt entryPointIndex,
    Int targetIndex,
    ISlangMutableFileSystem** outFileSystem)
{
    ComPtr<ISlangBlob> diagnostics;
    ComPtr<ISlangBlob> code;

    SLANG_RETURN_ON_FAIL(
        getEntryPointCode(entryPointIndex, targetIndex, diagnostics.writeRef(), code.writeRef()));

    auto linkage = getLinkage();

    auto target = linkage->targets[targetIndex];

    auto targetProgram = getTargetProgram(target);

    IArtifact* artifact = targetProgram->getExistingEntryPointResult(entryPointIndex);

    // Add diagnostics id needs be...
    if (diagnostics && !_findDiagnosticRepresentation(artifact))
    {
        // Add as an associated

        auto diagnosticsArtifact = Artifact::create(
            ArtifactDesc::make(Artifact::Kind::HumanText, ArtifactPayload::Diagnostics));
        diagnosticsArtifact->addRepresentationUnknown(diagnostics);

        artifact->addAssociated(diagnosticsArtifact);

        SLANG_ASSERT(diagnosticsArtifact == _findDiagnosticRepresentation(artifact));
    }

    // Add obfuscated source maps
    if (!_findObfuscatedSourceMap(artifact))
    {
        List<IRModule*> irModules;
        enumerateIRModules([&](IRModule* irModule) -> void { irModules.add(irModule); });

        for (auto irModule : irModules)
        {
            if (auto obfuscatedSourceMap = irModule->getObfuscatedSourceMap())
            {
                auto artifactDesc = ArtifactDesc::make(
                    ArtifactKind::Json,
                    ArtifactPayload::SourceMap,
                    ArtifactStyle::Obfuscated);

                // Create the source map artifact
                auto sourceMapArtifact = Artifact::create(
                    artifactDesc,
                    obfuscatedSourceMap->get().m_file.getUnownedSlice());

                sourceMapArtifact->addRepresentation(obfuscatedSourceMap);

                // associate with the artifact
                artifact->addAssociated(sourceMapArtifact);
            }
        }
    }

    // Turn into a file system and return
    ComPtr<ISlangMutableFileSystem> fileSystem(new MemoryFileSystem);

    // Filter the containerArtifact into things that can be written
    ComPtr<IArtifact> writeArtifact;
    SLANG_RETURN_ON_FAIL(ArtifactContainerUtil::filter(artifact, writeArtifact));
    SLANG_RETURN_ON_FAIL(ArtifactContainerUtil::writeContainer(writeArtifact, "", fileSystem));

    *outFileSystem = fileSystem.detach();

    return SLANG_OK;
}

SLANG_NO_THROW SlangResult SLANG_MCALL ComponentType::getEntryPointCode(
    SlangInt entryPointIndex,
    Int targetIndex,
    slang::IBlob** outCode,
    slang::IBlob** outDiagnostics)
{
    auto linkage = getLinkage();
    if (targetIndex < 0 || targetIndex >= linkage->targets.getCount())
        return SLANG_E_INVALID_ARG;
    auto target = linkage->targets[targetIndex];

    auto targetProgram = getTargetProgram(target);

    DiagnosticSink sink(linkage->getSourceManager(), Lexer::sourceLocationLexer);
    applySettingsToDiagnosticSink(&sink, &sink, linkage->m_optionSet);
    applySettingsToDiagnosticSink(&sink, &sink, m_optionSet);

    IArtifact* artifact = targetProgram->getOrCreateEntryPointResult(entryPointIndex, &sink);
    sink.getBlobIfNeeded(outDiagnostics);

    if (artifact == nullptr)
        return SLANG_FAIL;

    return artifact->loadBlob(ArtifactKeep::Yes, outCode);
}

SLANG_NO_THROW void SLANG_MCALL ComponentType::getEntryPointHash(
    SlangInt entryPointIndex,
    SlangInt targetIndex,
    slang::IBlob** outHash)
{
    DigestBuilder<SHA1> builder;

    // A note on enums that may be hashed in as part of the following two function calls:
    //
    // While enums are not guaranteed to be encoded the same way across all versions of
    // the compiler, part of hashing the linkage is hashing in the compiler version.
    // Consequently, any encoding differences as a result of different compiler versions
    // will already be reflected in the resulting hash.
    getLinkage()->buildHash(builder, targetIndex);

    buildHash(builder);

    // Add the name and name override for the specified entry point to the hash.
    auto entryPoint = getEntryPoint(entryPointIndex);
    if (entryPoint)
    {
        auto entryPointName = entryPoint->getName()->text;
        builder.append(entryPointName);
        auto entryPointMangledName = getEntryPointMangledName(entryPointIndex);
        builder.append(entryPointMangledName);
        auto entryPointNameOverride = getEntryPointNameOverride(entryPointIndex);
        builder.append(entryPointNameOverride);
    }

    auto hash = builder.finalize().toBlob();
    *outHash = hash.detach();
}

SLANG_NO_THROW SlangResult SLANG_MCALL ComponentType::getEntryPointHostCallable(
    int entryPointIndex,
    int targetIndex,
    ISlangSharedLibrary** outSharedLibrary,
    slang::IBlob** outDiagnostics)
{
    auto linkage = getLinkage();
    if (targetIndex < 0 || targetIndex >= linkage->targets.getCount())
        return SLANG_E_INVALID_ARG;
    auto target = linkage->targets[targetIndex];

    auto targetProgram = getTargetProgram(target);

    DiagnosticSink sink(linkage->getSourceManager(), Lexer::sourceLocationLexer);
    applySettingsToDiagnosticSink(&sink, &sink, m_optionSet);

    IArtifact* artifact = targetProgram->getOrCreateEntryPointResult(entryPointIndex, &sink);
    sink.getBlobIfNeeded(outDiagnostics);

    if (artifact == nullptr)
        return SLANG_FAIL;

    return artifact->loadSharedLibrary(ArtifactKeep::Yes, outSharedLibrary);
}

SLANG_NO_THROW SlangResult SLANG_MCALL ComponentType::getEntryPointMetadata(
    SlangInt entryPointIndex,
    Int targetIndex,
    slang::IMetadata** outMetadata,
    slang::IBlob** outDiagnostics)
{
    auto linkage = getLinkage();
    if (targetIndex < 0 || targetIndex >= linkage->targets.getCount())
        return SLANG_E_INVALID_ARG;
    auto target = linkage->targets[targetIndex];

    auto targetProgram = getTargetProgram(target);

    DiagnosticSink sink(linkage->getSourceManager(), Lexer::sourceLocationLexer);
    applySettingsToDiagnosticSink(&sink, &sink, linkage->m_optionSet);
    applySettingsToDiagnosticSink(&sink, &sink, m_optionSet);

    IArtifact* artifact = targetProgram->getOrCreateEntryPointResult(entryPointIndex, &sink);
    sink.getBlobIfNeeded(outDiagnostics);

    if (artifact == nullptr)
        return SLANG_E_NOT_AVAILABLE;

    auto metadata = findAssociatedRepresentation<IArtifactPostEmitMetadata>(artifact);
    if (!metadata)
        return SLANG_E_NOT_AVAILABLE;

    *outMetadata = static_cast<slang::IMetadata*>(metadata);
    (*outMetadata)->addRef();
    return SLANG_OK;
}

RefPtr<ComponentType> ComponentType::specialize(
    SpecializationArg const* inSpecializationArgs,
    SlangInt specializationArgCount,
    DiagnosticSink* sink)
{
    if (specializationArgCount == 0)
    {
        return this;
    }

    List<SpecializationArg> specializationArgs;
    specializationArgs.addRange(inSpecializationArgs, specializationArgCount);

    // We next need to validate that the specialization arguments
    // make sense, and also expand them to include any derived data
    // (e.g., interface conformance witnesses) that doesn't get
    // passed explicitly through the API interface.
    //
    Index consumedArgCount = 0;
    RefPtr<SpecializationInfo> specializationInfo = _validateSpecializationArgs(
        specializationArgs.getBuffer(),
        specializationArgCount,
        consumedArgCount,
        sink);
    if (consumedArgCount != specializationArgCount)
    {
        sink->diagnose(
            SourceLoc(),
            Diagnostics::mismatchSpecializationArguments,
            Math::Max(consumedArgCount, getSpecializationParamCount()),
            specializationArgCount);
    }
    if (sink->getErrorCount() != 0)
        return nullptr;
    return new SpecializedComponentType(this, specializationInfo, specializationArgs, sink);
}

SLANG_NO_THROW SlangResult SLANG_MCALL ComponentType::specialize(
    slang::SpecializationArg const* specializationArgs,
    SlangInt specializationArgCount,
    slang::IComponentType** outSpecializedComponentType,
    ISlangBlob** outDiagnostics)
{
    DiagnosticSink sink(getLinkage()->getSourceManager(), Lexer::sourceLocationLexer);

    List<SpecializationArg> expandedArgs;
    for (Int aa = 0; aa < specializationArgCount; ++aa)
    {
        auto apiArg = specializationArgs[aa];

        SpecializationArg expandedArg;
        switch (apiArg.kind)
        {
        case slang::SpecializationArg::Kind::Type:
            expandedArg.val = asInternal(apiArg.type);
            break;
        case slang::SpecializationArg::Kind::Expr:
            {
                auto parsedExpr = parseExprFromString(apiArg.expr, &sink);
                if (!parsedExpr)
                    return SLANG_FAIL;

                SharedSemanticsContext sharedSemanticsContext(getLinkage(), nullptr, &sink);
                SemanticsVisitor visitor(&sharedSemanticsContext);
                auto checkedExpr = visitor.CheckTerm(parsedExpr);
                if (auto typeType = as<TypeType>(checkedExpr->type.type))
                    expandedArg.val = typeType->getType();
                else
                    expandedArg.expr = checkedExpr;
            }
            break;
        default:
            sink.getBlobIfNeeded(outDiagnostics);
            return SLANG_FAIL;
        }
        expandedArgs.add(expandedArg);
    }

    auto specializedComponentType =
        specialize(expandedArgs.getBuffer(), expandedArgs.getCount(), &sink);

    sink.getBlobIfNeeded(outDiagnostics);

    *outSpecializedComponentType = specializedComponentType.detach();
    if (sink.getErrorCount() != 0)
        return SLANG_FAIL;
    return SLANG_OK;
}

SLANG_NO_THROW SlangResult SLANG_MCALL
ComponentType::renameEntryPoint(const char* newName, IComponentType** outEntryPoint)
{
    RefPtr<RenamedEntryPointComponentType> result =
        new RenamedEntryPointComponentType(this, newName);
    *outEntryPoint = result.detach();
    return SLANG_OK;
}

RefPtr<ComponentType> fillRequirements(ComponentType* inComponentType);

SLANG_NO_THROW SlangResult SLANG_MCALL
ComponentType::link(slang::IComponentType** outLinkedComponentType, ISlangBlob** outDiagnostics)
{
    // TODO: It should be possible for `fillRequirements` to fail,
    // in cases where we have a dependency that can't be automatically
    // resolved.
    //
    SLANG_UNUSED(outDiagnostics);

    DiagnosticSink sink(getLinkage()->getSourceManager(), Lexer::sourceLocationLexer);

    try
    {
        auto linked = fillRequirements(this);
        if (!linked)
            return SLANG_FAIL;

        *outLinkedComponentType = ComPtr<slang::IComponentType>(linked).detach();
        return SLANG_OK;
    }
    catch (const AbortCompilationException& e)
    {
        outputExceptionDiagnostic(e, sink, outDiagnostics);
        return SLANG_FAIL;
    }
    catch (const Exception& e)
    {
        outputExceptionDiagnostic(e, sink, outDiagnostics);
        return SLANG_FAIL;
    }
    catch (...)
    {
        outputExceptionDiagnostic(sink, outDiagnostics);
        return SLANG_FAIL;
    }
}

SLANG_NO_THROW SlangResult SLANG_MCALL ComponentType::linkWithOptions(
    slang::IComponentType** outLinkedComponentType,
    uint32_t count,
    slang::CompilerOptionEntry* entries,
    ISlangBlob** outDiagnostics)
{
    SLANG_RETURN_ON_FAIL(link(outLinkedComponentType, outDiagnostics));

    auto linked = *outLinkedComponentType;

    if (linked)
    {
        static_cast<ComponentType*>(linked)->getOptionSet().load(count, entries);
    }

    return SLANG_OK;
}

SLANG_NO_THROW SlangResult SLANG_MCALL ComponentType::getEntryPointCompileResult(
    SlangInt entryPointIndex,
    Int targetIndex,
    slang::ICompileResult** outCompileResult,
    slang::IBlob** outDiagnostics)
{
    auto linkage = getLinkage();
    if (targetIndex < 0 || targetIndex >= linkage->targets.getCount())
        return SLANG_E_INVALID_ARG;
    auto target = linkage->targets[targetIndex];

    auto targetProgram = getTargetProgram(target);

    DiagnosticSink sink(linkage->getSourceManager(), Lexer::sourceLocationLexer);
    applySettingsToDiagnosticSink(&sink, &sink, linkage->m_optionSet);
    applySettingsToDiagnosticSink(&sink, &sink, m_optionSet);

    IArtifact* artifact = targetProgram->getOrCreateEntryPointResult(entryPointIndex, &sink);
    sink.getBlobIfNeeded(outDiagnostics);
    if (artifact == nullptr)
        return SLANG_E_NOT_AVAILABLE;

    *outCompileResult = static_cast<slang::ICompileResult*>(artifact);
    (*outCompileResult)->addRef();
    return SLANG_OK;
}

SLANG_NO_THROW SlangResult SLANG_MCALL ComponentType::getTargetCompileResult(
    Int targetIndex,
    slang::ICompileResult** outCompileResult,
    slang::IBlob** outDiagnostics)
{
    IArtifact* artifact = getTargetArtifact(targetIndex, outDiagnostics);
    if (artifact == nullptr)
        return SLANG_E_NOT_AVAILABLE;

    *outCompileResult = static_cast<slang::ICompileResult*>(artifact);
    (*outCompileResult)->addRef();
    return SLANG_OK;
}

/// Visitor used by `ComponentType::enumerateModules`
struct EnumerateModulesVisitor : ComponentTypeVisitor
{
    EnumerateModulesVisitor(ComponentType::EnumerateModulesCallback callback, void* userData)
        : m_callback(callback), m_userData(userData)
    {
    }

    ComponentType::EnumerateModulesCallback m_callback;
    void* m_userData;

    void visitEntryPoint(EntryPoint*, EntryPoint::EntryPointSpecializationInfo*) SLANG_OVERRIDE {}

    void visitRenamedEntryPoint(
        RenamedEntryPointComponentType* entryPoint,
        EntryPoint::EntryPointSpecializationInfo* specializationInfo) SLANG_OVERRIDE
    {
        entryPoint->getBase()->acceptVisitor(this, specializationInfo);
    }

    void visitModule(Module* module, Module::ModuleSpecializationInfo*) SLANG_OVERRIDE
    {
        m_callback(module, m_userData);
    }

    void visitComposite(
        CompositeComponentType* composite,
        CompositeComponentType::CompositeSpecializationInfo* specializationInfo) SLANG_OVERRIDE
    {
        visitChildren(composite, specializationInfo);
    }

    void visitSpecialized(SpecializedComponentType* specialized) SLANG_OVERRIDE
    {
        visitChildren(specialized);
    }

    void visitTypeConformance(TypeConformance* conformance) SLANG_OVERRIDE
    {
        SLANG_UNUSED(conformance);
    }
};


void ComponentType::enumerateModules(EnumerateModulesCallback callback, void* userData)
{
    EnumerateModulesVisitor visitor(callback, userData);
    acceptVisitor(&visitor, nullptr);
}

/// Visitor used by `ComponentType::enumerateIRModules`
struct EnumerateIRModulesVisitor : ComponentTypeVisitor
{
    EnumerateIRModulesVisitor(ComponentType::EnumerateIRModulesCallback callback, void* userData)
        : m_callback(callback), m_userData(userData)
    {
    }

    ComponentType::EnumerateIRModulesCallback m_callback;
    void* m_userData;

    void visitEntryPoint(EntryPoint*, EntryPoint::EntryPointSpecializationInfo*) SLANG_OVERRIDE {}

    void visitRenamedEntryPoint(
        RenamedEntryPointComponentType* entryPoint,
        EntryPoint::EntryPointSpecializationInfo* specializationInfo) SLANG_OVERRIDE
    {
        entryPoint->getBase()->acceptVisitor(this, specializationInfo);
    }

    void visitModule(Module* module, Module::ModuleSpecializationInfo*) SLANG_OVERRIDE
    {
        m_callback(module->getIRModule(), m_userData);
    }

    void visitComposite(
        CompositeComponentType* composite,
        CompositeComponentType::CompositeSpecializationInfo* specializationInfo) SLANG_OVERRIDE
    {
        visitChildren(composite, specializationInfo);
    }

    void visitSpecialized(SpecializedComponentType* specialized) SLANG_OVERRIDE
    {
        visitChildren(specialized);

        m_callback(specialized->getIRModule(), m_userData);
    }

    void visitTypeConformance(TypeConformance* conformance) SLANG_OVERRIDE
    {
        m_callback(conformance->getIRModule(), m_userData);
    }
};

void ComponentType::enumerateIRModules(EnumerateIRModulesCallback callback, void* userData)
{
    EnumerateIRModulesVisitor visitor(callback, userData);
    acceptVisitor(&visitor, nullptr);
}

IArtifact* ComponentType::getTargetArtifact(Int targetIndex, slang::IBlob** outDiagnostics)
{
    auto linkage = getLinkage();
    if (targetIndex < 0 || targetIndex >= linkage->targets.getCount())
        return nullptr;
    ComPtr<IArtifact> artifact;
    if (m_targetArtifacts.tryGetValue(targetIndex, artifact))
    {
        return artifact.get();
    }
    try
    {
        // If the user hasn't specified any entry points, then we should
        // discover all entrypoints that are defined in linked modules, and
        // include all of them in the compile.
        //
        if (getEntryPointCount() == 0)
        {
            List<Module*> modules;
            this->enumerateModules([&](Module* module) { modules.add(module); });
            List<RefPtr<ComponentType>> components;
            components.add(this);
            bool entryPointsDiscovered = false;
            for (auto module : modules)
            {
                for (auto entryPoint : module->getEntryPoints())
                {
                    components.add(entryPoint);
                    entryPointsDiscovered = true;
                }
            }

            // If any entry points were discovered, then we should emit the program with entrypoints
            // linked.
            if (entryPointsDiscovered)
            {
                RefPtr<CompositeComponentType> composite =
                    new CompositeComponentType(linkage, components);
                ComPtr<IComponentType> linkedComponentType;
                SLANG_RETURN_NULL_ON_FAIL(
                    composite->link(linkedComponentType.writeRef(), outDiagnostics));
                auto targetArtifact = static_cast<ComponentType*>(linkedComponentType.get())
                                          ->getTargetArtifact(targetIndex, outDiagnostics);
                if (targetArtifact)
                {
                    m_targetArtifacts[targetIndex] = targetArtifact;
                }
                return targetArtifact;
            }
        }

        auto target = linkage->targets[targetIndex];
        auto targetProgram = getTargetProgram(target);

        DiagnosticSink sink(linkage->getSourceManager(), Lexer::sourceLocationLexer);
        applySettingsToDiagnosticSink(&sink, &sink, linkage->m_optionSet);
        applySettingsToDiagnosticSink(&sink, &sink, m_optionSet);

        IArtifact* targetArtifact = targetProgram->getOrCreateWholeProgramResult(&sink);
        sink.getBlobIfNeeded(outDiagnostics);
        m_targetArtifacts[targetIndex] = ComPtr<IArtifact>(targetArtifact);
        return targetArtifact;
    }
    catch (const Exception& e)
    {
        if (outDiagnostics && !*outDiagnostics)
        {
            DiagnosticSink sink(linkage->getSourceManager(), Lexer::sourceLocationLexer);
            applySettingsToDiagnosticSink(&sink, &sink, linkage->m_optionSet);
            applySettingsToDiagnosticSink(&sink, &sink, m_optionSet);
            sink.diagnose(
                SourceLoc(),
                Diagnostics::compilationAbortedDueToException,
                typeid(e).name(),
                e.Message);
            sink.getBlobIfNeeded(outDiagnostics);
        }
        return nullptr;
    }
}

SLANG_NO_THROW SlangResult SLANG_MCALL
ComponentType::getTargetCode(Int targetIndex, slang::IBlob** outCode, slang::IBlob** outDiagnostics)
{
    IArtifact* artifact = getTargetArtifact(targetIndex, outDiagnostics);

    if (artifact == nullptr)
        return SLANG_FAIL;

    return artifact->loadBlob(ArtifactKeep::Yes, outCode);
}

SLANG_NO_THROW SlangResult SLANG_MCALL ComponentType::getTargetMetadata(
    Int targetIndex,
    slang::IMetadata** outMetadata,
    slang::IBlob** outDiagnostics)
{
    IArtifact* artifact = getTargetArtifact(targetIndex, outDiagnostics);

    if (artifact == nullptr)
        return SLANG_FAIL;

    auto metadata = findAssociatedRepresentation<IArtifactPostEmitMetadata>(artifact);
    if (!metadata)
        return SLANG_E_NOT_AVAILABLE;
    *outMetadata = static_cast<slang::IMetadata*>(metadata);
    (*outMetadata)->addRef();
    return SLANG_OK;
}

Expr* ComponentType::parseExprFromString(String exprStr, DiagnosticSink* sink)
{
    auto linkage = getLinkage();
    SLANG_AST_BUILDER_RAII(linkage->getASTBuilder());
    auto astBuilder = linkage->getASTBuilder();
    Scope* scope = _getOrCreateScopeForLegacyLookup(astBuilder);
    Expr* expr = linkage->parseTermString(exprStr, scope);
    if (!expr || as<IncompleteExpr>(expr))
        sink->diagnose(SourceLoc(), Diagnostics::syntaxError);
    return expr;
}

Type* ComponentType::getTypeFromString(String const& typeStr, DiagnosticSink* sink)
{
    // If we've looked up this type name before,
    // then we can re-use it.
    //
    Type* type = nullptr;
    if (m_types.tryGetValue(typeStr, type))
        return type;

    auto astBuilder = getLinkage()->getASTBuilder();

    // Otherwise, we need to start looking in
    // the modules that were directly or
    // indirectly referenced.
    //
    Scope* scope = _getOrCreateScopeForLegacyLookup(astBuilder);

    auto linkage = getLinkage();

    SLANG_AST_BUILDER_RAII(linkage->getASTBuilder());

    Expr* typeExpr = linkage->parseTermString(typeStr, scope);
    SharedSemanticsContext sharedSemanticsContext(linkage, nullptr, sink);
    SemanticsVisitor visitor(&sharedSemanticsContext);
    type = visitor.TranslateTypeNode(typeExpr);
    auto typeOut = visitor.tryCoerceToProperType(TypeExp(type));
    if (typeOut.type)
        type = typeOut.type;

    if (type)
    {
        m_types[typeStr] = type;
    }
    return type;
}

Expr* ComponentType::tryResolveOverloadedExpr(Expr* exprIn)
{
    auto linkage = getLinkage();
    SemanticsContext context(linkage->getSemanticsForReflection());
    SemanticsVisitor visitor(context);
    return visitor.maybeResolveOverloadedExpr(exprIn, LookupMask::Function, nullptr);
}

// This function tries to simplify an overloaded expr into OverloadedExpr for reflection API usage.
// There are two kinds of overloaded expr in the AST: OverloadedExpr and OverloadedExpr2.
//
// OverloadedExpr stores candidates in LookupResult, where a list of `DeclRef<Decl>` hold
// the properly-specialized reference to the declaration that was found. And all the candidates
// must share a same base (if it is coming from a member-reference), and same orignalExpr.
//
// While OverloadedExpr2 stores candidates in a list of Expr, which is not necessary to be
// DeclRefExpr.
//
// When the input orignalExpr is already OverloadedExpr, we can directly return it. But when
// the input orignalExpr is OverloadedExpr2, we need to simplify it by converting it into
// OverloadedExpr. The conversion routine conservatively performs the conversion when each Expr
// candidates of OverloadedExpr2 is DeclRefExpr and all the candidates DeclRefExpr share the same
// orignalExpr. If such condition is not met, it will return nullptr to indicated failed conversion.
static Expr* maybeSimplifyExprForReflectionAPIUsage(Expr* originalExpr, ASTBuilder* astBuilder)
{
    // return directly if it is already OverloadedExpr
    if (as<OverloadedExpr>(originalExpr))
        return originalExpr;

    OverloadedExpr2* overloadedExpr2 = as<OverloadedExpr2>(originalExpr);
    // Don't perform any conversion if it is not OverloadedExpr2
    if (!overloadedExpr2)
        return originalExpr;

    if (!overloadedExpr2->candidateExprs.getCount())
        return nullptr;

    auto overloadedExpr = astBuilder->create<OverloadedExpr>();

    Expr* sharedOriginalExpr = nullptr;

    // Start the conversion
    for (auto candidate : overloadedExpr2->candidateExprs)
    {
        if (auto declRefExpr = as<DeclRefExpr>(candidate))
        {
            LookupResultItem item;
            item.declRef = declRefExpr->declRef;
            overloadedExpr->lookupResult2.items.add(item);

            if (!sharedOriginalExpr)
            {
                sharedOriginalExpr = declRefExpr->originalExpr;
            }
            else if (sharedOriginalExpr != declRefExpr->originalExpr)
            {
                return nullptr;
            }
        }
        else
        {
            return nullptr;
        }
    }

    if (overloadedExpr->lookupResult2.items.getCount())
    {
        overloadedExpr->lookupResult2.item = overloadedExpr->lookupResult2.items[0];
        overloadedExpr->base = overloadedExpr2->base;
        overloadedExpr->originalExpr = sharedOriginalExpr;
        return overloadedExpr;
    }

    return nullptr;
}

Expr* ComponentType::findDeclFromString(String const& name, DiagnosticSink* sink)
{
    // If we've looked up this type name before,
    // then we can re-use it.
    //
    Expr* result = nullptr;
    if (m_decls.tryGetValue(name, result))
        return result;


    // TODO(JS): For now just used the linkages ASTBuilder to keep on scope
    //
    // The parseTermString uses the linkage ASTBuilder for it's parsing.
    //
    // It might be possible to just create a temporary ASTBuilder - the worry though is
    // that the parsing sets a member variable in AST node to one of these scopes, and then
    // it become a dangling pointer. So for now we go with the linkages.
    auto astBuilder = getLinkage()->getASTBuilder();

    // Otherwise, we need to start looking in
    // the modules that were directly or
    // indirectly referenced.
    //
    Scope* scope = _getOrCreateScopeForLegacyLookup(astBuilder);

    auto linkage = getLinkage();

    SLANG_AST_BUILDER_RAII(linkage->getASTBuilder());

    Expr* expr = linkage->parseTermString(name, scope);

    SemanticsContext context(linkage->getSemanticsForReflection());
    context = context.allowStaticReferenceToNonStaticMember().withSink(sink);

    SemanticsVisitor visitor(context);

    auto checkedExpr = visitor.CheckTerm(expr);

    if (as<DeclRefExpr>(checkedExpr) || as<OverloadedExpr>(checkedExpr))
    {
        result = checkedExpr;
    }
    result = maybeSimplifyExprForReflectionAPIUsage(checkedExpr, astBuilder);

    m_decls[name] = result;
    return result;
}

static bool _isSimpleName(String const& name)
{
    for (char c : name)
    {
        if (!CharUtil::isAlphaOrDigit(c) && c != '_' && c != '$')
            return false;
    }
    return true;
}

Expr* ComponentType::findDeclFromStringInType(
    Type* type,
    String const& name,
    LookupMask mask,
    DiagnosticSink* sink)
{
    // Only look up in the type if it is a DeclRefType
    if (!as<DeclRefType>(type))
        return nullptr;

    // TODO(JS): For now just used the linkages ASTBuilder to keep on scope
    //
    // The parseTermString uses the linkage ASTBuilder for it's parsing.
    //
    // It might be possible to just create a temporary ASTBuilder - the worry though is
    // that the parsing sets a member variable in AST node to one of these scopes, and then
    // it become a dangling pointer. So for now we go with the linkages.
    auto astBuilder = getLinkage()->getASTBuilder();

    // Otherwise, we need to start looking in
    // the modules that were directly or
    // indirectly referenced.
    //
    Scope* scope = _getOrCreateScopeForLegacyLookup(astBuilder);

    auto linkage = getLinkage();

    SLANG_AST_BUILDER_RAII(linkage->getASTBuilder());

    Expr* expr = nullptr;

    if (_isSimpleName(name))
    {
        auto varExpr = astBuilder->create<VarExpr>();
        varExpr->scope = scope;
        varExpr->name = getLinkage()->getNamePool()->getName(name);
        expr = varExpr;
    }
    else
    {
        expr = linkage->parseTermString(name, scope);
    }
    SemanticsContext context(linkage->getSemanticsForReflection());
    context = context.allowStaticReferenceToNonStaticMember().withSink(sink);

    SemanticsVisitor visitor(context);

    GenericAppExpr* genericOuterExpr = nullptr;
    if (as<GenericAppExpr>(expr))
    {
        // Unwrap the generic application, and re-wrap it around the static-member expr
        genericOuterExpr = as<GenericAppExpr>(expr);
        expr = genericOuterExpr->functionExpr;
    }

    if (!as<VarExpr>(expr))
        return nullptr;

    auto rs = astBuilder->create<StaticMemberExpr>();
    auto typeExpr = astBuilder->create<SharedTypeExpr>();
    auto typetype = astBuilder->getOrCreate<TypeType>(type);
    typeExpr->type = typetype;
    rs->baseExpression = typeExpr;
    rs->name = as<VarExpr>(expr)->name;

    expr = rs;

    // If we have a generic-app expression, re-wrap the static-member expr
    if (genericOuterExpr)
    {
        genericOuterExpr->functionExpr = expr;
        expr = genericOuterExpr;
    }

    auto checkedTerm = visitor.CheckTerm(expr);

    checkedTerm = maybeSimplifyExprForReflectionAPIUsage(checkedTerm, astBuilder);

    if (auto overloadedExpr = as<OverloadedExpr>(checkedTerm))
    {
        // For functions, since we don't know the argument list yet, we will have to defer
        // non-parameter-related candidate comparison logic into its separate step.
        if (mask != LookupMask::Function)
            return visitor.maybeResolveOverloadedExpr(checkedTerm, mask, nullptr);
        overloadedExpr->lookupResult2 = refineLookup(overloadedExpr->lookupResult2, mask);

        // Filter out abstract base interface method implementations for reflection.
        if (!isInterfaceType(type))
        {
            LookupResult filteredResult;
            for (auto candidate : overloadedExpr->lookupResult2)
            {
                if (as<InterfaceDecl>(getParentDecl(candidate.declRef.getDecl())))
                {
                    if (!candidate.declRef.getDecl()
                             ->hasModifier<HasInterfaceDefaultImplModifier>())
                        continue;
                }
                AddToLookupResult(filteredResult, candidate);
            }
            if (filteredResult.isValid() && !filteredResult.isOverloaded())
            {
                // If there are exactly one candidate after filtering, we can
                // safely return resolved expr.
                return visitor.maybeResolveOverloadedExpr(checkedTerm, mask, nullptr);
            }
            overloadedExpr->lookupResult2 = filteredResult;
        }
        return overloadedExpr;
    }
    if (auto declRefExpr = as<DeclRefExpr>(checkedTerm))
    {
        return declRefExpr;
    }

    return nullptr;
}

bool ComponentType::isSubType(Type* subType, Type* superType)
{
    SemanticsContext context(getLinkage()->getSemanticsForReflection());
    SemanticsVisitor visitor(context);

    return (visitor.isSubtype(subType, superType, IsSubTypeOptions::None) != nullptr);
}

static void collectExportedConstantInContainer(
    Dictionary<String, IntVal*>& dict,
    ASTBuilder* builder,
    ContainerDecl* containerDecl)
{
    for (auto varMember : containerDecl->getDirectMemberDeclsOfType<VarDeclBase>())
    {
        if (!varMember->val)
            continue;
        bool isExported = false;
        bool isConst = false;
        bool isExtern = false;
        for (auto modifier : varMember->modifiers)
        {
            if (as<HLSLExportModifier>(modifier))
                isExported = true;
            if (as<ExternAttribute>(modifier) || as<ExternModifier>(modifier))
            {
                isExtern = true;
                isExported = true;
            }
            if (as<ConstModifier>(modifier))
                isConst = true;
        }
        if (isExported && isConst)
        {
            auto mangledName = getMangledName(builder, varMember);
            if (isExtern && dict.containsKey(mangledName))
                continue;
            dict[mangledName] = varMember->val;
        }
    }

    for (auto member : containerDecl->getDirectMemberDecls())
    {
        if (as<NamespaceDecl>(member) || as<FileDecl>(member))
        {
            collectExportedConstantInContainer(dict, builder, (ContainerDecl*)member);
        }
    }
}

Dictionary<String, IntVal*>& ComponentType::getMangledNameToIntValMap()
{
    if (m_mapMangledNameToIntVal)
    {
        return *m_mapMangledNameToIntVal;
    }
    m_mapMangledNameToIntVal = std::make_unique<Dictionary<String, IntVal*>>();
    auto astBuilder = getLinkage()->getASTBuilder();
    SLANG_AST_BUILDER_RAII(astBuilder);
    Scope* scope = _getOrCreateScopeForLegacyLookup(astBuilder);
    for (; scope; scope = scope->nextSibling)
    {
        if (scope->containerDecl)
            collectExportedConstantInContainer(
                *m_mapMangledNameToIntVal,
                astBuilder,
                scope->containerDecl);
    }
    return *m_mapMangledNameToIntVal;
}

ConstantIntVal* ComponentType::tryFoldIntVal(IntVal* intVal)
{
    auto astBuilder = getLinkage()->getASTBuilder();
    SLANG_AST_BUILDER_RAII(astBuilder);
    return as<ConstantIntVal>(intVal->linkTimeResolve(getMangledNameToIntValMap()));
}

TargetProgram* ComponentType::getTargetProgram(TargetRequest* target)
{
    RefPtr<TargetProgram> targetProgram;
    if (!m_targetPrograms.tryGetValue(target, targetProgram))
    {
        targetProgram = new TargetProgram(this, target);
        m_targetPrograms[target] = targetProgram;
    }
    return targetProgram;
}

} // namespace Slang
