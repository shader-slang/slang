import { normalizePath } from "./file-system.js";

/** Cancel one consumer without aborting a request shared with other VMs. */
export function waitForFile(result, signal) {
    signal?.throwIfAborted();
    if (!result || typeof result.then !== "function") return result;
    const pending = Promise.resolve(result);
    if (!signal) return pending;
    return new Promise((resolve, reject) => {
        const abort = () => reject(signal.reason);
        signal.addEventListener("abort", abort, { once: true });
        pending.then(resolve, reject).finally(() => signal.removeEventListener("abort", abort));
    });
}

/** Adapt a caller-owned filesystem to the lifetime and byte contract of one native VM. */
export class FileSystemHost {
    #fileSystem;
    #abort = new AbortController();

    constructor(fileSystem) {
        if (typeof fileSystem?.loadFile !== "function") {
            throw new TypeError("slangvm.init requires a fileSystem with a loadFile method");
        }
        this.#fileSystem = fileSystem;
    }

    loadFile(path) {
        const signal = this.#abort.signal;
        signal.throwIfAborted();
        const validate = bytes => {
            signal.throwIfAborted();
            if (bytes !== null && !(bytes instanceof Uint8Array)) {
                throw new TypeError(`loadFile(${path}) must return a Uint8Array or null`);
            }
            return bytes;
        };
        const result = waitForFile(this.#fileSystem.loadFile(normalizePath(path), { signal }), signal);
        return result instanceof Promise ? result.then(validate) : validate(result);
    }

    close() { this.#abort.abort(new Error("The slangvm VM is closed")); }
}
