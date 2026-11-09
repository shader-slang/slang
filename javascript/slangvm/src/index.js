import { BrowserHost } from "./browser-host.js";
import { createVM, checkNativeResult } from "./runtime.js";
import { FileSystemHost } from "./file-system-host.js";

export { VM, Value, Window } from "./runtime.js";

export async function init(settings = {}) {
    if ("baseURL" in settings || "sources" in settings || "missingFileCache" in settings) {
        throw new TypeError("Use fileSystem with MemorySlangFileSystem and HTTPSlangFileSystem instead of baseURL/sources/missingFileCache");
    }
    const files = new FileSystemHost(settings.fileSystem);
    if (typeof WebAssembly.promising !== "function" ||
        typeof WebAssembly.Suspending !== "function") {
        throw new Error("slangvm requires a browser with WebAssembly JSPI support");
    }
    const backend = settings.backend ?? "none";
    if (backend !== "none" && backend !== "webgpu") {
        throw new TypeError("The browser backend must be 'none' or 'webgpu'");
    }
    const host = new BrowserHost(settings);
    let native;
    try {
        const { default: createWasmModule } = await import("../wasm/slangvm-wasm.js");
        // Keep the default asset URL literal so bundlers can discover and copy the WASM file.
        const wasmURL = new URL("../wasm/slangvm-wasm.wasm", import.meta.url);
        const wasmDirectory = settings.wasmURL ? new URL(settings.wasmURL, import.meta.url) : null;
        const module = await createWasmModule({
            locateFile: (path) => wasmDirectory ? new URL(path, wasmDirectory).href : wasmURL.href,
            print: (line) => host.print(line),
            printErr: (line) => (settings.printErr ?? console.error)(line),
        });
        native = new module.WasmVM({
            backend,
            searchPaths: settings.searchPaths ?? ["/"],
        }, host, files);
        checkNativeResult(await native.initialize());
        return createVM(native, host, files);
    } catch (error) {
        files.close();
        host.close();
        try {
            if (native) {
                try { await native.close(); }
                finally { native.delete(); }
            }
        } finally {
            host.dispose();
        }
        throw error;
    }
}

export default Object.freeze({ init });
