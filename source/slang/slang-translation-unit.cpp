// slang-translation-unit.cpp
#include "slang-translation-unit.h"

#include "slang-compiler.h"

namespace Slang
{

//
// TranslationUnitRequest
//

TranslationUnitRequest::TranslationUnitRequest(FrontEndCompileRequest* compileRequest)
    : compileRequest(compileRequest)
{
    module = new Module(compileRequest->getLinkage());
}

TranslationUnitRequest::TranslationUnitRequest(FrontEndCompileRequest* compileRequest, Module* m)
    : compileRequest(compileRequest), module(m), isChecked(true)
{
    moduleName = getNamePool()->getName(m->getName());
}

Session* TranslationUnitRequest::getSession()
{
    return compileRequest->getSession();
}

NamePool* TranslationUnitRequest::getNamePool()
{
    return compileRequest->getNamePool();
}

SourceManager* TranslationUnitRequest::getSourceManager()
{
    return compileRequest->getSourceManager();
}

Scope* TranslationUnitRequest::getLanguageScope()
{
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
    return languageScope;
}

Dictionary<String, String> TranslationUnitRequest::getCombinedPreprocessorDefinitions()
{
    Dictionary<String, String> combinedPreprocessorDefinitions;
    for (const auto& def : preprocessorDefinitions)
        combinedPreprocessorDefinitions.addIfNotExists(def);
    for (const auto& def : compileRequest->optionSet.getArray(CompilerOptionName::MacroDefine))
        combinedPreprocessorDefinitions.addIfNotExists(def.stringValue, def.stringValue2);

    // Define standard macros, if not already defined. This style assumes using `#if __SOME_VAR`
    // style, as in
    //
    // ```
    // #if __SLANG_COMPILER__
    // ```
    //
    // This choice is made because slang outputs a warning on using a variable in an #if if not
    // defined
    //
    // Of course this means using #ifndef/#ifdef/defined() is probably not appropraite with thes
    // variables.
    {
        // Used to identify level of HLSL language compatibility
        combinedPreprocessorDefinitions.addIfNotExists("__HLSL_VERSION", "2018");

        // Indicates this is being compiled by the slang *compiler*
        combinedPreprocessorDefinitions.addIfNotExists("__SLANG_COMPILER__", "1");

        // Set macro depending on source type
        switch (sourceLanguage)
        {
        case SourceLanguage::HLSL:
            // Used to indicate compiled as HLSL language
            combinedPreprocessorDefinitions.addIfNotExists("__HLSL__", "1");
            break;
        case SourceLanguage::Slang:
            // Used to indicate compiled as Slang language
            combinedPreprocessorDefinitions.addIfNotExists("__SLANG__", "1");
            break;
        default:
            break;
        }

        // If not set, define as 0.
        combinedPreprocessorDefinitions.addIfNotExists("__HLSL__", "0");
        combinedPreprocessorDefinitions.addIfNotExists("__SLANG__", "0");
    }

    return combinedPreprocessorDefinitions;
}

void TranslationUnitRequest::addSourceArtifact(IArtifact* sourceArtifact)
{
    SLANG_ASSERT(sourceArtifact);
    m_sourceArtifacts.add(ComPtr<IArtifact>(sourceArtifact));
}


void TranslationUnitRequest::addSource(IArtifact* sourceArtifact, SourceFile* sourceFile)
{
    SLANG_ASSERT(sourceArtifact && sourceFile);
    // Must be in sync!
    SLANG_ASSERT(m_sourceFiles.getCount() == m_sourceArtifacts.getCount());

    addSourceArtifact(sourceArtifact);
    _addSourceFile(sourceFile);
}

void TranslationUnitRequest::addIncludedSourceFileIfNotExist(SourceFile* sourceFile)
{
    if (m_includedFileSet.contains(sourceFile))
        return;

    sourceFile->setIncludedFile();
    m_sourceFiles.add(sourceFile);
    m_includedFileSet.add(sourceFile);
}

PathInfo TranslationUnitRequest::_findSourcePathInfo(IArtifact* artifact)
{
    auto pathRep = findRepresentation<IPathArtifactRepresentation>(artifact);

    if (pathRep && pathRep->getPathType() == SLANG_PATH_TYPE_FILE)
    {
        // See if we have a unique identity set with the path
        if (const auto uniqueIdentity = pathRep->getUniqueIdentity())
        {
            return PathInfo::makeNormal(pathRep->getPath(), uniqueIdentity);
        }

        // If we couldn't get a unique identity, just use the path
        return PathInfo::makePath(pathRep->getPath());
    }

    // If there isn't a path, we can try with the name
    const char* name = artifact->getName();
    if (name && name[0] != 0)
    {
        return PathInfo::makeFromString(name);
    }

    return PathInfo::makeUnknown();
}

SlangResult TranslationUnitRequest::requireSourceFiles()
{
    SLANG_ASSERT(m_sourceFiles.getCount() <= m_sourceArtifacts.getCount());

    if (m_sourceFiles.getCount() == m_sourceArtifacts.getCount())
    {
        return SLANG_OK;
    }

    auto sink = compileRequest->getSink();
    SourceManager* sourceManager = compileRequest->getSourceManager();

    for (Index i = m_sourceFiles.getCount(); i < m_sourceArtifacts.getCount(); ++i)
    {
        IArtifact* artifact = m_sourceArtifacts[i];

        const PathInfo pathInfo = _findSourcePathInfo(artifact);

        SourceFile* sourceFile = nullptr;
        ComPtr<ISlangBlob> blob;

        // If we have a unique identity see if we have it already
        if (pathInfo.hasUniqueIdentity())
        {
            // See if this an already loaded source file
            sourceFile = sourceManager->findSourceFileRecursively(pathInfo.uniqueIdentity);
            // If we have a sourceFile see if it has a blob
            if (sourceFile)
            {
                blob = sourceFile->getContentBlob();
            }
        }

        // If we *don't* have a blob try and get a blob from the artifact
        if (!blob)
        {
            const SlangResult res = artifact->loadBlob(ArtifactKeep::Yes, blob.writeRef());
            if (SLANG_FAILED(res))
            {
                // Report couldn't load
                sink->diagnose(SourceLoc(), Diagnostics::cannotOpenFile, pathInfo.getName());
                return res;
            }
        }

        // If we don't have a blob on the artifact we can now add the one we have
        if (!findRepresentation<ISlangBlob>(artifact))
        {
            artifact->addRepresentationUnknown(blob);
        }

        // If we have a sourceFile check if it has contents, and set the blob if doesn't
        if (sourceFile)
        {
            if (!sourceFile->getContentBlob())
            {
                sourceFile->setContents(blob);
            }
        }
        else
        {
            // Create a new source file, using the pathInfo and blob
            sourceFile = sourceManager->createSourceFileWithBlob(pathInfo, blob);
        }

        auto uniqueIdentity = pathInfo.getMostUniqueIdentity();
        if (uniqueIdentity.getLength())
            sourceManager->addSourceFileIfNotExist(uniqueIdentity, sourceFile);

        // Finally add the source file
        _addSourceFile(sourceFile);
    }

    return SLANG_OK;
}

void TranslationUnitRequest::_addSourceFile(SourceFile* sourceFile)
{
    m_sourceFiles.add(sourceFile);

    getModule()->addFileDependency(sourceFile);
    getModule()->getIncludedSourceFileMap().add(sourceFile, nullptr);
}

List<SourceFile*> const& TranslationUnitRequest::getSourceFiles()
{
    return m_sourceFiles;
}

} // namespace Slang
