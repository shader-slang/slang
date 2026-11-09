/** Normalize a virtual path, without interpreting any part of it as a URL. */
export function normalizePath(path) {
    if (typeof path !== "string" || path.includes("\0") || /^[a-z][a-z0-9+.-]*:/i.test(path)) {
        throw new TypeError("Expected a virtual filesystem path");
    }
    const parts = [];
    for (const part of path.replaceAll("\\", "/").split("/")) {
        if (!part || part === ".") continue;
        if (part === "..") {
            if (!parts.length) throw new Error("Path escapes the virtual root");
            parts.pop();
        } else parts.push(part);
    }
    return "/" + parts.join("/");
}

/** Take an owned snapshot of text sources or binary files. Returned bytes are read-only. */
export class MemorySlangFileSystem {
    #files = new Map();

    constructor(files = {}) {
        if (!files || typeof files !== "object" || Array.isArray(files)) {
            throw new TypeError("files must be a record of paths to strings or Uint8Arrays");
        }
        for (const [path, value] of Object.entries(files)) {
            const name = normalizePath(path);
            if (this.#files.has(name)) throw new Error(`Duplicate file path: ${path}`);
            if (typeof value !== "string" && !(value instanceof Uint8Array)) {
                throw new TypeError(`File ${path} must be a string or Uint8Array`);
            }
            this.#files.set(name, typeof value === "string" ? new TextEncoder().encode(value) : value.slice());
        }
    }

    loadFile(path, { signal } = {}) {
        signal?.throwIfAborted();
        return this.#files.get(normalizePath(path)) ?? null;
    }
}

/** Try layers in order. Only a missing file falls through; errors propagate to the caller. */
export class OverlaySlangFileSystem {
    #layers;

    constructor(layers) {
        this.#layers = Array.from(layers);
        if (this.#layers.some(layer => typeof layer?.loadFile !== "function")) {
            throw new TypeError("Each filesystem must implement loadFile");
        }
    }

    loadFile(path, options = {}) {
        const name = normalizePath(path);
        const next = (index) => {
            options.signal?.throwIfAborted();
            if (index === this.#layers.length) return null;
            const result = this.#layers[index].loadFile(name, options);
            if (result && typeof result.then === "function") {
                return Promise.resolve(result).then(bytes => bytes === null ? next(index + 1) : bytes);
            }
            return result === null ? next(index + 1) : result;
        };
        return next(0);
    }
}
