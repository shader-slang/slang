import { normalizePath } from "./file-system.js";
import { waitForFile } from "./file-system-host.js";

/** Default location of the package's standard modules; composition remains explicit. */
export const standardLibraryURL = new URL("../slang/", import.meta.url);

/** A reusable HTTP filesystem. Cached bytes are read-only and reads from cache are synchronous. */
export class HTTPSlangFileSystem {
    #baseURL;
    #fetch;
    #cache = new Map();
    #missing;
    #allowedPaths;

    constructor(baseURL, { fetch: fetchFile = globalThis.fetch, cacheNotFound = false,
        missingFileCache = new Set(), allowedPaths } = {}) {
        if (allowedPaths !== undefined) {
            if (typeof allowedPaths === "string" || typeof allowedPaths?.[Symbol.iterator] !== "function") {
                throw new TypeError("allowedPaths must be an iterable of file paths");
            }
            this.#allowedPaths = new Set(Array.from(allowedPaths, normalizePath));
        }
        this.#baseURL = new URL(baseURL);
        if (!this.#baseURL.pathname.endsWith("/")) this.#baseURL.pathname += "/";
        this.#baseURL.search = this.#baseURL.hash = "";
        this.#fetch = fetchFile.bind(globalThis);
        // A supplied cache does not implicitly enable caching. This is always opt-in.
        this.#missing = cacheNotFound ? missingFileCache : null;
    }

    #url(path) {
        return new URL(normalizePath(path).slice(1).split("/").map(encodeURIComponent).join("/"),
            this.#baseURL).href;
    }

    loadFile(path, { signal } = {}) {
        signal?.throwIfAborted();
        const name = normalizePath(path);
        // Reject import search candidates immediately, without an HTTP request or suspension.
        if (this.#allowedPaths && !this.#allowedPaths.has(name)) return null;
        const url = this.#url(name);
        if (this.#cache.has(name)) return waitForFile(this.#cache.get(name), signal);
        if (this.#missing?.has(url)) return null;
        // The shared fetch belongs to this filesystem, not to the first VM that requests it.
        const pending = Promise.resolve().then(() => this.#fetch(url)).then(async response => {
            if (response.status === 404 || response.status === 410) {
                this.#missing?.add(url);
                return null;
            }
            if (!response.ok) throw new Error(`Cannot load ${url}: HTTP ${response.status}`);
            return new Uint8Array(await response.arrayBuffer());
        }).then(bytes => {
            if (this.#cache.get(name) === pending) {
                if (bytes === null) this.#cache.delete(name);
                else this.#cache.set(name, bytes);
            }
            return bytes;
        }, error => {
            if (this.#cache.get(name) === pending) this.#cache.delete(name);
            throw error;
        });
        this.#cache.set(name, pending);
        return waitForFile(pending, signal);
    }

    /** Load required files concurrently. Missing files reject instead of silently omitting assets. */
    async preload(paths, { concurrency = 8, signal } = {}) {
        if (!Number.isInteger(concurrency) || concurrency < 1) {
            throw new TypeError("concurrency must be a positive integer");
        }
        const names = [...new Set(Array.from(paths, normalizePath))];
        let next = 0;
        const load = async () => {
            while (next < names.length) {
                signal?.throwIfAborted();
                const name = names[next++];
                if (await this.loadFile(name, { signal }) === null) {
                    throw new Error(`Cannot preload missing file: ${name}`);
                }
            }
        };
        await Promise.all(Array.from({ length: Math.min(concurrency, names.length) }, load));
        signal?.throwIfAborted();
    }

    /** HTTP has no directory listing: the caller supplies a package's file inventory. */
    preloadDirectory(path, { manifest, ...options }) {
        const prefix = normalizePath(path).replace(/\/$/, "") + "/";
        return this.preload(manifest.filter(name => normalizePath(name).startsWith(prefix)), options);
    }

    /** Copy resolved bytes for a worker. Never transfer this filesystem's own buffers. */
    snapshot() {
        return Object.fromEntries([...this.#cache].filter(([, bytes]) => bytes instanceof Uint8Array)
            .map(([path, bytes]) => [path, bytes.slice()]));
    }

    /** Invalidate downloaded bytes. Previously loaded native VM snapshots remain unchanged. */
    clearCache() { this.#cache.clear(); }
}
