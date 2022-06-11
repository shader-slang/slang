#include "slang-workspace-version.h"
#include "../core/slang-io.h"
#include "../core/slang-file-system.h"
#include "../compiler-core/slang-lexer.h"
#include "slang-serialize-container.h"

namespace Slang
{
struct DirEnumerationContext
{
    List<String> workList;
    OrderedHashSet<String> paths;
    String currentPath;
    String root;
    void addSearchPath(String path)
    {
        while (path.getLength())
        {
            String canonicalPath;
            Path::getCanonical(path, canonicalPath);
            if (!paths.Add(canonicalPath))
                break;
            path = Path::getParentDirectory(path);
            if (!path.startsWith(root))
                break;
        }
    }
};
DocumentVersion* Workspace::openDoc(String path, String text)
{
    RefPtr<DocumentVersion> doc = new DocumentVersion();
    doc->setText(text.getUnownedSlice());
    doc->setURI(URI::fromLocalFilePath(path.getUnownedSlice()));
    openedDocuments[path] = doc;
    searchPaths.Add(Path::getParentDirectory(path));
    moduleCache.invalidate(path);
    invalidate();
    return doc.Ptr();
}

void Workspace::changeDoc(const String& path, LanguageServerProtocol::Range range, const String& text)
{
    RefPtr<DocumentVersion> doc;
    if (openedDocuments.TryGetValue(path, doc))
    {
        Index line, col;
        doc->zeroBasedUTF16LocToOneBasedUTF8Loc(range.start.line, range.start.character, line, col);
        auto startOffset = doc->getOffset(line, col);
        doc->zeroBasedUTF16LocToOneBasedUTF8Loc(range.end.line, range.end.character, line, col);
        auto endOffset = doc->getOffset(line, col);
        auto originalText = doc->getText().getUnownedSlice();
        StringBuilder newText;
        newText << originalText.head(startOffset) << text << originalText.tail(endOffset);
        doc->setText(newText.ProduceString());
    }
    moduleCache.invalidate(path);
    invalidate();
}

void Workspace::closeDoc(const String& path)
{
    moduleCache.invalidate(path);
    openedDocuments.Remove(path);
    invalidate();
}

bool Workspace::updatePredefinedMacros(List<String> macros)
{
    List<OnwedPreprocessorMacroDefinition> newDefs;
    for (auto macro : macros)
    {
        auto index = macro.indexOf('=');
        OnwedPreprocessorMacroDefinition def;
        def.name = macro.getUnownedSlice().head(index).trim();
        if (index != -1)
        {
            def.value = macro.getUnownedSlice().tail(index + 1).trim();
        }
        newDefs.add(def);
    }
    
    bool changed = false;
    if (newDefs.getCount() != predefinedMacros.getCount())
        changed = true;
    else
    {
        for (Index i = 0; i < newDefs.getCount(); i++)
        {
            if (newDefs[i].name != predefinedMacros[i].name ||
                newDefs[i].value != predefinedMacros[i].value)
            {
                changed = true;
                break;
            }
        }
    }
    if (changed)
    {
        predefinedMacros = _Move(newDefs);
        invalidate();
    }
    return changed;
}

void Workspace::init(List<URI> rootDirURI, slang::IGlobalSession* globalSession)
{
    for (auto uri : rootDirURI)
    {
        auto path = uri.getPath();
        rootDirectories.add(path);
        DirEnumerationContext context;
        context.workList.add(path);
        context.root = path;
        auto fileSystem = Slang::OSFileSystem::getExtSingleton();
        for (int i = 0; i < context.workList.getCount(); i++)
        {
            context.currentPath = context.workList[i];
            fileSystem->enumeratePathContents(
                context.currentPath.getBuffer(),
                [](SlangPathType pathType, const char* name, void* userData)
                {
                    auto dirContext = (DirEnumerationContext*)userData;
                    auto nameSlice = UnownedStringSlice(name);
                    if (pathType == SLANG_PATH_TYPE_DIRECTORY)
                    {
                        dirContext->workList.add(Path::combine(dirContext->currentPath, name));
                    }
                    else if (nameSlice.endsWithCaseInsensitive(".slang") || nameSlice.endsWithCaseInsensitive(".hlsl"))
                    {
                        dirContext->addSearchPath(dirContext->currentPath);
                    }
                },
                &context);
        }
        searchPaths = _Move(context.paths);
    }
    slangGlobalSession = globalSession;
}

void Workspace::invalidate() { currentVersion = nullptr; }

void parseDiagnostics(Dictionary<String, DocumentDiagnostics>& diagnostics, String compilerOutput)
{
    List<UnownedStringSlice> lines;
    StringUtil::calcLines(compilerOutput.getUnownedSlice(), lines);
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
        auto& diagnosticList = diagnostics.GetOrAddValue(fileName, DocumentDiagnostics());

        LanguageServerProtocol::Diagnostic diagnostic;
        Index pos = lparentIndex + 1;
        int lineLoc = StringUtil::parseIntAndAdvancePos(line, pos);
        if (lineLoc == 0)
            lineLoc = 1;
        diagnostic.range.end.line = diagnostic.range.start.line = lineLoc - 1;
        pos++;
        int colLoc = StringUtil::parseIntAndAdvancePos(line, pos);
        if (colLoc == 0)
            colLoc = 1;
        diagnostic.range.end.character = diagnostic.range.start.character = colLoc - 1;
        if (pos >= line.getLength())
            continue;
        line = line.tail(colonIndex + 3);
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
        diagnostic.code = StringUtil::parseIntAndAdvancePos(line, pos);
        diagnostic.message = line.tail(colonIndex + 2);
        if (lineIndex + 1 < lines.getCount() && lines[lineIndex].startsWith("^+"))
        {
            lineIndex++;
            pos = 2;
            auto tokenLength = StringUtil::parseIntAndAdvancePos(line, pos);
            diagnostic.range.end.character += tokenLength;
        }
        diagnosticList.messages.Add(diagnostic);
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

    desc.preprocessorMacroCount = predefinedMacros.getCount();
    List<slang::PreprocessorMacroDesc> macroDescs;
    for (auto& macro : predefinedMacros)
    {
        slang::PreprocessorMacroDesc macroDesc;
        macroDesc.name = macro.name.getBuffer();
        macroDesc.value = macro.value.getBuffer();
        macroDescs.add(macroDesc);
    }
    desc.preprocessorMacros = macroDescs.getBuffer();

    ComPtr<slang::ISession> session;
    slangGlobalSession->createSession(desc, session.writeRef());
    version->linkage = static_cast<Linkage*>(session.get());
    // TODO(yong): module cache does improves performance by 30%. However there are some issues
    // that prevents the deserialization to resolve the imported decls from the correct module.
    // This doesn't lead to crash, but may cause problems. We can enable this when the issues
    // are fixed.
#if 0
    version->linkage->setModuleCache(&moduleCache);
#endif
    return version;
}

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

void* Workspace::getInterface(const Guid& uuid)
{
    if (uuid == ISlangUnknown::getTypeGuid() || uuid == ISlangFileSystem::getTypeGuid())
    {
        return static_cast<ISlangFileSystem*>(this);
    }
    return nullptr;
}

void DocumentVersion::setText(const String& newText)
{
    text = newText;
    StringUtil::calcLines(text.getUnownedSlice(), lines);
    utf16CharStarts.clear();
}
ArrayView<Index> DocumentVersion::getUTF16Boundaries(Index line)
{
    if (!utf16CharStarts.getCount())
    {
        for (auto slice : lines)
        {
            List<Index> bounds;
            Index index = 0;
            while (index < slice.getLength())
            {
                auto startIndex = index;
                const Char32 codePoint = getUnicodePointFromUTF8(
                    [&]() -> Byte
                    {
                        if (index < slice.getLength())
                            return slice[index++];
                        else
                            return '\0';
                    });
                if (!codePoint)
                    break;
                Char16 buffer[2];
                int count = encodeUnicodePointToUTF16Reversed(codePoint, buffer);
                for (int i = 0; i < count; i++)
                    bounds.add(startIndex);
            }
            bounds.add(slice.getLength());
            utf16CharStarts.add(_Move(bounds));
        }
    }
    return line >= 1 && line <= utf16CharStarts.getCount() ? utf16CharStarts[line - 1].getArrayView()
                                                           : ArrayView<Index>();
}

void DocumentVersion::oneBasedUTF8LocToZeroBasedUTF16Loc(
    Index inLine, Index inCol, Index& outLine, Index& outCol)
{
    Index rsLine = inLine - 1;
    auto line = lines[rsLine];
    auto bounds = getUTF16Boundaries(inLine);
    outLine = rsLine;
    outCol = std::lower_bound(bounds.begin(), bounds.end(), inCol - 1) - bounds.begin();
}

void DocumentVersion::zeroBasedUTF16LocToOneBasedUTF8Loc(
    Index inLine, Index inCol, Index& outLine, Index& outCol)
{
    outLine = inLine + 1;
    auto bounds = getUTF16Boundaries(inLine + 1);
    outCol = inCol >=0 && inCol < bounds.getCount()? bounds[inCol] + 1 : 0;
}

ASTMarkup* WorkspaceVersion::getOrCreateMarkupAST(ModuleDecl* module)
{
    RefPtr<ASTMarkup> astMarkup;
    if (markupASTs.TryGetValue(module, astMarkup))
        return astMarkup.Ptr();
    DiagnosticSink sink;
    astMarkup = new ASTMarkup();
    sink.setSourceManager(linkage->getSourceManager());
    ASTMarkupUtil::extract(module, linkage->getSourceManager(), &sink, astMarkup.Ptr());
    markupASTs[module] = astMarkup;
    return astMarkup.Ptr();
}

Module* WorkspaceVersion::getOrLoadModule(String path)
{
    Module* module;
    if (modules.TryGetValue(path, module))
    {
        return module;
    }
    auto doc = workspace->openedDocuments.TryGetValue(path);
    if (!doc)
        return nullptr;
    ComPtr<ISlangBlob> diagnosticBlob;
    RefPtr<StringBlob> sourceBlob = new StringBlob((*doc)->getText());
    auto parsedModule = linkage->loadModuleFromSource(
        Path::getFileNameWithoutExt(path).getBuffer(),
        path.getBuffer(),
        sourceBlob.Ptr(),
        diagnosticBlob.writeRef());
    if (parsedModule)
    {
        modules[path] = static_cast<Module*>(parsedModule);
    }
    if (diagnosticBlob)
    {
        auto diagnosticString = String((const char*)diagnosticBlob->getBufferPointer());
        parseDiagnostics(diagnostics, diagnosticString);
        auto docDiagnostic = diagnostics.TryGetValue(path);
        if (docDiagnostic)
            docDiagnostic->originalOutput = diagnosticString;
    }
    return static_cast<Module*>(parsedModule);
}

RefPtr<Module> SerializedModuleCache::tryLoadModule(
    Linkage* linkage, String filePath)
{
    Path::getCanonical(filePath, filePath);
    if (List<uint8_t>* rawData = serializedModules.TryGetValue(filePath))
    {
        RefPtr<MemoryStreamBase> memStream =
            new MemoryStreamBase(FileAccess::Read, rawData->getBuffer(), rawData->getCount());
        RiffContainer riffContainer;
        RiffUtil::read(memStream.Ptr(), riffContainer);
        SerialContainerData outData;
        SerialContainerUtil::ReadOptions options;
        options.linkage = linkage;
        options.namePool = linkage->getNamePool();
        options.session = linkage->getSessionImpl();
        options.sharedASTBuilder = linkage->getASTBuilder()->getSharedASTBuilder();
        options.astBuilder = linkage->getASTBuilder();
        DiagnosticSink sink(linkage->getSourceManager(), Lexer::sourceLocationLexer);
        options.sink = &sink;
        options.sourceManager = linkage->getSourceManager();
        SLANG_RETURN_NULL_ON_FAIL(SerialContainerUtil::read(&riffContainer, options, outData));
        if (outData.modules.getCount() == 1)
        {
            RefPtr<Module> module = new Module(linkage, linkage->getASTBuilder());
            auto moduleDecl = as<ModuleDecl>(outData.modules[0].astRootNode);
            if (moduleDecl)
            {
                moduleDecl->module = module.Ptr();
                module->setModuleDecl(moduleDecl);
                return module;
            }
        }
    }
    return nullptr;
}

void SerializedModuleCache::storeModule(
    Linkage* linkage, String filePath, RefPtr<Module> module)
{
    Path::getCanonical(filePath, filePath);
    RiffContainer container;
    SerialContainerUtil::WriteOptions options;
    options.sourceManager = linkage->getSourceManager();
    options.compressionType = SerialCompressionType::None;
    options.optionFlags = SerialOptionFlag::SourceLocation | SerialOptionFlag::ASTModule;
    SerialContainerData data;
    SerialContainerData::Module moduleData;
    moduleData.astBuilder = linkage->getASTBuilder();
    moduleData.astRootNode = module->getModuleDecl();
    moduleData.irModule = nullptr;
    data.modules.add(moduleData);
    SerialContainerUtil::write(data, options, &container);
    RefPtr<OwnedMemoryStream> memStream = new OwnedMemoryStream(FileAccess::Write);
    RiffUtil::write(&container, memStream);
    List<uint8_t> rawData;
    memStream->swapContents(rawData);
    serializedModules[filePath] = _Move(rawData);
}

} // namespace Slang
