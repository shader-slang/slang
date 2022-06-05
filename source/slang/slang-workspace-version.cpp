#include "slang-workspace-version.h"
#include "../core/slang-io.h"
#include "../core/slang-file-system.h"
#include "../compiler-core/slang-lexer.h"

namespace Slang
{
struct DirEnumerationContext
{
    List<String> workList;
    String currentPath;
};
void Workspace::init(List<URI> rootDirURI, slang::IGlobalSession* globalSession)
{
    for (auto uri : rootDirURI)
    {
        auto path = uri.getPath();
        rootDirectories.add(path);
        DirEnumerationContext context;
        context.workList.add(path);
        auto fileSystem = Slang::OSFileSystem::getExtSingleton();
        for (int i = 0; i < context.workList.getCount(); i++)
        {
            context.currentPath = context.workList[i];
            fileSystem->enumeratePathContents(
                context.currentPath.getBuffer(),
                [](SlangPathType pathType, const char* name, void* userData)
                {
                    auto dirContext = (DirEnumerationContext*)userData;
                    if (pathType == SLANG_PATH_TYPE_DIRECTORY)
                    {
                        dirContext->workList.add(Path::combine(dirContext->currentPath, name));
                    }
                },
                &context);
        }
        searchPaths = _Move(context.workList);
    }
    slangGlobalSession = globalSession;
}

void Workspace::invalidate() { currentVersion = nullptr; }

int parseInt(UnownedStringSlice text, Index& pos)
{
    int result = 0;
    while (text[pos] == ' ' && pos < text.getLength())
    {
        pos++;
        continue;
    }
    while (pos < text.getLength())
    {
        if (text[pos] >= '0' && text[pos] <= '9')
        {
            result *= 10;
            result += text[pos] - '0';
            pos++;
        }
        else
        {
            break;
        }
    }
    return result;
}

void parseDiagnostics(Dictionary<String, OrderedHashSet<LanguageServerProtocol::Diagnostic>>& diagnostics, String compilerOutput)
{
    List<UnownedStringSlice> lines;
    StringUtil::split(compilerOutput.getUnownedSlice(), '\n', lines);
    for (Index lineIndex = 0; lineIndex < lines.getCount(); lineIndex++)
    {
        auto line = lines[lineIndex];
        Index colonIndex = line.indexOf(UnownedStringSlice("):"));
        if (colonIndex == -1)
            continue;
        Index lparentIndex = line.indexOf('(');
        if (lparentIndex > colonIndex)
            continue;
        String fileName = line.subString(0, lparentIndex);
        Path::getCanonical(fileName, fileName);
        auto& diagnosticList = diagnostics.GetOrAddValue(
            fileName, OrderedHashSet<LanguageServerProtocol::Diagnostic>());

        LanguageServerProtocol::Diagnostic diagnostic;
        Index pos = lparentIndex + 1;
        int lineLoc = parseInt(line, pos);
        if (lineLoc == 0)
            lineLoc = 1;
        diagnostic.range.end.line = diagnostic.range.start.line = lineLoc - 1;
        pos++;
        int colLoc = parseInt(line, pos);
        if (colLoc == 0)
            colLoc = 1;
        diagnostic.range.end.character = diagnostic.range.start.character = colLoc - 1;
        if (pos >= line.getLength())
            continue;
        line = line.subString(colonIndex + 3, line.getLength());
        colonIndex = line.indexOf(':');
        if (colonIndex == -1)
            continue;
        if (line.startsWith("error"))
        {
            diagnostic.severity = LanguageServerProtocol::kDiagnosticsSeverityError;
        }
        else if (line.startsWith("warning"))
        {
            diagnostic.severity = LanguageServerProtocol::kDiagnosticsSeverityWarning;
        }
        else if (line.startsWith("note"))
        {
            diagnostic.severity = LanguageServerProtocol::kDiagnosticsSeverityInformation;
        }
        else
        {
            continue;
        }
        pos = line.indexOf(' ');
        diagnostic.code = parseInt(line, pos);
        diagnostic.message = line.subString(colonIndex + 2, line.getLength());
        diagnosticList.Add(diagnostic);
        if (lineIndex + 1 < lines.getCount() && lines[lineIndex].startsWith("^+"))
        {
            lineIndex++;
            pos = 2;
            auto tokenLength = parseInt(lines[lineIndex], pos);
            diagnostic.range.end.character += tokenLength;
        }
    }
}

RefPtr<WorkspaceVersion> Workspace::createWorkspaceVersion()
{
    RefPtr<WorkspaceVersion> version = new WorkspaceVersion();
    version->workspace = this;
    slang::SessionDesc desc = {};
    desc.fileSystem = this;
    desc.targetCount = 1;
    desc.flags = slang::kSessionFlag_LanguageServer;
    slang::TargetDesc targetDesc = {};
    targetDesc.profile = slangGlobalSession->findProfile("sm_6_6");
    desc.targets = &targetDesc;

    List<const char*> searchPathsRaw;
    for (auto path : searchPaths)
        searchPathsRaw.add(path.getBuffer());
    desc.searchPaths = searchPathsRaw.getBuffer();
    desc.searchPathCount = searchPathsRaw.getCount();

    ComPtr<slang::ISession> session;
    slangGlobalSession->createSession(desc, session.writeRef());
    version->linkage = static_cast<Linkage*>(session.get());

    ComPtr<ISlangBlob> diagnosticBlob;
    StringBuilder sb;
    for (auto& doc : openedDocuments)
    {
        RefPtr<StringBlob> sourceBlob = new StringBlob(doc.Value->getText());
        auto parsedModule = version->linkage->loadModuleFromSource(
            Path::getFileNameWithoutExt(doc.Key).getBuffer(),
            doc.Key.getBuffer(),
            sourceBlob.Ptr(),
            diagnosticBlob.writeRef());
        if (diagnosticBlob)
        {
            sb << (char*)diagnosticBlob->getBufferPointer() << "\n";
        }
        if (parsedModule)
        {
            version->modules[doc.Key] = static_cast<Module*>(parsedModule);
        }
    }
    version->diagnosticOutput = sb.ProduceString();
    parseDiagnostics(version->diagnostics, version->diagnosticOutput);
    return version;
}

SlangResult Workspace::queryInterface(const SlangUUID& uuid, void** outObject)
{
    if (uuid == ISlangFileSystem::getTypeGuid())
    {
        *outObject = static_cast<ISlangFileSystem*>(this);
        return SLANG_OK;
    }
    return SLANG_E_NO_INTERFACE;
}

uint32_t Workspace::addRef() { return this->addRefImpl(); }

uint32_t Workspace::release() { return this->releaseImpl(); }

SlangResult Workspace::loadFile(const char* path, ISlangBlob** outBlob)
{
    String canonnicalPath;
    SLANG_RETURN_ON_FAIL(Path::getCanonical(path, canonnicalPath));
    RefPtr<DocumentVersion> doc;
    if (openedDocuments.TryGetValue(canonnicalPath, doc))
    {
        RefPtr<StringBlob> stringBlob = new StringBlob(doc->getText());
        *outBlob = stringBlob.detach();
        return SLANG_OK;
    }
    return Slang::OSFileSystem::getExtSingleton()->loadFile(path, outBlob);
}
WorkspaceVersion* Workspace::getCurrentVersion()
{
    if (!currentVersion)
        currentVersion = createWorkspaceVersion();
    return currentVersion.Ptr();
}

Int convertHexDigit(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return 0;
}

String URI::getPath() const
{
    Index startIndex = uri.indexOf("://");
    if (startIndex == -1) return String();
    startIndex += 3;
    Index endIndex = uri.indexOf('?');
    if (endIndex == -1)
        endIndex = uri.getLength();
    StringBuilder sb;
#if SLANG_WINDOWS_FAMILY
    if (uri[startIndex] == '/')
        startIndex++;
#endif
    for (Index i = startIndex; i < endIndex;)
    {
        auto ch = uri[i];
        if (ch == '%')
        {
            Int charVal = convertHexDigit(uri[i + 1]) * 16 + convertHexDigit(uri[i + 2]);
            sb.appendChar((char)charVal);
            i += 3;
        }
        else
        {
            sb.appendChar(uri[i]);
            i++;
        }
    }
    return sb.ProduceString();
}

bool URI::isSafeURIChar(char ch)
{
    return (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
           ch == '-' || ch == '_' || ch == '/' || ch == '.';
}

URI URI::fromLocalFilePath(UnownedStringSlice path)
{
    URI uri;
    StringBuilder sb;
    sb << "file://";

#if SLANG_WINDOWS_FAMILY
    sb << "/";
#endif

    for (auto ch : path)
    {
        if (isSafeURIChar(ch))
        {
            sb.appendChar(ch);
        }
        else if (ch == '\\')
        {
            sb.appendChar('/');
        }
        else
        {
            char buffer[32];
            int length = IntToAscii(buffer, (int)ch, 16);
            ReverseInternalAscii(buffer, length);
            sb << "%" << buffer;
        }
    }
    return URI::fromString(sb.getUnownedSlice());
}

URI URI::fromString(UnownedStringSlice uriString)
{
    URI uri;
    uri.uri = uriString;
    return uri;
}
void DocumentVersion::setText(const String& newText)
{
    text = newText;
    lineBreaks.clear();
    for (Index i = 0; i < newText.getLength(); i++)
    {
        if (newText[i] == '\n')
            lineBreaks.add(i);
    }
    lineBreaks.add(newText.getLength());
}
ASTMarkup* WorkspaceVersion::getOrCreateMarkupAST(ModuleDecl* module)
{
    RefPtr<ASTMarkup> astMarkup;
    if (markupASTs.TryGetValue(module, astMarkup))
        return astMarkup.Ptr();
    DiagnosticSink sink;
    astMarkup = new ASTMarkup();
    ASTMarkupUtil::extract(module, linkage->getSourceManager(), &sink, astMarkup.Ptr());
    markupASTs[module] = astMarkup;
    return astMarkup.Ptr();
}
} // namespace Slang
