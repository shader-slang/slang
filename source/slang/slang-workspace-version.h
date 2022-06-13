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
        List<Int> lineBreaks;
    public:
        void setURI(URI newURI)
        {
            uri = newURI;
        }
        URI getURI() { return uri; }
        const String& getText() { return text; }
        void setText(const String& newText);
        Index getOffset(Index lineIndex, Index colIndex)
        {
            if(lineIndex < 0) return -1;
            if (lineIndex - 1 >= lineBreaks.getCount())
                return -1;
            if (lineBreaks.getCount() == 0)
                return -1;

            Index lineStart = lineIndex >= 2 ? lineBreaks[lineIndex - 2] : 0;
            return lineStart + colIndex - 1;
        }
        void offsetToLineCol(Index offset, Index& line, Index& col)
        {
            auto firstGreater = std::upper_bound(lineBreaks.begin(), lineBreaks.end(), offset);
            line = Index(firstGreater - lineBreaks.begin() + 1);
            if (firstGreater == lineBreaks.begin())
            {
                col = offset + 1;
            }
            else
            {
                col = Index(offset - *(firstGreater - 1));
            }
        }
        UnownedStringSlice getLine(Index lineIndex)
        {
            if (lineIndex < 0)
                return UnownedStringSlice();
            if (lineIndex - 1 >= lineBreaks.getCount())
                return UnownedStringSlice();
            if (lineBreaks.getCount() == 0)
                return UnownedStringSlice();

            Int lineStart = lineIndex >= 2 ? lineBreaks[lineIndex - 2] : 0;
            Int lineEnd = lineBreaks[lineIndex - 1];
            return text.getUnownedSlice().subString(lineStart, lineEnd);
        }
    };

    struct DocumentDiagnostics
    {
        OrderedHashSet<LanguageServerProtocol::Diagnostic> messages;
        String originalOutput;
    };

    class WorkspaceVersion : public RefObject
    {
    private:
        Dictionary<String, Module*> modules;
        Dictionary<ModuleDecl*, RefPtr<ASTMarkup>> markupASTs;
    public:
        Workspace* workspace;
        RefPtr<Linkage> linkage;
        Dictionary<String, DocumentDiagnostics> diagnostics;
        List<Decl*> currentCompletionItems;
        ASTMarkup* getOrCreateMarkupAST(ModuleDecl* module);

        Module* getOrLoadModule(String path);
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

        slang::IGlobalSession* slangGlobalSession;
        Dictionary<String, RefPtr<DocumentVersion>> openedDocuments;
        DocumentVersion* openDoc(String path, String text);
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
