import type { SlangFileSystem } from "./file-system.js";
export const standardLibraryURL: URL;
export interface PreloadOptions {
    concurrency?: number;
    signal?: AbortSignal;
}
export interface HTTPFileSystemOptions {
    fetch?: typeof globalThis.fetch;
    /** Cache HTTP 404 and 410 responses. Defaults to false. */
    cacheNotFound?: boolean;
    /** Optional shared cache of absolute URLs. Used only when cacheNotFound is true. */
    missingFileCache?: { has(url: string): boolean; add(url: string): void };
}
export class HTTPSlangFileSystem implements SlangFileSystem {
    constructor(baseURL: string | URL, options?: HTTPFileSystemOptions);
    loadFile(path: string, options?: { signal?: AbortSignal }):
        Uint8Array | null | Promise<Uint8Array | null>;
    preload(paths: Iterable<string>, options?: PreloadOptions): Promise<void>;
    preloadDirectory(path: string, options: PreloadOptions & { manifest: readonly string[] }): Promise<void>;
    snapshot(): Record<string, Uint8Array>;
    clearCache(): void;
}
