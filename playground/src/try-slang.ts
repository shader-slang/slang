import { PLAYGROUND_SOURCE, SlangCompiler } from 'slang-compilation-engine';
import { RUNNABLE_ENTRY_POINT_NAMES } from "slang-playground-shared";
import { fetchWithProgress } from './util.js';
import type { LanguageServer, MainModule } from "./slang-wasm.js";
import type { PublicApi as JJsonPublicApi } from "jjsontree.js/src/ts/api.js"
import pako from 'pako';
import createModule from './slang-wasm.js';

declare global {
    let $jsontree: JJsonPublicApi
}

export let compiler: SlangCompiler | null = null;
export let slangd: LanguageServer | null = null;

export let moduleLoadingMessage = "";

export function checkShaderType(userSource: string) {
    // we did a pre-filter on the user input source code.
    let shaderTypes = RUNNABLE_ENTRY_POINT_NAMES.filter((entryPoint) => userSource.includes(entryPoint));

    // Only one of the main function should be defined.
    // In this case, we will know that the shader is not runnable, so we can only compile it.
    if (shaderTypes.length !== 1)
        return null;

    return shaderTypes[0];
}

// Event when loading the WebAssembly module
type ReplaceReturnType<T extends (...a: any) => any, TNewReturn> = (...a: Parameters<T>) => TNewReturn;
type ConfigType = Omit<EmscriptenModule, "instantiateWasm"> & {
    instantiateWasm: ReplaceReturnType<EmscriptenModule["instantiateWasm"], Promise<WebAssembly.Exports>>
};

// Define the Module object with a callback for initialization
const moduleConfig = {
    locateFile: function (path: string) {
        if (path.endsWith('.wasm')) {
            return new URL('./slang-wasm.wasm.gz', import.meta.url).href; // Use the gzip compressed file
        }
        return path;
    },
    instantiateWasm: async function (imports: WebAssembly.Imports, receiveInstance: (arg0: WebAssembly.Instance) => void): Promise<WebAssembly.Exports> {
        // Step 1: Fetch the compressed .wasm.gz file
        let progressBar = document.getElementById('progress-bar');
        const compressedData = await fetchWithProgress(new URL('./slang-wasm.wasm.gz', import.meta.url).href, (loaded, total) => {
            const progress = (loaded / total) * 100;
            if (progressBar == null) progressBar = document.getElementById('progress-bar');
            if (progressBar) progressBar.style.width = `${progress}%`;
        });

        // Step 2: Decompress the gzip data
        const wasmBinary = pako.inflate(compressedData);

        // Step 3: Instantiate the WebAssembly module from the decompressed data
        const { instance } = await WebAssembly.instantiate(wasmBinary, imports);
        receiveInstance(instance);
        return instance.exports;
    }
} satisfies Partial<ConfigType> as any;

createModule(moduleConfig).then((module) => {
    let label = document.getElementById("loadingStatusLabel");
    if (label)
        label.innerText = "Initializing Slang Compiler...";
    try {
        {
            let FS = module.FS;
            FS.writeFile("/playground.slang", PLAYGROUND_SOURCE);
            FS.writeFile("/user.slang", "");
        }
        compiler = new SlangCompiler(module);
        let result = compiler.init();
        slangd = module.createLanguageServer();
        if (result.succ) {
            moduleLoadingMessage = "Slang compiler initialized successfully.\n";
            window.dispatchEvent(new CustomEvent('slangLoaded', {}));
        }
        else {
            console.log(result.message);
            moduleLoadingMessage = "Failed to initialize Slang Compiler.\n";
            if (label)
                label.innerText = moduleLoadingMessage;
        }
    }
    catch (error: any) {
        if (label) {
            label.innerText = error.toString(error);
            console.error(error);
        }
    }
});