// slang-state-serialize.h
#ifndef SLANG_STATE_SERIALIZE_H_INCLUDED
#define SLANG_STATE_SERIALIZE_H_INCLUDED

#include "../core/slang-riff.h"
#include "../core/slang-string.h"

// For TranslationUnitRequest
#include "slang-compiler.h"

#include "../core/slang-relative-container.h"

#include "slang-file-system.h"

namespace Slang {

struct StateSerializeUtil
{
    static const uint32_t kSlangStateFourCC = SLANG_FOUR_CC('S', 'L', 'S', 'T');             ///< Holds all the slang specific chunks
    
    struct Header
    {
        RiffChunk m_chunk;
        uint32_t m_compressionType;         ///< Holds the compression type used (if used at all)
    };

    struct FileState
    {
        Relative32Ptr<RelativeString> uniqueIdentity;           ///< The unique identity for the file (from ISlangFileSystem), or nullptr
        Relative32Ptr<RelativeString> contents;                 ///< The contents of this file
        Relative32Ptr<RelativeString> canonicalPath;            ///< The canonical name of this file (or nullptr)
        Relative32Ptr<RelativeString> foundPath;                ///< The 'found' path

        Relative32Ptr<RelativeString> uniqueName;               ///< A generated unique name (not used by slang, but used as mechanism to replace files)
    };

    struct PathInfoState
    {
        typedef CacheFileSystem::CompressedResult CompressedResult;

        SlangPathType pathType = SLANG_PATH_TYPE_FILE;
        CompressedResult loadFileResult = CompressedResult::Uninitialized;
        CompressedResult getPathTypeResult = CompressedResult::Uninitialized;
        CompressedResult getCanonicalPathResult = CompressedResult::Uninitialized;

        Relative32Ptr<FileState> file;                          ///< File contents
    };

    struct PathAndPathInfo
    {
        Relative32Ptr<RelativeString> path;
        Relative32Ptr<PathInfoState> pathInfo;
    };

        // spSetCodeGenTarget/spAddCodeGenTarget
        // spSetTargetProfile
        // spSetTargetFlags
        // spSetTargetFloatingPointMode
        // spSetTargetMatrixLayoutMode
    struct TargetRequestState
    {
        Profile profile;
        CodeGenTarget target;
        SlangTargetFlags targetFlags;
        FloatingPointMode floatingPointMode;
    };

    struct FileReference
    {
        Relative32Ptr<RelativeString> name;
        Relative32Ptr<FileState> file;
    };

    struct StringPair
    {
        Relative32Ptr<RelativeString> first;
        Relative32Ptr<RelativeString> second;
    };

    struct SourceFileState
    {
        PathInfo::Type type;                           ///< The type of this file
        Relative32Ptr<RelativeString> foundPath;       ///< The Path this was found along
        Relative32Ptr<FileState> file;                 ///< The file contents
    };

        // spAddTranslationUnit
    struct TranslationUnitRequestState
    {
        SourceLanguage language;

        Relative32Ptr<RelativeString> moduleName;

        // spTranslationUnit_addPreprocessorDefine
        Relative32Array<StringPair> preprocessorDefinitions;

        Relative32Array<Relative32Ptr<SourceFileState> > sourceFiles;
    };

    
    struct RequestState
    {
        Relative32Array<Relative32Ptr<FileState>> files;                   ///< All of the files
        Relative32Array<Relative32Ptr<SourceFileState>> sourceFiles;       ///< All of the source files (from source manager)

            // spSetCompileFlags
        SlangCompileFlags compileFlags;
            // spSetDumpIntermediates
        bool shouldDumpIntermediates;
            // spSetLineDirectiveMode
        LineDirectiveMode lineDirectiveMode;

        Relative32Array<TargetRequestState> targetRequests;

            // spSetDebugInfoLevel
        DebugInfoLevel debugInfoLevel;
            // spSetOptimizationLevel
        OptimizationLevel optimizationLevel;
            // spSetOutputContainerFormat
        ContainerFormat containerFormat;
            // spSetPassThrough
        PassThroughMode passThroughMode;

            // spAddSearchPath
        Relative32Array<Relative32Ptr<RelativeString> > searchPaths;

            // spAddPreprocessorDefine
        Relative32Array<StringPair> preprocessorDefinitions;

        Relative32Array<PathAndPathInfo> pathInfoMap;                  ///< Stores all the accesses to the file system

        Relative32Array<TranslationUnitRequestState> translationUnits;

        SlangMatrixLayoutMode defaultMatrixLayoutMode;
    };

    static SlangResult store(EndToEndCompileRequest* request, RelativeContainer& inOutContainer, Safe32Ptr<RequestState>& outRequest);
    
    static SlangResult saveState(EndToEndCompileRequest* request, const String& filename);

    static SlangResult saveState(EndToEndCompileRequest* request, Stream* stream);

        /// Load the requestState into request
        /// The fileSystem is optional and can be passed as nullptr. If set, as each file is loaded
        /// it will attempt to load from fileSystem the *uniqueName*
    static SlangResult load(RequestState* requestState, ISlangFileSystem* fileSystem, EndToEndCompileRequest* request);

    static SlangResult loadState(const String& filename, List<uint8_t>& outBuffer);
    static SlangResult loadState(Stream* stream, List<uint8_t>& outBuffer);
    static SlangResult loadState(const uint8_t* data, size_t size, List<uint8_t>& outBuffer);

    static RequestState* getRequest(const List<uint8_t>& inBuffer);

    static SlangResult extractFilesToDirectory(const String& file);

    static SlangResult extractFiles(RequestState* requestState, ISlangFileSystemExt* fileSystem);

        /// Given the repo file work out a suitable path
    static SlangResult calcDirectoryPathFromFilename(const String& filename, String& outPath);
};

} // namespace Slang

#endif
