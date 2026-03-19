#include "slang-workspace-version.h"

#include "../compiler-core/slang-lexer.h"
#include "../core/slang-file-system.h"
#include "../core/slang-io.h"
#include "slang-check-impl.h"
#include "slang-mangle.h"
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
            if (!paths.add(canonicalPath))
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
    doc->setPath(path);
    openedDocuments[path] = doc;
    workspaceSearchPaths.add(Path::getParentDirectory(path));
    invalidate();
    return doc.Ptr();
}

void Workspace::changeDoc(
    const String& path,
    LanguageServerProtocol::Range range,
    const String& text)
{
    RefPtr<DocumentVersion> doc;
    if (openedDocuments.tryGetValue(path, doc))
    {
        Index line, col;
        doc->zeroBasedUTF16LocToOneBasedUTF8Loc(range.start.line, range.start.character, line, col);
        auto startOffset = doc->getOffset(line, col);
        doc->zeroBasedUTF16LocToOneBasedUTF8Loc(range.end.line, range.end.character, line, col);
        auto endOffset = doc->getOffset(line, col);
        auto originalText = doc->getText().getUnownedSlice();
        StringBuilder newText;
        if (startOffset != -1)
            newText << originalText.head(startOffset);
        newText << text;
        if (endOffset != -1)
            newText << originalText.tail(endOffset);
        changeDoc(doc.Ptr(), newText.produceString());
    }
}

void Workspace::changeDoc(DocumentVersion* doc, const String& newText)
{
    doc->setText(newText);
    invalidate();
}

void Workspace::closeDoc(const String& path)
{
    openedDocuments.remove(path);
    invalidate();
}

bool Workspace::updatePredefinedMacros(List<String> macros)
{
    List<OwnedPreprocessorMacroDefinition> newDefs;
    for (auto macro : macros)
    {
        auto index = macro.indexOf('=');
        OwnedPreprocessorMacroDefinition def;
        if (index != -1)
        {
            def.name = macro.getUnownedSlice().head(index).trim();
            def.value = macro.getUnownedSlice().tail(index + 1).trim();
        }
        else
        {
            def.name = macro.trim();
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

bool Workspace::updateSearchPaths(List<String> paths)
{
    bool changed = false;
    if (paths.getCount() != additionalSearchPaths.getCount())
        changed = true;
    else
    {
        for (Index i = 0; i < paths.getCount(); i++)
        {
            if (paths[i] != additionalSearchPaths[i])
            {
                changed = true;
                break;
            }
        }
    }
    if (changed)
    {
        additionalSearchPaths = _Move(paths);
        invalidate();
    }
    return changed;
}

bool Workspace::updateSearchInWorkspace(bool value)
{
    bool changed = searchInWorkspace != value;
    searchInWorkspace = value;
    if (changed)
    {
        invalidate();
    }
    return changed;
}

void Workspace::init(List<URI> rootDirURI, slang::IGlobalSession* globalSession)
{
    for (auto uri : rootDirURI)
    {
        auto path = uri.getPath();
        Path::getCanonical(path, path);
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
                        // Ignore directories starting with '.'
                        if (nameSlice.getLength() && nameSlice[0] == '.')
                            return;
                        dirContext->workList.add(Path::combine(dirContext->currentPath, name));
                    }
                    else if (
                        nameSlice.endsWithCaseInsensitive(".slang") ||
                        nameSlice.endsWithCaseInsensitive(".hlsl"))
                    {
                        dirContext->addSearchPath(dirContext->currentPath);
                    }
                },
                &context);
        }
        workspaceSearchPaths = _Move(context.paths);
    }
    slangGlobalSession = globalSession;
}

void Workspace::invalidate()
{
    currentVersion = nullptr;
}

void WorkspaceVersion::parseDiagnostics(String compilerOutput)
{
    // ===================================================================================
    // Machine-Readable Diagnostic Format Parser
    // ===================================================================================
    //
    // The compiler emits diagnostics in a tab-separated machine-readable format:
    //   E<code>\t<severity>\t<filename>\t<beginline>\t<begincol>\t<endline>\t<endcol>\t<message>
    //
    // Each diagnostic produces multiple lines grouped by error code:
    //
    //   1. Primary line (severity = "error", "warning", or "fatal"):
    //      The main diagnostic with the technical error message.
    //
    //   2. Primary span line (severity = "span", immediately follows primary):
    //      Points to the specific code location with a more descriptive message.
    //
    //   3. Secondary span lines (severity = "span", same code):
    //      Additional locations relevant to the error, shown as LSP relatedInformation.
    //
    //   4. Note lines (severity = "note"):
    //      Supplementary information, shown as LSP relatedInformation.
    //
    //   5. Note-span lines (severity = "note-span"):
    //      Location highlights for notes, shown as LSP relatedInformation.
    //
    // -----------------------------------------------------------------------------------
    // Example: For the following rich diagnostic output:
    //
    //   error[E30013]: invalid subscript expression
    //     ╭╼ tests/diagnostics/no-subscript.slang:5:13
    //     │
    //   5 │ return x[0];
    //     │         ━ no subscript declarations found for type 'int'
    //   ──╯
    //
    // The machine-readable format produces:
    //   E30013\terror\ttests/diagnostics/no-subscript.slang\t5\t13\t5\t13\tinvalid subscript
    //   expression E30013\tspan\ttests/diagnostics/no-subscript.slang\t5\t13\t5\t14\tno subscript
    //   declarations found for type 'int'
    //
    // -----------------------------------------------------------------------------------
    // Why we prefer the primary span message over the main diagnostic message:
    // -----------------------------------------------------------------------------------
    //
    // The main diagnostic message is the error title (e.g., "invalid subscript expression").
    // The primary span message describes the specific problem at the code location
    // (e.g., "no subscript declarations found for type 'int'").
    //
    // For LSP, we show the span message because:
    //   - It's more specific and actionable for the user
    //   - It appears directly at the error location in the editor
    //   - It matches what users see under the highlighted code in terminal output
    //
    // We use the primary span's RANGE for precise error highlighting, and fall back
    // to the main diagnostic message only if the span message is empty.
    // ===================================================================================

    List<UnownedStringSlice> lines;
    StringUtil::calcLines(compilerOutput.getUnownedSlice(), lines);

    // Structure to hold parsed diagnostic line info
    struct ParsedLine
    {
        String code;
        String severity;
        String fileName;
        int beginLine;
        int beginCol;
        int endLine;
        int endCol;
        String message;
    };

    // Parse a single line into its components
    auto parseLine = [](UnownedStringSlice line, ParsedLine& out) -> bool
    {
        // Must start with 'E'
        if (!line.startsWith("E"))
            return false;

        List<UnownedStringSlice> fields;
        StringUtil::split(line, '\t', fields);

        // Must have 8 fields: E<code>, severity, filename, beginline, begincol, endline, endcol,
        // message
        if (fields.getCount() < 8)
            return false;

        out.code = fields[0].tail(1); // Skip the 'E' prefix
        out.severity = fields[1];
        out.fileName = fields[2];

        Index pos = 0;
        out.beginLine = StringUtil::parseIntAndAdvancePos(fields[3], pos);
        pos = 0;
        out.beginCol = StringUtil::parseIntAndAdvancePos(fields[4], pos);
        pos = 0;
        out.endLine = StringUtil::parseIntAndAdvancePos(fields[5], pos);
        pos = 0;
        out.endCol = StringUtil::parseIntAndAdvancePos(fields[6], pos);

        out.message = fields[7];

        return true;
    };

    // Helper to convert parsed location to LSP range, handling UTF-16 conversion
    auto convertToLSPRange =
        [this](const ParsedLine& parsed, LanguageServerProtocol::Range& range, String& fileName)
    {
        Path::getCanonical(parsed.fileName, fileName);

        int beginLine = parsed.beginLine > 0 ? parsed.beginLine : 1;
        int beginCol = parsed.beginCol > 0 ? parsed.beginCol : 1;
        int endLine = parsed.endLine > 0 ? parsed.endLine : beginLine;
        int endCol = parsed.endCol > 0 ? parsed.endCol : beginCol;

        range.start.line = beginLine;
        range.start.character = beginCol;
        range.end.line = endLine;
        range.end.character = endCol;

        if (auto doc = workspace->openedDocuments.tryGetValue(fileName))
        {
            // If the file is open, translate to UTF16 positions using the document.
            Index lineUTF16, colUTF16;
            doc->Ptr()->oneBasedUTF8LocToZeroBasedUTF16Loc(
                range.start.line,
                range.start.character,
                lineUTF16,
                colUTF16);
            range.start.line = (int)lineUTF16;
            range.start.character = (int)colUTF16;
            doc->Ptr()->oneBasedUTF8LocToZeroBasedUTF16Loc(
                range.end.line,
                range.end.character,
                lineUTF16,
                colUTF16);
            range.end.line = (int)lineUTF16;
            range.end.character = (int)colUTF16;
        }
        else
        {
            // Otherwise, just return an 0-based position.
            range.start.line--;
            range.start.character--;
            range.end.line--;
            range.end.character--;
        }
    };

    // Helper to check if severity represents a primary diagnostic.
    // Possible values from getSeverityName(): "error", "warning", "fatal error",
    // "internal error", "note", "ignored", "unknown error"
    auto isPrimarySeverity = [](const String& severity) -> bool
    {
        // Primary diagnostics are errors or warnings (not notes/spans)
        return severity == "error" || severity == "warning" || severity == "fatal error" ||
               severity == "internal error" || severity == "unknown error";
    };

    // Helper to check if severity is an error (not warning)
    auto isErrorSeverity = [](const String& severity) -> bool
    {
        return severity == "error" || severity == "fatal error" || severity == "internal error" ||
               severity == "unknown error";
    };

    // Helper to add a span/note as related information
    auto addRelatedInfo =
        [&](LanguageServerProtocol::Diagnostic& diag, const ParsedLine& line, const String& message)
    {
        if (message.getLength() == 0)
            return; // Skip spans with no message

        LanguageServerProtocol::DiagnosticRelatedInformation relatedInfo;
        String relatedFileName;
        convertToLSPRange(line, relatedInfo.location.range, relatedFileName);
        relatedInfo.location.uri = URI::fromLocalFilePath(relatedFileName.getUnownedSlice()).uri;
        relatedInfo.message = message;
        diag.relatedInformation.add(relatedInfo);
    };

    // Process lines - group by diagnostic code
    Index lineIndex = 0;
    while (lineIndex < lines.getCount())
    {
        ParsedLine primaryLine;
        if (!parseLine(lines[lineIndex], primaryLine))
        {
            lineIndex++;
            continue;
        }

        // Check if this is a primary diagnostic
        if (!isPrimarySeverity(primaryLine.severity))
        {
            lineIndex++;
            continue;
        }

        String currentCode = primaryLine.code;

        // Build the LSP diagnostic
        LanguageServerProtocol::Diagnostic diagnostic;

        // Set severity
        if (isErrorSeverity(primaryLine.severity))
            diagnostic.severity = LanguageServerProtocol::kDiagnosticsSeverityError;
        else
            diagnostic.severity = LanguageServerProtocol::kDiagnosticsSeverityWarning;

        // Parse the error code number
        Index codePos = 0;
        diagnostic.code = StringUtil::parseIntAndAdvancePos(currentCode.getUnownedSlice(), codePos);

        // Default message and range from primary line
        String primaryMessage = primaryLine.message;
        LanguageServerProtocol::Range primaryRange;
        String fileName;
        convertToLSPRange(primaryLine, primaryRange, fileName);

        diagnostic.message = primaryMessage;
        diagnostic.range = primaryRange;

        lineIndex++;

        // Look for the primary span line (immediately follows, same code, severity = "span")
        if (lineIndex < lines.getCount())
        {
            ParsedLine spanLine;
            if (parseLine(lines[lineIndex], spanLine) && spanLine.code == currentCode &&
                spanLine.severity == "span")
            {
                // Use span's range
                String spanFileName;
                convertToLSPRange(spanLine, diagnostic.range, spanFileName);

                // Use span's message if non-empty, otherwise keep the primary message
                if (spanLine.message.getLength() > 0)
                {
                    diagnostic.message = spanLine.message;
                }
                lineIndex++;
            }
        }

        // Collect related information from secondary spans and notes (same code)
        while (lineIndex < lines.getCount())
        {
            ParsedLine relatedLine;
            if (!parseLine(lines[lineIndex], relatedLine) || relatedLine.code != currentCode)
            {
                break;
            }

            if (relatedLine.severity == "span" || relatedLine.severity == "note" ||
                relatedLine.severity == "note-span")
            {
                // Secondary spans, notes, and note-spans - add as related info if they have a
                // message
                addRelatedInfo(diagnostic, relatedLine, relatedLine.message);
            }

            lineIndex++;
        }

        // Add the diagnostic to the appropriate file's list
        auto& diagnosticList = diagnostics.getOrAddValue(fileName, DocumentDiagnostics());
        diagnosticList.messages.add(diagnostic);

        if (diagnosticList.messages.getCount() >= 1000)
            break;
    }
}

RefPtr<WorkspaceVersion> Workspace::createWorkspaceVersion()
{
    RefPtr<WorkspaceVersion> version = new WorkspaceVersion();
    version->workspace = this;
    slang::SessionDesc desc = {};
    desc.fileSystem = this;
    desc.targetCount = 1;
    slang::TargetDesc targetDesc = {};
    targetDesc.profile = slangGlobalSession->findProfile("sm_6_6");
    desc.targets = &targetDesc;
    List<const char*> searchPathsRaw;
    for (auto& path : additionalSearchPaths)
        searchPathsRaw.add(path.getBuffer());
    if (searchInWorkspace)
    {
        for (auto& path : workspaceSearchPaths)
            searchPathsRaw.add(path.getBuffer());
    }
    else
    {
        HashSet<String> set;
        for (const auto& [docPath, _] : openedDocuments)
        {
            auto dir = Path::getParentDirectory(docPath.getBuffer());
            if (set.add(dir))
                searchPathsRaw.add(dir.getBuffer());
        }
    }
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

    ComPtr<Linkage> linkage;
    session->queryInterface(Linkage::getTypeGuid(), (void**)linkage.writeRef());
    version->linkage = linkage.get();
    version->linkage->contentAssistInfo.checkingMode = ContentAssistCheckingMode::General;
    return version;
}

SlangResult Workspace::loadFile(const char* path, ISlangBlob** outBlob)
{
    String canonnicalPath;
    SLANG_RETURN_ON_FAIL(Path::getCanonical(path, canonnicalPath));
    RefPtr<DocumentVersion> doc;
    if (openedDocuments.tryGetValue(canonnicalPath, doc))
    {
        *outBlob = StringBlob::create(doc->getText()).detach();
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
WorkspaceVersion* Workspace::createVersionForCompletion()
{
    currentCompletionVersion = createWorkspaceVersion();
    currentCompletionVersion->linkage->contentAssistInfo.checkingMode =
        ContentAssistCheckingMode::Completion;
    return currentCompletionVersion.Ptr();
}

void* Workspace::getObject(const Guid& uuid)
{
    SLANG_UNUSED(uuid);
    return nullptr;
}

void* Workspace::getInterface(const Guid& uuid)
{
    if (uuid == ISlangUnknown::getTypeGuid() || uuid == ISlangCastable::getTypeGuid() ||
        uuid == ISlangFileSystem::getTypeGuid())
    {
        return static_cast<ISlangFileSystem*>(this);
    }
    return nullptr;
}

void* Workspace::castAs(const Guid& guid)
{
    if (auto ptr = getInterface(guid))
    {
        return ptr;
    }
    return getObject(guid);
}

void DocumentVersion::setText(const String& newText)
{
    text = newText;
    StringUtil::calcLines(text.getUnownedSlice(), lines);
    mapUTF16CharIndexToCodePointIndex.clear();
    mapCodePointIndexToUTF8ByteOffset.clear();
}

void DocumentVersion::ensureUTFBoundsAvailable()
{
    for (auto slice : lines)
    {
        List<Index> bounds;
        List<Index> utf8Bounds;
        Index index = 0;
        Index codePointIndex = 0;
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
                bounds.add(codePointIndex);
            utf8Bounds.add(startIndex);
            codePointIndex++;
        }
        bounds.add(slice.getLength());
        utf8Bounds.add(slice.getLength());
        mapUTF16CharIndexToCodePointIndex.add(_Move(bounds));
        mapCodePointIndexToUTF8ByteOffset.add(_Move(utf8Bounds));
    }
}

ArrayView<Index> DocumentVersion::getUTF16Boundaries(Index line)
{
    if (!mapUTF16CharIndexToCodePointIndex.getCount())
    {
        ensureUTFBoundsAvailable();
    }
    return line >= 1 && line <= mapUTF16CharIndexToCodePointIndex.getCount()
               ? mapUTF16CharIndexToCodePointIndex[line - 1].getArrayView()
               : ArrayView<Index>();
}

ArrayView<Index> DocumentVersion::getUTF8Boundaries(Index line)
{
    if (!mapCodePointIndexToUTF8ByteOffset.getCount())
    {
        ensureUTFBoundsAvailable();
    }
    return line >= 1 && line <= mapCodePointIndexToUTF8ByteOffset.getCount()
               ? mapCodePointIndexToUTF8ByteOffset[line - 1].getArrayView()
               : ArrayView<Index>();
}

void DocumentVersion::oneBasedUTF8LocToZeroBasedUTF16Loc(
    Index inLine,
    Index inCol,
    int64_t& outLine,
    int64_t& outCol)
{
    if (inLine <= 0)
    {
        outLine = 0;
        outCol = 0;
    }

    Index rsLine = inLine - 1;
    auto bounds = getUTF16Boundaries(inLine);
    outLine = rsLine;
    if (bounds.getCount() != 0)
        outCol = std::lower_bound(bounds.begin(), bounds.end(), inCol - 1) - bounds.begin();
    else
        outCol = inCol - 1;
}

void DocumentVersion::oneBasedUTF8LocToZeroBasedUTF16Loc(
    Index inLine,
    Index inCol,
    int32_t& outLine,
    int32_t& outCol)
{
    int64_t ioutLine, ioutCol;
    oneBasedUTF8LocToZeroBasedUTF16Loc(inLine, inCol, ioutLine, ioutCol);
    outLine = (int32_t)ioutLine;
    outCol = (int32_t)ioutCol;
}

void DocumentVersion::zeroBasedUTF16LocToOneBasedUTF8Loc(
    Index inLine,
    Index inCol,
    Index& outLine,
    Index& outCol)
{
    outLine = inLine + 1;
    auto bounds = getUTF16Boundaries(inLine + 1);
    outCol = inCol >= 0 && inCol < bounds.getCount() ? bounds[inCol] + 1 : 0;
}

static bool _isIdentifierChar(char ch)
{
    return ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z' || ch >= '0' && ch <= '9' || ch == '_';
}

UnownedStringSlice DocumentVersion::peekIdentifier(Index& offset)
{
    Index start = offset;
    Index end = offset;
    if (start >= text.getLength())
        return UnownedStringSlice("");

    while (start >= 0 && _isIdentifierChar(text[start]))
        start--;
    while (end < text.getLength() && _isIdentifierChar(text[end]))
        end++;
    offset = start + 1;
    if (end > offset)
        return text.getUnownedSlice().subString(start + 1, end - start - 1);
    return UnownedStringSlice("");
}

int DocumentVersion::getTokenLength(Index offset)
{
    if (offset >= 0)
    {
        Index pos = offset;
        for (; pos < text.getLength() && _isIdentifierChar(text[pos]); ++pos)
        {
        }
        return (int)(pos - offset);
    }
    return 0;
}

int DocumentVersion::getTokenLength(Index line, Index col)
{
    auto offset = getOffset(line, col);
    return getTokenLength(offset);
}

ASTMarkup* WorkspaceVersion::getOrCreateMarkupAST(ModuleDecl* module)
{
    RefPtr<ASTMarkup> astMarkup;
    if (markupASTs.tryGetValue(module, astMarkup))
        return astMarkup.Ptr();
    DiagnosticSink sink;
    astMarkup = new ASTMarkup();
    sink.setSourceManager(linkage->getSourceManager());
    ASTMarkupUtil::extract(module, linkage->getSourceManager(), &sink, astMarkup.Ptr(), true);
    markupASTs[module] = astMarkup;
    return astMarkup.Ptr();
}

void WorkspaceVersion::ensureWorkspaceFlavor(UnownedStringSlice path)
{
    if (flavor != WorkspaceFlavor::Standard)
        return;

    if (workspace->workspaceFlavor == WorkspaceFlavor::VFX || path.endsWithCaseInsensitive(".vfx"))
    {
        // Setup linkage for vfx files.
        // TODO: consider supporting this as an external config file.
        flavor = WorkspaceFlavor::VFX;
        linkage->m_optionSet.set(CompilerOptionName::EnableEffectAnnotations, true);
        linkage->addPreprocessorDefine("VS", "__file_decl");
        linkage->addPreprocessorDefine("CS", "__file_decl");
        linkage->addPreprocessorDefine("GS", "__file_decl");
        linkage->addPreprocessorDefine("PS", "__file_decl");
        linkage->addPreprocessorDefine("RTX", "__file_decl");
        linkage->addPreprocessorDefine("PS_RTX", "__file_decl");
        linkage->addPreprocessorDefine("VS_RTX", "__file_decl");
        linkage->addPreprocessorDefine("MS", "__file_decl");
        linkage->addPreprocessorDefine("VS_MS_RTX", "__file_decl");
        linkage->addPreprocessorDefine("FRAGMENT_ANNOTATION", "");
        linkage->addPreprocessorDefine("DynamicCombo", "//");
        linkage->addPreprocessorDefine("DynamicComboRule", "//");
        linkage->addPreprocessorDefine("HEADER", "__ignored_block");
        linkage->addPreprocessorDefine("MODES", "__ignored_block");
        linkage->addPreprocessorDefine("PS_RENDER_STATE", "__ignored_block");
        linkage->addPreprocessorDefine("FEATURES", "__ignored_block");
        linkage->addPreprocessorDefine("COMMON", "__transparent_block");
    }
}

Module* WorkspaceVersion::getOrLoadModule(String path)
{
    Module* module;
    if (modules.tryGetValue(path, module))
    {
        return module;
    }
    auto doc = workspace->openedDocuments.tryGetValue(path);
    if (!doc)
        return nullptr;
    ComPtr<ISlangBlob> diagnosticBlob;
    auto sourceBlob = StringBlob::create((*doc)->getText());

    auto moduleName = getMangledNameFromNameString(path.getUnownedSlice());
    linkage->contentAssistInfo.primaryModuleName = linkage->getNamePool()->getName(moduleName);
    linkage->contentAssistInfo.primaryModulePath = path;

    ensureWorkspaceFlavor(path.getUnownedSlice());

    // Note:
    // The module at `path` may have already been loaded into the linkage previously
    // due to an `import`. However that module won't get fully checked in when the checker
    // is in language server mode to speed things up.
    // Therefore, we always call `loadModuleFromSource` to load a fresh module instead of
    // trying to reuse the existing one through `findOrImportModule`, this will result in
    // redundant parsing and storage, but it saves us from the hassle of handling
    // incremental/lazy checking on a previously loaded module.
    auto parsedModule = linkage->loadModuleFromSource(
        moduleName.getBuffer(),
        path.getBuffer(),
        sourceBlob,
        diagnosticBlob.writeRef());
    if (parsedModule)
    {
        modules[path] = static_cast<Module*>(parsedModule);
    }
    if (diagnosticBlob)
    {
        auto diagnosticString = String((const char*)diagnosticBlob->getBufferPointer());
        parseDiagnostics(diagnosticString);
        auto docDiagnostic = diagnostics.tryGetValue(path);
        if (docDiagnostic)
            docDiagnostic->originalOutput = diagnosticString;
    }
    return static_cast<Module*>(parsedModule);
}

MacroDefinitionContentAssistInfo* WorkspaceVersion::tryGetMacroDefinition(UnownedStringSlice name)
{
    if (macroDefinitions.getCount() == 0)
    {
        // build dictionary.
        for (auto& def : linkage->contentAssistInfo.preprocessorInfo.macroDefinitions)
        {
            macroDefinitions[def.name] = &def;
        }
    }
    MacroDefinitionContentAssistInfo* result = nullptr;
    auto namePtr = linkage->getNamePool()->tryGetName(name);
    if (!namePtr)
        return nullptr;
    macroDefinitions.tryGetValue(namePtr, result);
    return result;
}

} // namespace Slang
