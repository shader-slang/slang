// slang-translation-unit.h
#pragma once

//
// This file provides the `TranslationUnitRequest` class,
// which is used to represent the inputs to front-end compilation
// that will yield a single `Module`.
//

#include "compiler-core/slang-artifact.h"
#include "compiler-core/slang-source-loc.h"
#include "core/slang-smart-pointer.h"
#include "slang-compiler-fwd.h"
#include "slang-entry-point.h"
#include "slang-module.h"
#include "slang-profile.h"

namespace Slang
{

/// A request for the front-end to compile a translation unit.
class TranslationUnitRequest : public RefObject
{
public:
    TranslationUnitRequest(FrontEndCompileRequest* compileRequest);
    TranslationUnitRequest(FrontEndCompileRequest* compileRequest, Module* m);

    // The parent compile request
    FrontEndCompileRequest* compileRequest = nullptr;

    /// The effective source language of this translation unit.
    ///
    /// A translation unit has exactly one source language. The language explicitly requested by
    /// an API or command-line caller takes precedence over a language inferred from the primary
    /// source-file extensions. Source directives are expected to agree with that choice. For
    /// backward compatibility, preprocessing currently diagnoses but honors a conflicting
    /// `#version` or `#language` directive before parsing begins.
    ///
    /// Parser and semantic-checking code must use only this resolved field. In particular, the
    /// deprecated request-wide `-allow-glsl` option is normalized into this per-translation-unit
    /// value before preprocessing and has no independent meaning later in the pipeline.
    SourceLanguage sourceLanguage = SourceLanguage::Unknown;

    /// The language explicitly requested through the API or command-line options, if any.
    ///
    /// Normalizing the deprecated `-allow-glsl` option records `GLSL` here because that option's
    /// only remaining meaning is an explicit request to treat every translation unit as GLSL.
    SourceLanguage sourceLanguageExplicitlyRequested = SourceLanguage::Unknown;

    /// The language agreed on by the primary source-file extensions, if any.
    ///
    /// This field is `Unknown` when no extension identifies a language or when the extensions
    /// disagree. Disagreement is an error unless an explicitly requested language resolves it.
    SourceLanguage sourceLanguageImpliedByFileExtension = SourceLanguage::Unknown;

    /// The language selected by a primary source-file directive, if any.
    ///
    /// A conflicting directive currently overrides the request-level choice only as a backward-
    /// compatibility concession. The diagnostic makes such a request visible so this behavior can
    /// eventually become an error instead.
    SourceLanguage sourceLanguageImpliedBySourceContents = SourceLanguage::Unknown;

    /// The location of the directive that selected `sourceLanguageImpliedBySourceContents`.
    SourceLoc sourceLanguageImpliedBySourceContentsLoc;

    /// Makes any source artifact available as a SourceFile.
    /// If successful any of the source artifacts will be represented by the same index
    /// of sourceArtifacts
    SlangResult requireSourceFiles();

    /// Get the source files.
    /// Since lazily evaluated requires calling requireSourceFiles to know it's in sync
    /// with sourceArtifacts.
    List<SourceFile*> const& getSourceFiles() { return m_sourceFiles; }

    /// Get the source artifacts associated
    const List<ComPtr<IArtifact>>& getSourceArtifacts() const { return m_sourceArtifacts; }

    /// Clear all of the source
    void clearSource()
    {
        m_sourceArtifacts.clear();
        m_sourceFiles.clear();
        m_includedFileSet.clear();
    }

    /// Add a source artifact
    void addSourceArtifact(IArtifact* sourceArtifact);

    /// Add both the artifact and the sourceFile.
    void addSource(IArtifact* sourceArtifact, SourceFile* sourceFile);

    void addIncludedSourceFileIfNotExist(SourceFile* sourceFile);

    // The entry points associated with this translation unit
    List<RefPtr<EntryPoint>> const& getEntryPoints() { return module->getEntryPoints(); }

    void _addEntryPoint(EntryPoint* entryPoint) { module->_addEntryPoint(entryPoint); }

    // Preprocessor definitions to use for this translation unit only
    // (whereas the ones on `compileRequest` will be shared)
    Dictionary<String, String> preprocessorDefinitions;

    /// The name that will be used for the module this translation unit produces.
    Name* moduleName = nullptr;

    /// Result of compiling this translation unit (a module)
    RefPtr<Module> module;

    bool isChecked = false;

    Module* getModule() { return module; }
    ModuleDecl* getModuleDecl() { return module->getModuleDecl(); }

    Session* getSession();
    NamePool* getNamePool();
    SourceManager* getSourceManager();

    Scope* getLanguageScope();

    Dictionary<String, String> getCombinedPreprocessorDefinitions();

    void setModuleName(Name* name)
    {
        moduleName = name;
        if (module)
            module->setName(name);
    }

protected:
    void _addSourceFile(SourceFile* sourceFile);
    /* Given an artifact, find a PathInfo.
    If no PathInfo can be found will return an unknown PathInfo */
    PathInfo _findSourcePathInfo(IArtifact* artifact);

    List<ComPtr<IArtifact>> m_sourceArtifacts;
    // The source file(s) that will be compiled to form this translation unit
    //
    // Usually, for HLSL or GLSL there will be only one file.
    // NOTE! This member is generated lazily from m_sourceArtifacts
    // it is *necessary* to call requireSourceFiles to ensure it's in sync.
    List<SourceFile*> m_sourceFiles;

    // Track all the included source files added in m_sourceFiles
    HashSet<SourceFile*> m_includedFileSet;
};

} // namespace Slang
