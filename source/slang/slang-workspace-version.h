#pragma once

#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"
#include "../../slang.h"
#include "../core/slang-basic.h"
#include "../core/slang-com-object.h"
#include "../compiler-core/slang-language-server-protocol.h"
#include "slang-compiler.h"
#include "slang-doc-ast.h"

namespace Slang
{
    class Workspace;

    class DocumentVersion : public RefObject
    {
    private:
        URI uri;
        String text;
        List<UnownedStringSlice> lines;
        List<List<Index>> utf16CharStarts;
    public:
        void setURI(URI newURI)
        {
            uri = newURI;
        }
        URI getURI() { return uri; }
        const String& getText() { return text; }
        void setText(const String& newText);

        ArrayView<Index> getUTF16Boundaries(Index line);

        void oneBasedUTF8LocToZeroBasedUTF16Loc(
            Index inLine, Index inCol, Index& outLine, Index& outCol);
        void zeroBasedUTF16LocToOneBasedUTF8Loc(
            Index inLine, Index inCol, Index& outLine, Index& outCol);

        // Get starting offset of line.
        Index getLineStart(UnownedStringSlice line) { return line.begin() - text.begin(); }

        // Get offset from 1-based, utf-8 encoding location.
        Index getOffset(Index lineIndex, Index colIndex)
        {
            if(lineIndex < 0) return -1;
            if (lineIndex - 1 >= lines.getCount())
                return -1;
            if (lines.getCount() == 0)
                return -1;

            Index lineStart = lineIndex >= 1 ? getLineStart(lines[lineIndex - 1]) : 0;
            return lineStart + colIndex - 1;
        }

        // Get 1-based, utf-8 encoding location from offset.
        void offsetToLineCol(Index offset, Index& line, Index& col)
        {
            auto firstGreater = std::upper_bound(
                lines.begin(),
                lines.end(),
                offset,
                [this](Index first, UnownedStringSlice second)
                { return first < getLineStart(second); });
            line = Index(firstGreater - lines.begin());
            if (firstGreater == lines.begin())
            {
                col = offset + 1;
            }
            else
            {
                col = Index(offset - getLineStart(lines[line-1])) + 1;
            }
        }

        // Get line from 1-based index.
        UnownedStringSlice getLine(Index lineIndex)
        {
            if (lineIndex < 0)
                return UnownedStringSlice();
            if (lineIndex - 1 >= lines.getCount())
                return UnownedStringSlice();
            if (lines.getCount() == 0)
                return UnownedStringSlice();

            return lineIndex > 0 ? lines[lineIndex - 1] : UnownedStringSlice();
        }

        // Get length of an identifier token starting at the specified position.
        int getTokenLength(Index line, Index col);
    };

    struct DocumentDiagnostics
    {
        OrderedHashSet<LanguageServerProtocol::Diagnostic> messages;
        String originalOutput;
    };

    
    class SerializedModuleCache
        : public RefObject
        , public IModuleCache
    {
    public:
        Dictionary<String, List<uint8_t>> serializedModules;

        void invalidate(const String& path) { serializedModules.Remove(path); }
        virtual RefPtr<Module> tryLoadModule(Linkage* linkage, String filePath) override;
        virtual void storeModule(Linkage* linkage, String filePath, RefPtr<Module> module) override;
    };

    class WorkspaceVersion : public RefObject
    {
    private:
        Dictionary<String, Module*> modules;
        Dictionary<ModuleDecl*, RefPtr<ASTMarkup>> markupASTs;
        void parseDiagnostics(String compilerOutput);
    public:
        Workspace* workspace;
        RefPtr<Linkage> linkage;
        Dictionary<String, DocumentDiagnostics> diagnostics;
        List<Decl*> currentCompletionItems;
        ASTMarkup* getOrCreateMarkupAST(ModuleDecl* module);

        Module* getOrLoadModule(String path);
    };

    struct OnwedPreprocessorMacroDefinition
    {
        String name;
        String value;
    };
    class Workspace
        : public ISlangFileSystem
        , public ComObject
    {
    private:
        RefPtr<WorkspaceVersion> currentVersion;
        RefPtr<WorkspaceVersion> createWorkspaceVersion();
    public:
        List<String> rootDirectories;
        OrderedHashSet<String> searchPaths;
        List<OnwedPreprocessorMacroDefinition> predefinedMacros;
        SerializedModuleCache moduleCache;

        slang::IGlobalSession* slangGlobalSession;
        Dictionary<String, RefPtr<DocumentVersion>> openedDocuments;
        DocumentVersion* openDoc(String path, String text);
        void changeDoc(const String& path, LanguageServerProtocol::Range range, const String& text);
        void closeDoc(const String& path);

        // Update predefined macro settings. Returns true if the new settings are different from existing ones.
        bool updatePredefinedMacros(List<String> predefinedMacros);

        void init(List<URI> rootDirURI, slang::IGlobalSession* globalSession);
        void invalidate();
        WorkspaceVersion* getCurrentVersion();

    public:
        // Inherited via ISlangFileSystem
        SLANG_COM_OBJECT_IUNKNOWN_ALL
        void* getInterface(const Guid& uuid);
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL
            loadFile(const char* path, ISlangBlob** outBlob) override;
    };
} // namespace LanguageServerProtocol
