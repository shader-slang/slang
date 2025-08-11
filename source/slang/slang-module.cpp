// slang-module.cpp
#include "slang-module.h"

#include "slang-check-impl.h"
#include "slang-compiler.h"
#include "slang-mangle.h"
#include "slang-serialize-container.h"

namespace Slang
{

//
// Module
//

Module::Module(Linkage* linkage, ASTBuilder* astBuilder)
    : ComponentType(linkage), m_mangledExportPool(StringSlicePool::Style::Empty)
{
    if (astBuilder)
    {
        m_astBuilder = astBuilder;
    }
    else
    {
        m_astBuilder = linkage->getASTBuilder();
    }
    getOptionSet() = linkage->m_optionSet;
    addModuleDependency(this);
}

ISlangUnknown* Module::getInterface(const Guid& guid)
{
    if (guid == IModule::getTypeGuid())
        return asExternal(this);
    if (guid == IModulePrecompileService_Experimental::getTypeGuid())
        return static_cast<slang::IModulePrecompileService_Experimental*>(this);
    return Super::getInterface(guid);
}

void Module::buildHash(DigestBuilder<SHA1>& builder)
{
    builder.append(computeDigest());
}

slang::DeclReflection* Module::getModuleReflection()
{
    return (slang::DeclReflection*)m_moduleDecl;
}

SHA1::Digest Module::computeDigest()
{
    if (m_digest == SHA1::Digest())
    {
        DigestBuilder<SHA1> digestBuilder;
        auto version = String(getBuildTagString());
        digestBuilder.append(version);
        getOptionSet().buildHash(digestBuilder);

        auto fileDependencies = getFileDependencies();

        for (auto file : fileDependencies)
        {
            digestBuilder.append(file->getDigest());
        }
        m_digest = digestBuilder.finalize();
    }
    return m_digest;
}

void Module::addModuleDependency(Module* module)
{
    m_moduleDependencyList.addDependency(module);
    m_fileDependencyList.addDependency(module);
}

void Module::addFileDependency(SourceFile* sourceFile)
{
    m_fileDependencyList.addDependency(sourceFile);
}

void Module::setModuleDecl(ModuleDecl* moduleDecl)
{
    m_moduleDecl = moduleDecl;
    moduleDecl->module = this;
}

void Module::setName(String name)
{
    m_name = getLinkage()->getNamePool()->getName(name);
}


RefPtr<EntryPoint> Module::findEntryPointByName(UnownedStringSlice const& name)
{
    for (auto entryPoint : m_entryPoints)
    {
        if (entryPoint->getName()->text.getUnownedSlice() == name)
            return entryPoint;
    }

    return nullptr;
}

RefPtr<EntryPoint> Module::findAndCheckEntryPoint(
    UnownedStringSlice const& name,
    SlangStage stage,
    ISlangBlob** outDiagnostics)
{
    // If there is already an entrypoint marked with the [shader] attribute,
    // we should just return that.
    //
    if (auto existingEntryPoint = findEntryPointByName(name))
        return existingEntryPoint;

    SLANG_AST_BUILDER_RAII(m_astBuilder);

    // If the function hasn't been marked as [shader], then it won't be discovered
    // by findEntryPointByName. We need to route this to the `findAndValidateEntryPoint`
    // function. To do that we need to setup a FrontEndCompileRequest and a
    // FrontEndEntryPointRequest.
    //
    DiagnosticSink sink(getLinkage()->getSourceManager(), DiagnosticSink::SourceLocationLexer());
    FrontEndCompileRequest frontEndRequest(getLinkage(), StdWriters::getSingleton(), &sink);
    RefPtr<TranslationUnitRequest> tuRequest = new TranslationUnitRequest(&frontEndRequest);
    tuRequest->module = this;
    tuRequest->moduleName = m_name;
    frontEndRequest.translationUnits.add(tuRequest);
    FrontEndEntryPointRequest entryPointRequest(
        &frontEndRequest,
        0,
        getLinkage()->getNamePool()->getName(name),
        Profile((Stage)stage));
    auto result = findAndValidateEntryPoint(&entryPointRequest);
    if (outDiagnostics)
    {
        sink.getBlobIfNeeded(outDiagnostics);
    }
    return result;
}

void Module::_addEntryPoint(EntryPoint* entryPoint)
{
    m_entryPoints.add(entryPoint);
}

static bool _canExportDeclSymbol(ASTNodeType type)
{
    switch (type)
    {
    case ASTNodeType::EmptyDecl:
        {
            return false;
        }
    default:
        break;
    }

    return true;
}

static bool _canRecurseExportSymbol(Decl* decl)
{
    if (as<FunctionDeclBase>(decl) || as<ScopeDecl>(decl))
    {
        return false;
    }
    return true;
}

void Module::_processFindDeclsExportSymbolsRec(Decl* decl)
{
    if (_canExportDeclSymbol(decl->astNodeType))
    {
        // It's a reference to a declaration in another module, so first get the symbol name.
        String mangledName = getMangledName(getCurrentASTBuilder(), decl);

        Index index = Index(m_mangledExportPool.add(mangledName));

        // TODO(JS): It appears that more than one entity might have the same mangled name.
        // So for now we ignore and just take the first one.
        if (index == m_mangledExportSymbols.getCount())
        {
            m_mangledExportSymbols.add(decl);
        }
    }

    if (!_canRecurseExportSymbol(decl))
    {
        // We don't need to recurse any further into this
        return;
    }

    // If it's a container process it's children
    if (auto containerDecl = as<ContainerDecl>(decl))
    {
        for (auto child : containerDecl->getDirectMemberDecls())
        {
            _processFindDeclsExportSymbolsRec(child);
        }
    }

    // GenericDecl is also a container, so do subsequent test
    if (auto genericDecl = as<GenericDecl>(decl))
    {
        _processFindDeclsExportSymbolsRec(genericDecl->inner);
    }
}

Decl* Module::findExportedDeclByMangledName(const UnownedStringSlice& mangledName)
{
    // If this module is a serialized module that is being
    // deserialized on-demand, then we want to use the
    // mangled name mapping that was baked into the serialized
    // data, rather than attempt to enumerate all of the declarations
    // in the module (as would be done if we proceeded to call
    // `ensureExportLookupAcceleratorBuilt()`).
    //
    if (this->m_moduleDecl->isUsingOnDemandDeserializationForExports())
    {
        return m_moduleDecl->_findSerializedDeclByMangledExportName(mangledName);
    }

    ensureExportLookupAcceleratorBuilt();

    const Index index = m_mangledExportPool.findIndex(mangledName);
    return (index >= 0) ? m_mangledExportSymbols[index] : nullptr;
}

void Module::ensureExportLookupAcceleratorBuilt()
{
    // Will be non zero if has been previously attempted
    if (m_mangledExportSymbols.getCount() == 0)
    {
        // Build up the exported mangled name list
        _processFindDeclsExportSymbolsRec(getModuleDecl());

        // If nothing found, mark that we have tried looking by making
        // m_mangledExportSymbols.getCount() != 0
        if (m_mangledExportSymbols.getCount() == 0)
        {
            m_mangledExportSymbols.add(nullptr);
        }
    }
}

Count Module::getExportedDeclCount()
{
    ensureExportLookupAcceleratorBuilt();

    return m_mangledExportPool.getSlicesCount();
}

Decl* Module::getExportedDecl(Index index)
{
    ensureExportLookupAcceleratorBuilt();
    return m_mangledExportSymbols[index];
}

UnownedStringSlice Module::getExportedDeclMangledName(Index index)
{
    ensureExportLookupAcceleratorBuilt();
    return m_mangledExportPool.getSlices()[index];
}

SLANG_NO_THROW SlangResult SLANG_MCALL Module::serialize(ISlangBlob** outSerializedBlob)
{
    SerialContainerUtil::WriteOptions writeOptions;
    OwnedMemoryStream memoryStream(FileAccess::Write);
    SLANG_RETURN_ON_FAIL(SerialContainerUtil::write(this, writeOptions, &memoryStream));
    *outSerializedBlob = RawBlob::create(
                             memoryStream.getContents().getBuffer(),
                             (size_t)memoryStream.getContents().getCount())
                             .detach();
    return SLANG_OK;
}

SLANG_NO_THROW SlangResult SLANG_MCALL Module::writeToFile(char const* fileName)
{
    SerialContainerUtil::WriteOptions writeOptions;
    FileStream fileStream;
    SLANG_RETURN_ON_FAIL(fileStream.init(fileName, FileMode::Create));
    return SerialContainerUtil::write(this, writeOptions, &fileStream);
}

SLANG_NO_THROW const char* SLANG_MCALL Module::getName()
{
    if (m_name)
        return m_name->text.getBuffer();
    return nullptr;
}

SLANG_NO_THROW const char* SLANG_MCALL Module::getFilePath()
{
    if (m_pathInfo.hasFoundPath())
        return m_pathInfo.foundPath.getBuffer();
    return nullptr;
}

SLANG_NO_THROW const char* SLANG_MCALL Module::getUniqueIdentity()
{
    if (m_pathInfo.hasUniqueIdentity())
        return m_pathInfo.getMostUniqueIdentity().getBuffer();
    return nullptr;
}

SLANG_NO_THROW SlangInt32 SLANG_MCALL Module::getDependencyFileCount()
{
    return (SlangInt32)getFileDependencies().getCount();
}

SLANG_NO_THROW char const* SLANG_MCALL Module::getDependencyFilePath(SlangInt32 index)
{
    SourceFile* sourceFile = getFileDependencies()[index];
    return sourceFile->getPathInfo().hasFoundPath()
               ? sourceFile->getPathInfo().getMostUniqueIdentity().getBuffer()
               : nullptr;
}

void Module::_discoverEntryPoints(DiagnosticSink* sink, const List<RefPtr<TargetRequest>>& targets)
{
    if (m_entryPoints.getCount() > 0)
        return;
    _discoverEntryPointsImpl(m_moduleDecl, sink, targets);
}
void Module::_discoverEntryPointsImpl(
    ContainerDecl* containerDecl,
    DiagnosticSink* sink,
    const List<RefPtr<TargetRequest>>& targets)
{
    for (auto globalDecl : containerDecl->getDirectMemberDecls())
    {
        auto maybeFuncDecl = globalDecl;
        if (auto genericDecl = as<GenericDecl>(maybeFuncDecl))
        {
            maybeFuncDecl = genericDecl->inner;
        }

        if (as<NamespaceDeclBase>(globalDecl) || as<FileDecl>(globalDecl) ||
            as<StructDecl>(globalDecl))
        {
            _discoverEntryPointsImpl(as<ContainerDecl>(globalDecl), sink, targets);
            continue;
        }

        auto funcDecl = as<FuncDecl>(maybeFuncDecl);
        if (!funcDecl)
            continue;

        Profile profile;
        bool resolvedStageOfProfileWithEntryPoint = resolveStageOfProfileWithEntryPoint(
            profile,
            getLinkage()->m_optionSet,
            targets,
            funcDecl,
            sink);
        if (!resolvedStageOfProfileWithEntryPoint)
        {
            // If there isn't a [shader] attribute, look for a [numthreads] attribute
            // since that implicitly means a compute shader. We'll not do this when compiling for
            // CUDA/Torch since [numthreads] attributes are utilized differently for those targets.
            //

            bool allTargetsCUDARelated = true;
            for (auto target : targets)
            {
                if (!isCUDATarget(target) &&
                    target->getTarget() != CodeGenTarget::PyTorchCppBinding)
                {
                    allTargetsCUDARelated = false;
                    break;
                }
            }

            if (allTargetsCUDARelated && targets.getCount() > 0)
                continue;

            bool canDetermineStage = false;
            for (auto modifier : funcDecl->modifiers)
            {
                if (as<NumThreadsAttribute>(modifier))
                {
                    if (funcDecl->findModifier<OutputTopologyAttribute>())
                        profile.setStage(Stage::Mesh);
                    else
                        profile.setStage(Stage::Compute);
                    canDetermineStage = true;
                    break;
                }
                else if (as<PatchConstantFuncAttribute>(modifier))
                {
                    profile.setStage(Stage::Hull);
                    canDetermineStage = true;
                    break;
                }
            }
            if (!canDetermineStage)
                continue;
        }

        RefPtr<EntryPoint> entryPoint =
            EntryPoint::create(getLinkage(), makeDeclRef(funcDecl), profile);

        validateEntryPoint(entryPoint, sink);

        // Note: in the case that the user didn't explicitly
        // specify entry points and we are instead compiling
        // a shader "library," then we do not want to automatically
        // combine the entry points into groups in the generated
        // `Program`, since that would be slightly too magical.
        //
        // Instead, each entry point will end up in a singleton
        // group, so that its entry-point parameters lay out
        // independent of the others.
        //
        _addEntryPoint(entryPoint);
    }
}


} // namespace Slang
