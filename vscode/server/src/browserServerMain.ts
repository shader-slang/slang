import { createConnection, BrowserMessageReader, BrowserMessageWriter } from 'vscode-languageserver/browser';

import { InitializeParams, InitializeResult, TextDocuments, TextDocumentSyncKind, MarkupKind, DocumentSymbol, Location, SignatureHelp, CompletionItemKind, CompletionItem, SignatureInformation, ParameterInformation, TextDocumentContentChangeEvent, Diagnostic, SymbolKind } from 'vscode-languageserver';
import { TextDocument } from 'vscode-languageserver-textdocument';

import createModule from '../../media/slang-wasm.worker.js';
import type { LanguageServer, MainModule, CompletionContext, DocumentSymbol as WasmDocumentSymbol } from '../../media/slang-wasm.worker';
import spirvTools from '../../media/spirv-tools.worker.js';
import type { CompiledPlayground, EntrypointsResult, Result, ServerInitializationOptions, Shader, WorkerRequest } from 'slang-playground-shared';
import { DiagnosticSeverity } from 'vscode-languageserver';
import { PLAYGROUND_SOURCE, SlangCompiler, compilePlayground } from 'slang-compilation-engine';
import { modifyEmscriptenFile, getEmscriptenURI, getSlangdURI, removePrefix } from './lspSharedUtils.js';

// We'll set these after dynamic import
let slangd: LanguageServer;
let compiler: SlangCompiler;
let slangWasmModule: MainModule;

function convertDocumentSymbol(sym: WasmDocumentSymbol): DocumentSymbol {
    let children: DocumentSymbol[] = [];
    if (sym.children && typeof sym.children.size === 'function' && typeof sym.children.get === 'function') {
        for (let j = 0; j < sym.children.size(); j++) {
            const child = sym.children.get(j);
            if (child) children.push(convertDocumentSymbol(child));
        }
    } else if (Array.isArray(sym.children)) {
        children = sym.children.map(convertDocumentSymbol);
    }
    return {
        name: sym.name.toString(),
        detail: sym.detail.toString(),
        kind: sym.kind as SymbolKind,
        range: sym.range,
        selectionRange: sym.selectionRange,
        children,
    };
}


const messageReader = new BrowserMessageReader(self);
const messageWriter = new BrowserMessageWriter(self);
const connection = createConnection(messageReader, messageWriter);

// // Dynamically import the WASM module and set up the language server
let initializationOptions: ServerInitializationOptions;

function loadFileIntoEmscriptenFS(uri: string, content: string) {
    // Ensure directory exists
    const splitPath = uri.split("/")
    splitPath.pop()
    const dir = splitPath.join("/");
    let pathData = slangWasmModule.FS.analyzePath(uri, false);
    if (!pathData.parentExists) {
        slangWasmModule.FS.createPath('/', dir, true, true);
    }

    // Write the actual file
    slangWasmModule.FS.writeFile(uri, content);
}

let moduleReady: Promise<void> | null = null;
async function ensureSlangModuleLoaded() {
    if (slangd) return;
    if (moduleReady) return moduleReady;
    moduleReady = (async () => {
        // Instantiate the WASM module and create the language server
        slangWasmModule = await createModule();
        compiler = new SlangCompiler(slangWasmModule);
        let result = compiler.init();
        if (!result.succ) {
            console.error(`Failed to initialize compiler: ${result.message}`)
        }
        slangd = slangWasmModule.createLanguageServer()!;
    })();
    return moduleReady;
}

/* from here on, all code is non-browser specific and could be shared with a regular extension */

connection.onInitialize(async (_params: InitializeParams): Promise<InitializeResult> => {
    // Accept extensionUri from initializationOptions
    if (_params.initializationOptions) {
        initializationOptions = _params.initializationOptions;
    }
    try {
        await ensureSlangModuleLoaded();
    } catch (err) {
        console.error('Failed to load slang-wasm:', err);
    }

    for (const file of initializationOptions.files) {
        const emscriptenURI = getEmscriptenURI(file.uri, initializationOptions.workspaceUris);
        loadFileIntoEmscriptenFS(emscriptenURI, file.content);
    }

    return {
        capabilities: {
            textDocumentSync: TextDocumentSyncKind.Incremental,
            completionProvider: { triggerCharacters: [".", ":", ">", "(", "<", " ", "["] },
            hoverProvider: true,
            definitionProvider: true,
            signatureHelpProvider: { triggerCharacters: ['(', ','] },
            documentSymbolProvider: true,
        },
    };
});

// Track open, change and close text document events
const documents = new TextDocuments(TextDocument);
documents.listen(connection);

// --- LSP Handlers ---

// Completion
connection.onCompletion(async (params, _token, _progress): Promise<CompletionItem[]> => {
    const wasmURI = getSlangdURI(params.textDocument.uri, initializationOptions.workspaceUris);
    let lspContext: CompletionContext = {
        triggerKind: params.context!.triggerKind,
        triggerCharacter: params.context?.hasOwnProperty("triggerCharacter") ? (params.context?.triggerCharacter || "") : ""
    };
    const result = slangd.completion(wasmURI, params.position, lspContext);
    if (result == undefined) {
        return [];
    }
    const items: CompletionItem[] = [];
    for (let i = 0; i < result.size(); i++) {
        const item = result.get(i);
        if (!item) continue;
        // Only use LSP fields
        items.push({
            label: item.label.toString(),
            kind: item.kind as CompletionItemKind,
            detail: item.detail.toString(),
            documentation: item.documentation?.toString() ?? '',
            data: item.data,
            insertText: item.label.toString(),
        });
    }
    return items;
});

// Hover
connection.onHover(async (params, _token) => {
    const wasmURI = getSlangdURI(params.textDocument.uri, initializationOptions.workspaceUris);
    const result = slangd.hover(wasmURI, params.position);
    if (!result) return null;
    return {
        contents: {
            kind: MarkupKind.Markdown,
            value: result.contents.value.toString()
        },
        range: result.range
    };
});

// Definition
connection.onDefinition(async (params, _token) => {
    // Get the protocol from the textDocument uri
    const uri = params.textDocument.uri;
    const protocolMatch = uri.match(/^([a-zA-Z\-]+:\/\/\/?)/);
    const protocol = protocolMatch ? protocolMatch[1] : '';
    const wasmURI = getSlangdURI(uri, initializationOptions.workspaceUris);

    const result = slangd.gotoDefinition(wasmURI, params.position);
    if (!result) return null;

    const arr: Location[] = [];
    for (let i = 0; i < result.size(); i++) {
        let loc = result.get(i);
        if (!loc) throw new Error("Invalid state")

        let vscodeURI = loc.uri.toString();
        vscodeURI = removePrefix(vscodeURI, "file:///");
        vscodeURI = `${protocol}${vscodeURI}`;
        arr.push({
            ...loc,
            uri: vscodeURI,
        });
    }
    return arr;
});

// Signature Help
connection.onSignatureHelp(async (params, _token): Promise<SignatureHelp | null> => {
    const wasmURI = getSlangdURI(params.textDocument.uri, initializationOptions.workspaceUris);
    const result = slangd.signatureHelp(wasmURI, params.position);
    if (!result) return null;

    const sigs: SignatureInformation[] = [];
    for (let i = 0; i < result.signatures.size(); i++) {
        let lspSignature = result.signatures.get(i);
        if (lspSignature == undefined) {
            throw new Error("Invalid state!");
        }
        let params: ParameterInformation[] = [];
        for (let j = 0; j < lspSignature.parameters.size(); j++) {
            let lspParameter = lspSignature.parameters.get(j);
            if (lspParameter == undefined) {
                throw new Error("Invalid state!");
            }
            params.push({
                label: [lspParameter.label[0], lspParameter.label[1]],
                documentation: lspParameter.documentation.value.toString()
            });
        }
        let signature: SignatureInformation = {
            label: lspSignature.label.toString(),
            documentation: lspSignature.documentation.value.toString(),
            parameters: params
        };
        sigs.push(signature);
    }
    return { ...result, signatures: sigs };
});

// Document Symbols
connection.onDocumentSymbol(async (params, _token) => {
    const wasmURI = getSlangdURI(params.textDocument.uri, initializationOptions.workspaceUris);
    const result = slangd.documentSymbol(wasmURI);
    if (!result || typeof result.size !== 'function' || typeof result.get !== 'function') return [];
    const symbols = [];
    for (let i = 0; i < result.size(); i++) {
        const sym = result.get(i);
        if (!sym) continue;
        symbols.push(convertDocumentSymbol(sym));
    }
    return symbols;
});

// This whole technique is somewhat hacky, but I'm not sure there's a better way to make playground imports work
const loadedPlaygroundFiles: Set<string> = new Set();
function openPlayground(wasmURI: string) {
    let splitUri = wasmURI.split('/');
    splitUri.pop(); // Remove the file name
    const playgroundURI = splitUri.join('/') + '/playground.slang';
    if (loadedPlaygroundFiles.has(playgroundURI)) {
        return; // Already opened
    }
    const emscriptenPlaygroundURI = removePrefix(playgroundURI, "file://");
    loadedPlaygroundFiles.add(playgroundURI);
    slangd.didOpenTextDocument(playgroundURI, PLAYGROUND_SOURCE);
    loadFileIntoEmscriptenFS(emscriptenPlaygroundURI, PLAYGROUND_SOURCE);
}

function reportDiagnostics(uri: string) {
    const wasmURI = getSlangdURI(uri, initializationOptions.workspaceUris)
    const diagnostics = slangd.getDiagnostics(wasmURI);
    if (!diagnostics) {
        console.warn(`No diagnostics found for ${wasmURI}`);
        connection.sendDiagnostics({ uri, diagnostics: [] });
        return;
    }
    const lspDiagnostics: Diagnostic[] = [];
    for (let i = 0; i < diagnostics.size(); i++) {
        const d = diagnostics.get(i);
        if (!d) continue;
        lspDiagnostics.push({
            range: d.range,
            message: d.message.toString(),
            severity: d.severity as DiagnosticSeverity,
            code: d.code.toString(),
            source: 'slang',
        });
    }
    connection.sendDiagnostics({ uri, diagnostics: lspDiagnostics });

}

connection.onDidOpenTextDocument(async (params) => {
    const uri = params.textDocument.uri;
    const wasmURI = getSlangdURI(uri, initializationOptions.workspaceUris);
    const emscriptenURI = getEmscriptenURI(uri, initializationOptions.workspaceUris);
    loadFileIntoEmscriptenFS(emscriptenURI, params.textDocument.text);
    slangd.didOpenTextDocument(wasmURI, params.textDocument.text);

    openPlayground(wasmURI);
    reportDiagnostics(uri);
});
// Diagnostics (textDocument/didChange, didOpen, didClose handled by TextDocuments)
connection.onDidChangeTextDocument(async (params) => {
    const uri = params.textDocument.uri;
    const wasmURI = getSlangdURI(uri, initializationOptions.workspaceUris)
    const emscriptenURI = getEmscriptenURI(uri, initializationOptions.workspaceUris);
    modifyEmscriptenFile(emscriptenURI, params.contentChanges, slangWasmModule);
    let lspChanges = null;
    lspChanges = new slangWasmModule.TextEditList();
    for (const change of params.contentChanges) {
        if (TextDocumentContentChangeEvent.isIncremental(change))
            lspChanges.push_back(change);
        else
            console.error("Change should be incremental but isn't")
    }
    slangd.didChangeTextDocument(wasmURI, lspChanges);
    if (lspChanges.delete) lspChanges.delete();
    reportDiagnostics(uri);
});

connection.onDidCloseTextDocument(async (params) => {
    const uri = params.textDocument.uri;
    const wasmURI = getSlangdURI(uri, initializationOptions.workspaceUris);

    slangd.didCloseTextDocument(wasmURI);
});

connection.onRequest('slang/compile', async (params: WorkerRequest & { type: 'slang/compile' }): Promise<Result<Shader>> => {
    const shaderPath = getEmscriptenURI(params.shaderPath, initializationOptions.workspaceUris);
    return await compiler.compile(params, shaderPath, initializationOptions.workspaceUris, spirvTools);
});
connection.onRequest('slang/compilePlayground', async (params: WorkerRequest & { type: 'slang/compilePlayground' }): Promise<Result<CompiledPlayground>> => {
    const shaderPath = getEmscriptenURI(params.shaderPath, initializationOptions.workspaceUris);
    const compilationResult = await compiler.compile(params, shaderPath, initializationOptions.workspaceUris, spirvTools);
    if (compilationResult.succ === false) {
        return compilationResult;
    }
    return compilePlayground(compilationResult.result, params.uri, params.entrypoint);
});

connection.onRequest('slang/entrypoints', async (params: WorkerRequest & { type: 'slang/entrypoints' }): Promise<Result<EntrypointsResult>> => {
    let path = getEmscriptenURI(params.shaderPath, initializationOptions.workspaceUris);
    return compiler.findDefinedEntryPoints(params.sourceCode, path)
});

// Listen on the connection
connection.listen();
