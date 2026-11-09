export interface SlangFileSystem {
    /** Virtual paths use '/' separators. Returned bytes must not be mutated.
     * Return null only for a missing file; throw/reject on other errors. */
    loadFile(path: string, options?: { signal?: AbortSignal }):
        Uint8Array | null | Promise<Uint8Array | null>;
}
export function normalizePath(path: string): string;
export class MemorySlangFileSystem implements SlangFileSystem {
    constructor(files?: Readonly<Record<string, string | Uint8Array>>);
    loadFile(path: string, options?: { signal?: AbortSignal }): Uint8Array | null;
}
export class OverlaySlangFileSystem implements SlangFileSystem {
    constructor(layers: Iterable<SlangFileSystem>);
    loadFile(path: string, options?: { signal?: AbortSignal }):
        Uint8Array | null | Promise<Uint8Array | null>;
}
