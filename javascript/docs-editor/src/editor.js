import * as monaco from "monaco-editor/esm/vs/editor/editor.api.js";
import "monaco-editor/esm/vs/editor/contrib/suggest/browser/suggestController.js";
import "monaco-editor/esm/vs/editor/contrib/hover/browser/hoverContribution.js";
import "monaco-editor/esm/vs/editor/contrib/parameterHints/browser/parameterHints.js";
import "monaco-editor/esm/vs/editor/contrib/bracketMatching/browser/bracketMatching.js";
import { FoldingController } from "monaco-editor/esm/vs/editor/contrib/folding/browser/folding.js";
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
let nextModel = 0;
const position = value => ({ line: value.lineNumber - 1, character: value.column - 1 });
const range = value => ({ startLineNumber: value.start.line + 1, startColumn: value.start.character + 1,
    endLineNumber: value.end.line + 1, endColumn: value.end.character + 1 });
const markdown = value => value?.kind === "markdown" ? { value: value.value, isTrusted: false } : value?.value;
const kindNames = ["Text", "Method", "Function", "Constructor", "Field", "Variable", "Class", "Interface",
    "Module", "Property", "Unit", "Value", "Enum", "Keyword", "Snippet", "Color", "File", "Reference",
    "Folder", "EnumMember", "Constant", "Struct", "Event", "Operator", "TypeParameter"];

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
    getSources = () => ({}), onChange, standardModulesURL, foldLines = [] }) {
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
    const resize = () => { container.style.height = `${Math.min(600, Math.max(180, editor.getContentHeight()))}px`; };
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
    const focusListener = editor.onDidFocusEditorText(() => {
        if (!service.ready) void diagnose();
    });
    const changeListener = model.onDidChangeContent(() => {
        onChange?.();
        clearTimeout(timer);
        if (service.ready) timer = setTimeout(diagnose, 300);
    });
    return {
        getValue: () => model.getValue(),
        setValue(value, foldLines = []) {
            model.setValue(value);
            void foldDefaults(foldLines);
        },
        setReadOnly: readOnly => editor.updateOptions({ readOnly }),
        sourcesChanged() {
            service.workspaceVersion++;
            clearTimeout(timer);
            if (service.ready) timer = setTimeout(diagnose, 300);
        },
        dispose() {
            clearTimeout(timer);
            service.dispose();
            services.delete(uri.toString());
            focusListener.dispose(); changeListener.dispose(); sizeListener.dispose(); runAction.dispose();
            editor.dispose(); model.dispose();
        },
    };
}
