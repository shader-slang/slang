import type { SlangFileSystem } from "./file-system.js";
export type { SlangFileSystem } from "./file-system.js";

export interface Settings {
    /** Caller-owned filesystem, shared by the compiler and runtime file operations. */
    fileSystem: SlangFileSystem;
    /** Directory containing the WASM artifact, relative to this module for relative URLs. */
    wasmURL?: string | URL;
    backend?: "none" | "webgpu";
    /** Virtual search directories. Defaults to ["/"]. */
    searchPaths?: string[];
    /** Optional canvas used by the first Slang-side Window creation. */
    canvas?: HTMLCanvasElement;
    /** Called synchronously after presentation, while the current canvas image can be copied. */
    onPresent?: (canvas: HTMLCanvasElement) => void;
    /** Synchronous hook before processWindowEvents resumes; may update canvas backing dimensions. */
    beforeFrame?: (canvas: HTMLCanvasElement) => void;
    /** Synchronous line callback. Returned Promises are not awaited. */
    print?: (line: string) => void;
    printErr?: (line: string) => void;
}

export type Argument = number | boolean | Value | Window;
export type Result = number | boolean | Value | undefined;
export type BoundFunction = (...arguments_: Argument[]) => Promise<Result>;
export type Module<Functions = Record<string, BoundFunction>> = Functions & {
    readonly functionNames: readonly string[];
    /** Also accesses exports named `then`, `getFunction`, or `functionNames`. */
    getFunction(name: string): BoundFunction;
};

export class Value {
    private constructor();
    readonly typeName: string;
    readonly isValid: boolean;
    dispose(): void;
    toString(): string;
}

export class Window {
    private constructor();
    readonly isValid: boolean;
    close(): void;
    dispose(): void;
    toString(): string;
}

export class VM {
    private constructor();
    readonly closed: boolean;
    load<Functions = Record<string, BoundFunction>>(path: string): Promise<Module<Functions>>;
    createWindow(canvas: HTMLCanvasElement): Promise<Window>;
    /** Invalidates handles immediately; resolves after suspended native work has unwound. */
    close(): Promise<void>;
}

export function init(settings: Settings): Promise<VM>;
declare const slangvm: { readonly init: typeof init };
export default slangvm;
