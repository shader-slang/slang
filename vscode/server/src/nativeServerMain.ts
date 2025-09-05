
import createModule from '../../media/slang-wasm.node.js';
import type { MainModule } from '../../media/slang-wasm.node.js';
import spirvTools from '../../media/spirv-tools.node.js';
import type { EntrypointsRequest, Result, ServerInitializationOptions, WorkerRequest } from 'slang-playground-shared';
import { SlangCompiler, compilePlayground, PLAYGROUND_SOURCE } from 'slang-compilation-engine';
import { modifyEmscriptenFile, getEmscriptenURI, getSlangdURI, removePrefix } from './lspSharedUtils.js';
import { parentPort } from 'worker_threads';
// We'll set these after dynamic import
let compiler: SlangCompiler;
let slangWasmModule: MainModule;

// // Dynamically import the WASM module and set up the language server
let initializationOptions: ServerInitializationOptions;

globalThis.GPUShaderStage = { // Fix node's lack of support for WebGPU
    VERTEX: 0x1,
    FRAGMENT: 0x2,
    COMPUTE: 0x4,
};

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

let moduleReady: Promise<Result<undefined>> | null = null;
async function ensureSlangModuleLoaded() {
    if (moduleReady) return moduleReady;
    moduleReady = (async () => {
        // Instantiate the WASM module and create the language server
        slangWasmModule = await createModule();
        compiler = new SlangCompiler(slangWasmModule);
        return compiler.init();
    })();
    return moduleReady;
}

parentPort!.on("message", async (params: WorkerRequest) => {
    switch (params.type) {
        case 'Initialize':
            parentPort!.postMessage(await initialize(params));
            break;
        case 'DidOpenTextDocument':
            await DidOpenTextDocument(params);
            break;
        case 'DidChangeTextDocument':
            await DidChangeTextDocument(params);
            break;
        case 'slang/compile':
            await slangCompile(params);
            break;
        case 'slang/compilePlayground':
            await slangCompilePlayground(params);
            break;
        case 'slang/entrypoints':
            await slangEntrypoints(params);
            break;
        default:
            let _type: never = params; // Ensure all cases are handled
            console.error(`Unknown request type`);
            break;
    }
});

async function initialize(params: WorkerRequest & { type: 'Initialize' }): Promise<Result<undefined>> {
    // Accept extensionUri from initializationOptions
    if (params.initializationOptions) {
        initializationOptions = params.initializationOptions;
    }
    let moduleLoadResult = await ensureSlangModuleLoaded();

    if(!moduleLoadResult.succ) {
        return moduleLoadResult;
    }
    

    for (const file of initializationOptions.files) {
        const emscriptenURI = getEmscriptenURI(file.uri, initializationOptions.workspaceUris);
        loadFileIntoEmscriptenFS(emscriptenURI, file.content);
    }

    return {
        succ: true,
        result: undefined,
    }
}

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
    loadFileIntoEmscriptenFS(emscriptenPlaygroundURI, PLAYGROUND_SOURCE);
}

async function DidOpenTextDocument(params: WorkerRequest & { type: 'DidOpenTextDocument' }) {
    const uri = params.textDocument.uri;
    const wasmURI = getSlangdURI(uri, initializationOptions.workspaceUris);
    const emscriptenURI = getEmscriptenURI(uri, initializationOptions.workspaceUris);
    loadFileIntoEmscriptenFS(emscriptenURI, params.textDocument.text);

    openPlayground(wasmURI);
}
// Diagnostics (textDocument/didChange, didOpen, didClose handled by TextDocuments)
async function DidChangeTextDocument(params: WorkerRequest & { type: 'DidChangeTextDocument' }) {
    const uri = params.textDocument.uri;
    const emscriptenURI = getEmscriptenURI(uri, initializationOptions.workspaceUris);
    modifyEmscriptenFile(emscriptenURI, params.contentChanges, slangWasmModule);
}

async function slangCompile(params: WorkerRequest & { type: 'slang/compile' }) {
    const shaderPath = getEmscriptenURI(params.shaderPath, initializationOptions.workspaceUris);
    parentPort!.postMessage(await compiler.compile(params, shaderPath, initializationOptions.workspaceUris, spirvTools));
}

async function slangCompilePlayground(params: WorkerRequest & { type: 'slang/compilePlayground' }) {
    const shaderPath = getEmscriptenURI(params.shaderPath, initializationOptions.workspaceUris);
    const compilationResult = await compiler.compile(params, shaderPath, initializationOptions.workspaceUris, spirvTools);
    if (compilationResult.succ === false) {
        return compilationResult;
    }
    parentPort!.postMessage(compilePlayground(compilationResult.result, params.uri, params.entrypoint));
}

async function slangEntrypoints(params: EntrypointsRequest) {
    let path = getEmscriptenURI(params.shaderPath, initializationOptions.workspaceUris);
    parentPort!.postMessage(compiler.findDefinedEntryPoints(params.sourceCode, path));
}
