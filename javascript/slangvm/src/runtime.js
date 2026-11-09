const constructionKey = Symbol("slangvm construction");
const values = new WeakMap();
const windows = new WeakMap();
const owners = new WeakMap();
const finalizers = typeof FinalizationRegistry === "function"
    ? new FinalizationRegistry(({ owner, kind, id }) => {
        const vm = owner.deref();
        if (vm) owners.get(vm).release(kind, id);
    })
    : null;

function check(result) {
    if (result && typeof result.error === "string") {
        const error = new Error(result.error);
        error.code = result.code;
        throw error;
    }
    return result;
}

/** An opaque native value. Its type name remains available after disposal or VM close. */
export class Value {
    constructor(key, vm, id, typeName) {
        if (key !== constructionKey) throw new TypeError("Values are created by Slang functions");
        values.set(this, { vm, id, typeName, disposed: false });
        finalizers?.register(this, { owner: new WeakRef(vm), kind: "value", id }, this);
        Object.freeze(this);
    }

    get typeName() { return values.get(this).typeName; }
    get isValid() {
        const state = values.get(this);
        return !state.disposed && !state.vm.closed;
    }
    toString() { return `${this.typeName}${this.isValid ? "" : " (closed)"}`; }
    dispose() {
        const state = values.get(this);
        if (state.disposed) return;
        state.disposed = true;
        finalizers?.unregister(this);
        owners.get(state.vm).release("value", state.id);
    }
}

/** A Canvas-backed Slang Window owned by one VM. */
export class Window {
    constructor(key, vm, id, canvasId) {
        if (key !== constructionKey) throw new TypeError("Use vm.createWindow(canvas)");
        windows.set(this, { vm, id, canvasId, disposed: false });
        finalizers?.register(this, { owner: new WeakRef(vm), kind: "window", id }, this);
        Object.freeze(this);
    }

    get isValid() {
        const state = windows.get(this);
        return !state.disposed && !state.vm.closed;
    }
    toString() { return `Window${this.isValid ? "" : " (closed)"}`; }
    close() {
        const state = windows.get(this);
        if (state.disposed) return;
        state.disposed = true;
        finalizers?.unregister(this);
        owners.get(state.vm).closeWindow(state.canvasId);
        owners.get(state.vm).release("window", state.id);
    }
    dispose() { this.close(); }
}

/** Owns one compiler snapshot, native VM, and browser host. */
export class VM {
    #native;
    #host;
    #fileSystem;
    #closed = false;
    #pending = new Set();
    #releases = [];
    #closing;

    constructor(key, native, host, fileSystem) {
        if (key !== constructionKey) throw new TypeError("Use slangvm.init(settings)");
        this.#native = native;
        this.#host = host;
        this.#fileSystem = fileSystem;
        owners.set(this, {
            release: (kind, id) => this.#release(kind, id),
            closeWindow: (id) => { if (!this.#closed) this.#host.closeWindow(id); },
        });
    }

    get closed() { return this.#closed; }

    #requireIdle() {
        if (this.#closed) throw new Error("The slangvm VM is closed");
        if (this.#native.isBusy()) throw new Error("The slangvm VM is busy");
    }

    #flushReleases() {
        if (this.#closed || this.#native.isBusy()) return;
        for (const { kind, id } of this.#releases.splice(0)) {
            if (kind === "value") this.#native.releaseValue(id);
            else this.#native.releaseWindow(id);
        }
    }

    #release(kind, id) {
        if (this.#closed) return;
        this.#releases.push({ kind, id });
        this.#flushReleases();
    }

    #run(operation, decode = (result) => result) {
        let finish;
        const token = new Promise((resolve) => { finish = resolve; });
        let nativeResult;
        try {
            this.#requireIdle();
            this.#flushReleases();
            // A print callback can call close() before native execution returns or suspends.
            // Register its lifetime first, while still entering native code on this stack.
            this.#pending.add(token);
            // Enter WASM before creating the completion continuation. No scheduling hop is
            // inserted ahead of the user's Slang code.
            nativeResult = operation();
        } catch (error) {
            this.#pending.delete(token);
            finish();
            return Promise.reject(error);
        }
        const completion = Promise.resolve(nativeResult).then((result) => {
            check(result);
            if (this.#closed) throw new Error("The slangvm VM is closed");
            return decode(result);
        });
        const settled = () => {
            this.#pending.delete(token);
            finish();
            this.#flushReleases();
        };
        completion.then(settled, settled);
        return completion;
    }

    #argument(argument) {
        const value = values.get(argument);
        const window = windows.get(argument);
        const state = value ?? window;
        if (state) {
            if (state.vm !== this) throw new TypeError("Cannot pass a value from another VM");
            if (state.disposed) throw new TypeError("Cannot pass a disposed value");
            return { kind: value ? "value" : "window", id: state.id };
        }
        if (typeof argument === "number" || typeof argument === "boolean") return argument;
        throw new TypeError("Expected a number, boolean, Slang value, or Window");
    }

    #result(result) {
        switch (result.kind) {
            case "void": return undefined;
            case "scalar": return result.value;
            case "value": return new Value(constructionKey, this, result.id, result.typeName);
            default: throw new Error("Invalid result from the slangvm bridge");
        }
    }

    load(path) {
        return this.#run(() => {
            if (typeof path !== "string" || !path) throw new TypeError("Expected a source path");
            return this.#native.load(path);
        }, ({ functions: exports }) => {
            const functions = new Map();
            for (const { name, functionId } of exports) {
                functions.set(name, (...args) => this.#run(
                    () => this.#native.invoke(functionId, args.map((arg) => this.#argument(arg))),
                    (result) => this.#result(result),
                ));
            }
            const module = Object.create(null);
            Object.defineProperties(module, {
                functionNames: { value: Object.freeze([...functions.keys()]), enumerable: true },
                getFunction: {
                    value: (name) => {
                        if (this.#closed) throw new Error("The slangvm VM is closed");
                        if (!functions.has(name)) throw new Error(`No exported function named ${name}`);
                        return functions.get(name);
                    },
                },
            });
            for (const [name, fn] of functions) {
                // A callable `then` would cause Promise resolution to invoke user code.
                if (name === "then" || Object.hasOwn(module, name)) continue;
                Object.defineProperty(module, name, { value: fn, enumerable: true });
            }
            return Object.freeze(module);
        });
    }

    createWindow(canvas) {
        return this.#run(() => {
            const canvasId = this.#host.registerCanvas(canvas);
            try {
                return Promise.resolve(this.#native.createWindow(canvasId)).then((result) => {
                    if (result.error) this.#host.unregisterCanvas(canvasId);
                    return { ...result, canvasId };
                }, (error) => {
                    this.#host.unregisterCanvas(canvasId);
                    throw error;
                });
            } catch (error) {
                this.#host.unregisterCanvas(canvasId);
                throw error;
            }
        }, ({ windowId, canvasId }) => new Window(constructionKey, this, windowId, canvasId));
    }

    close() {
        if (this.#closing) return this.#closing;
        this.#closed = true;
        this.#fileSystem?.close();
        this.#host.close();
        // A suspended WASM stack still owns C++ objects. Invalidate JS handles immediately,
        // but let native operations unwind before destroying the interpreter and its module.
        this.#closing = Promise.allSettled([...this.#pending]).then(async () => {
            try {
                check(await this.#native.close());
            } finally {
                this.#native.delete();
                this.#host.dispose();
                this.#releases.length = 0;
            }
        });
        return this.#closing;
    }
}

/** Internal seam for exercising ownership and call scheduling without loading WASM. */
export function createVM(native, host, fileSystem) { return new VM(constructionKey, native, host, fileSystem); }

export function checkNativeResult(result) { return check(result); }
