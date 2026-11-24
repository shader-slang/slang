import * as monaco from "monaco-editor/esm/vs/editor/editor.api.js";
import "monaco-editor/esm/vs/editor/contrib/suggest/browser/suggestController.js";
import "monaco-editor/esm/vs/editor/contrib/hover/browser/hoverContribution.js";
import "monaco-editor/esm/vs/editor/contrib/parameterHints/browser/parameterHints.js";
import "monaco-editor/esm/vs/editor/contrib/bracketMatching/browser/bracketMatching.js";
import "monaco-editor/esm/vs/editor/contrib/contextmenu/browser/contextmenu.js";
import "monaco-editor/esm/vs/editor/contrib/gotoSymbol/browser/goToCommands.js";
import "monaco-editor/esm/vs/editor/contrib/gotoSymbol/browser/link/goToDefinitionAtPosition.js";
import "monaco-editor/esm/vs/editor/standalone/browser/referenceSearch/standaloneReferenceSearch.js";
import { FoldingController } from "monaco-editor/esm/vs/editor/contrib/folding/browser/folding.js";
import { ICodeEditorService } from "monaco-editor/esm/vs/editor/browser/services/codeEditorService.js";
import { StandaloneServices } from "monaco-editor/esm/vs/editor/standalone/browser/standaloneServices.js";
import { registerSlang } from "./slang-language.js";
import { foldingHeaderLine, registerSlangFolding } from "./slang-folding.js";

self.MonacoEnvironment = {
    getWorker: () => new Worker(new URL("editor.worker.js", import.meta.url), { type: "module" }),
};
registerSlang(monaco);
registerSlangFolding(monaco);
monaco.editor.defineTheme("slang-docs", {
    base: "vs", inherit: true,
    rules: [{ token: "keyword", foreground: "286515" }, { token: "type", foreground: "176879" },
        { token: "comment", foreground: "667566" }, { token: "string", foreground: "9B4526" }],
    colors: { "editor.background": "#F8FAF7", "editorLineNumber.foreground": "#7B897B" },
});

const services = new Map();
const cells = new Map();
const codeEditors = StandaloneServices.get(ICodeEditorService);
let nextModel = 0;
const position = value => ({ line: value.lineNumber - 1, character: value.column - 1 });
const range = value => ({ startLineNumber: value.start.line + 1, startColumn: value.start.character + 1,
    endLineNumber: value.end.line + 1, endColumn: value.end.character + 1 });
const markdown = value => value?.kind === "markdown" ? { value: value.value, isTrusted: false } : value?.value;
const kindNames = ["Text", "Method", "Function", "Constructor", "Field", "Variable", "Class", "Interface",
    "Module", "Property", "Unit", "Value", "Enum", "Keyword", "Snippet", "Color", "File", "Reference",
    "Folder", "EnumMember", "Constant", "Struct", "Event", "Operator", "TypeParameter"];

// Monaco's public registerEditorOpener returns the source editor to its caller.
// Return the actual destination so definition highlights and peek navigation use
// the right cell, without replacing either editor's model or undo history.
codeEditors.registerCodeEditorOpenHandler(async input => {
    const target = cells.get(input.resource.toString());
    return target ? target.reveal(input.options?.selection) : null;
});

async function query(model, method, args, token) {
    const service = services.get(model.uri.toString());
    if (!service || token?.isCancellationRequested) return null;
    const version = model.getVersionId();
    const workspaceVersion = service.workspaceVersion;
    try {
        const value = await service.query(method, { ...args, source: model.getValue() });
        return model.isDisposed() || model.getVersionId() !== version ||
            service.workspaceVersion !== workspaceVersion || token?.isCancellationRequested ? null : value;
    } catch {
        // The service reports failures in its status line; keep editor providers usable.
        return null;
    }
}

monaco.languages.registerCompletionItemProvider("slang", {
    triggerCharacters: [".", ":", ">"],
    async provideCompletionItems(model, pos, context, token) {
        const items = await query(model, "completion", { position: position(pos), context: {
            triggerKind: context.triggerKind + 1, triggerCharacter: context.triggerCharacter || "",
        } }, token);
        const word = model.getWordUntilPosition(pos);
        const wordRange = { startLineNumber: pos.lineNumber, endLineNumber: pos.lineNumber,
            startColumn: word.startColumn, endColumn: word.endColumn };
        return { suggestions: (items || []).map(item => ({
            label: item.label, kind: monaco.languages.CompletionItemKind[kindNames[item.kind - 1]]
                ?? monaco.languages.CompletionItemKind.Text,
            detail: item.detail, documentation: markdown(item.documentation),
            insertText: item.textEdit?.text ?? item.label,
            range: item.textEdit ? range(item.textEdit.range) : wordRange,
            commitCharacters: item.commitCharacters,
        })) };
    },
});
monaco.languages.registerHoverProvider("slang", {
    async provideHover(model, pos, token) {
        const hover = await query(model, "hover", { position: position(pos) }, token);
        if (!hover) return null;
        return { range: range(hover.range), contents: [{ value: hover.contents.value, isTrusted: false }] };
    },
});
monaco.languages.registerDefinitionProvider("slang", {
    async provideDefinition(model, pos, token) {
        const locations = await query(model, "definition", { position: position(pos) }, token);
        return (locations || []).flatMap(location => {
            const uri = monaco.Uri.parse(location.uri);
            // Only page cells have an editor to reveal; standard-library files do not.
            return cells.has(uri.toString()) ? [{ uri, range: range(location.range) }] : [];
        });
    },
});
monaco.languages.registerSignatureHelpProvider("slang", {
    signatureHelpTriggerCharacters: ["(", ","], signatureHelpRetriggerCharacters: [")"],
    async provideSignatureHelp(model, pos, token) {
        const help = await query(model, "signatureHelp", { position: position(pos) }, token);
        if (!help) return null;
        return { dispose() {}, value: { ...help, signatures: help.signatures.map(signature => ({
            ...signature, documentation: markdown(signature.documentation),
            parameters: signature.parameters.map(parameter => ({
                ...parameter, documentation: markdown(parameter.documentation),
            })),
        })) } };
    },
});

class LanguageService {
    pending = new Map();
    nextId = 0;
    worker = null;
    ready = null;
    disposed = false;
    workspaceVersion = 0;
    constructor(model, files, onStatus, getSources, standardModulesURL) {
        this.model = model;
        this.files = files;
        this.onStatus = onStatus;
        this.getSources = getSources;
        this.standardModulesURL = standardModulesURL;
    }
    start() {
        if (this.ready) return this.ready;
        this.onStatus("Loading IntelliSense…");
        this.ready = (async () => {
            this.worker = new Worker(new URL("language.worker.js", import.meta.url), { type: "module" });
            this.worker.onmessage = ({ data }) => {
                const pending = this.pending.get(data.id);
                if (!pending) return;
                this.pending.delete(data.id);
                clearTimeout(pending.timeout);
                if (data.error) pending.reject(new Error(data.error));
                else pending.resolve(data.result);
            };
            this.worker.onerror = event => { event.preventDefault(); this.fail(new Error(event.message || "Language worker failed")); };
            this.worker.onmessageerror = () => this.fail(new Error("Cannot read language worker response"));
            await this.send("initialize", { files: this.files, uri: this.model.uri.toString(),
                source: this.model.getValue(), sources: this.getSources(), standardModulesURL: this.standardModulesURL });
            if (!this.disposed) this.onStatus("IntelliSense ready");
        })().catch(error => { this.fail(error); throw error; });
        return this.ready;
    }
    send(method, args) {
        return new Promise((resolve, reject) => {
            const id = ++this.nextId;
            const timeout = setTimeout(() => this.fail(new Error("Language service timed out")), 90000);
            this.pending.set(id, { resolve, reject, timeout });
            this.worker.postMessage({ id, method, ...args });
        });
    }
    async query(method, args) {
        await this.start();
        if (this.disposed || !this.worker) throw new Error("Language service is unavailable");
        try { return await this.send(method, { ...args, sources: this.getSources() }); }
        catch (error) { this.fail(error); throw error; }
    }
    fail(error) {
        this.worker?.terminate();
        this.worker = null;
        for (const pending of this.pending.values()) {
            clearTimeout(pending.timeout);
            pending.reject(error);
        }
        this.pending.clear();
        if (!this.disposed) this.onStatus(`IntelliSense unavailable: ${error.message}`, true);
    }
    dispose() {
        this.disposed = true;
        this.fail(new Error("Editor closed"));
    }
}

export function createEditor(container, { source, name, filename, files, onRun, onStatus,
    getSources = () => ({}), onChange, onReveal, standardModulesURL, foldLines = [] }) {
    const uri = monaco.Uri.parse(`file:///${filename ?? `snippet-${++nextModel}-${name}.slang`}`);
    const model = monaco.editor.createModel(source, "slang", uri);
    const editor = monaco.editor.create(container, {
        model, theme: "slang-docs", automaticLayout: true, fontSize: 14, lineHeight: 23,
        fontFamily: 'Consolas, "Liberation Mono", monospace', tabSize: 4,
        minimap: { enabled: false }, folding: true, showFoldingControls: "mouseover", glyphMargin: false,
        lineNumbersMinChars: 3, lineDecorationsWidth: 8, scrollBeyondLastLine: false,
        overviewRulerLanes: 0, hideCursorInOverviewRuler: true, stickyScroll: { enabled: false },
        renderLineHighlight: "none", padding: { top: 12, bottom: 12 },
        wordBasedSuggestions: "off", fixedOverflowWidgets: true,
        ariaLabel: `Slang source: ${name}`, scrollbar: { alwaysConsumeMouseWheel: false },
    });
    // The editor fits its content, so short modules stay short; long programs scroll past 600px.
    const resize = () => { container.style.height = `${Math.min(600, editor.getContentHeight())}px`; };
    resize();
    const sizeListener = editor.onDidContentSizeChange(resize);
    async function foldDefaults(lines) {
        if (!lines.length) return;
        const version = model.getVersionId();
        try {
            // Wait for Monaco's own folding ranges, without starting the language service.
            const folding = await FoldingController.get(editor)?.getFoldingModel();
            if (!folding || model.isDisposed() || model.getVersionId() !== version) return;
            const regions = [];
            for (let line of lines) {
                if (line < 1 || line > model.getLineCount()) continue;
                line = foldingHeaderLine(model, line);
                const region = folding.getRegionAtLine(line);
                // Ignore markers without their own region; never collapse an enclosing block.
                if (region?.startLineNumber === line && !region.isCollapsed) regions.push(region);
            }
            folding.toggleCollapseState(regions);
        } catch (error) {
            if (!model.isDisposed()) console.warn("Could not apply snippet default folds", error);
        }
    }
    void foldDefaults(foldLines);
    const navigation = {
        async reveal(selection, isCurrent = () => true) {
            await onReveal?.();
            if (model.isDisposed() || !isCurrent()) return null;
            const target = model.validateRange(new monaco.Range(
                selection?.startLineNumber ?? 1, selection?.startColumn ?? 1,
                selection?.endLineNumber ?? selection?.startLineNumber ?? 1,
                selection?.endColumn ?? selection?.startColumn ?? 1));
            const folding = await FoldingController.get(editor)?.getFoldingModel();
            if (model.isDisposed() || !isCurrent()) return null;
            // Reveal every enclosing fold immediately, before measuring and scrolling.
            if (folding) folding.toggleCollapseState(folding.getAllRegionsAtLine(
                target.startLineNumber, region => region.isCollapsed && target.startLineNumber > region.startLineNumber));
            resize();
            editor.layout();
            editor.setSelection(target);
            editor.revealRangeInCenter(target, monaco.editor.ScrollType.Immediate);
            container.scrollIntoView({ block: "center" });
            editor.focus();
            return editor;
        },
    };
    cells.set(uri.toString(), navigation);
    // Actions scope keybindings to this editor; an unscoped command runs the last snippet.
    const runAction = editor.addAction({
        id: "run-slang-example", label: "Run example",
        keybindings: [monaco.KeyMod.CtrlCmd | monaco.KeyCode.Enter],
        keybindingContext: "editorTextFocus", run: onRun,
    });
    const service = new LanguageService(model, files, onStatus, getSources, standardModulesURL);
    services.set(uri.toString(), service);
    let timer;
    async function diagnose() {
        const diagnostics = await query(model, "diagnostics", {});
        if (diagnostics === null || model.isDisposed()) return;
        monaco.editor.setModelMarkers(model, "slang", diagnostics.map(item => ({
            ...range(item.range), message: item.message, code: item.code,
            severity: [monaco.MarkerSeverity.Error, monaco.MarkerSeverity.Warning,
                monaco.MarkerSeverity.Info, monaco.MarkerSeverity.Hint][item.severity - 1] ?? monaco.MarkerSeverity.Info,
        })));
    }
    // A run reports diagnostics from the compilers that built it, which can find errors the
    // language service does not, such as a kernel that fails to compile for the GPU. Each run owns
    // its markers, and they describe the code that ran, so any edit clears them.
    const runOwners = new Set();
    function clearRunMarkers() {
        for (const owner of runOwners) monaco.editor.setModelMarkers(model, owner, []);
        runOwners.clear();
    }
    function setRunMarkers(owner, diagnostics) {
        runOwners.add(owner);
        monaco.editor.setModelMarkers(model, owner, diagnostics.flatMap(item => {
            if (item.line < 1 || item.line > model.getLineCount()) return [];
            // A diagnostic names one column; the marker covers the word there, or the line's
            // code when the column is past its end.
            const maxColumn = model.getLineMaxColumn(item.line);
            const word = item.column < maxColumn &&
                model.getWordAtPosition({ lineNumber: item.line, column: item.column });
            const startColumn = word ? word.startColumn :
                item.column < maxColumn ? item.column : model.getLineFirstNonWhitespaceColumn(item.line) || 1;
            const endColumn = word ? word.endColumn :
                item.column < maxColumn ? item.column + 1 : maxColumn;
            return [{
                startLineNumber: item.line, startColumn, endLineNumber: item.line, endColumn,
                message: item.message, code: item.code, source: "Run",
                severity: item.severity === "warning" ? monaco.MarkerSeverity.Warning : monaco.MarkerSeverity.Error,
            }];
        }));
    }
    const focusListener = editor.onDidFocusEditorText(() => {
        if (!service.ready) void diagnose();
    });
    const changeListener = model.onDidChangeContent(() => {
        clearRunMarkers();
        onChange?.();
        clearTimeout(timer);
        if (service.ready) timer = setTimeout(diagnose, 300);
    });
    return {
        getValue: () => model.getValue(),
        reveal: navigation.reveal,
        async findSymbols(name) {
            const symbols = await query(model, "symbols", {});
            if (symbols === null) return null;
            const matches = [];
            const visit = (symbols, parent = "") => {
                for (const symbol of symbols) {
                    const qualified = parent ? `${parent}.${symbol.name}` : symbol.name;
                    if (qualified === name) matches.push({
                        name: qualified,
                        detail: symbol.detail || model.getLineContent(symbol.range.start.line + 1).trim(),
                        range: range(symbol.selectionRange),
                    });
                    visit(symbol.children, qualified);
                }
            };
            visit(symbols);
            return matches;
        },
        setValue(value, foldLines = []) {
            model.setValue(value);
            void foldDefaults(foldLines);
        },
        setReadOnly: readOnly => editor.updateOptions({ readOnly }),
        setRunMarkers,
        sourcesChanged() {
            service.workspaceVersion++;
            clearTimeout(timer);
            if (service.ready) timer = setTimeout(diagnose, 300);
        },
        dispose() {
            clearTimeout(timer);
            service.dispose();
            services.delete(uri.toString());
            cells.delete(uri.toString());
            focusListener.dispose(); changeListener.dispose(); sizeListener.dispose(); runAction.dispose();
            editor.dispose(); model.dispose();
        },
    };
}
