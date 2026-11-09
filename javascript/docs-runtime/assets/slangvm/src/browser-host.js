let nextHostId = 1;
// A Canvas has one GPUCanvasContext and cannot be configured by two VMs at once.
const canvasOwners = new WeakMap();
const keyCodes = Object.freeze({
    Space: 32, Quote: 39, Comma: 44, Minus: 45, Period: 46, Slash: 47,
    Semicolon: 59, Equal: 61, BracketLeft: 91, Backslash: 92, BracketRight: 93,
    Backquote: 96, Escape: 256, Enter: 257, Tab: 258, Backspace: 259, Insert: 260,
    Delete: 261, ArrowRight: 262, ArrowLeft: 263, ArrowDown: 264, ArrowUp: 265,
    PageUp: 266, PageDown: 267, Home: 268, End: 269,
    ShiftLeft: 340, ShiftRight: 340, ControlLeft: 341, ControlRight: 341,
    AltLeft: 342, AltRight: 342,
});

function keyCode(code) {
    if (/^Key[A-Z]$/.test(code)) return code.charCodeAt(3);
    if (/^Digit[0-9]$/.test(code)) return code.charCodeAt(5);
    if (/^F([1-9]|1[0-2])$/.test(code)) return 289 + Number(code.slice(1));
    return keyCodes[code] ?? 0;
}

function inputEvent(type, properties = {}) {
    return { type, key: 0, button: 3, position: [0, 0], scroll: [0, 0], size: [0, 0],
        ...properties };
}

/** DOM services called by the native host through JSPI-aware imports. */
export class BrowserHost {
    #hostId = nextHostId++;
    #nextCanvasId = 1;
    #windows = new Map();
    #canvasIds = new WeakMap();
    #closed = false;
    #print;
    #onPresent;
    #beforeFrame;
    #defaultCanvas;

    constructor(settings) {
        this.#print = settings.print ?? ((line) => console.log(line));
        this.#onPresent = settings.onPresent;
        this.#beforeFrame = settings.beforeFrame;
        this.#defaultCanvas = settings.canvas;
    }

    isClosed() { return this.#closed; }
    print(line) { this.#print(line); }
    #window(id) {
        const window = this.#windows.get(id);
        if (!window) throw new Error("Unknown slangvm Canvas");
        return window;
    }

    #resize(window) {
        // The embedding page controls the Canvas backing resolution and its CSS layout.
        // Changing width/height here could resize intrinsic CSS dimensions recursively.
        const { width, height } = window.canvas;
        if (window.width === width && window.height === height) return;
        window.width = width;
        window.height = height;
        window.events.push(inputEvent(7, { size: [width, height] }));
    }

    registerCanvas(canvas, owned = false) {
        if (this.#closed) throw new Error("The slangvm VM is closed");
        const canvasType = canvas?.ownerDocument?.defaultView?.HTMLCanvasElement;
        if (!canvasType || !(canvas instanceof canvasType)) {
            throw new TypeError("Expected an HTMLCanvasElement");
        }
        const existingId = this.#canvasIds.get(canvas);
        if (canvasOwners.has(canvas) && canvasOwners.get(canvas) !== this) {
            throw new Error("Canvas is already registered with another slangvm VM");
        }
        if (existingId !== undefined) {
            throw new Error("This Canvas is already registered with the VM");
        }
        const id = this.#nextCanvasId++;
        const attribute = `data-slangvm-${this.#hostId}`;
        if (canvas.hasAttribute(attribute)) throw new Error("Canvas is already registered");
        canvas.setAttribute(attribute, String(id));
        const window = {
            canvas, owned, attribute, open: true, events: [], cleanups: [], waiters: new Set(),
            tabIndex: canvas.getAttribute("tabindex"), touchAction: canvas.style.touchAction,
            label: canvas.getAttribute("aria-label"), width: canvas.width, height: canvas.height,
        };
        this.#windows.set(id, window);
        this.#canvasIds.set(canvas, id);
        canvasOwners.set(canvas, this);
        canvas.tabIndex = canvas.tabIndex < 0 ? 0 : canvas.tabIndex;
        canvas.style.touchAction = "none";
        const listen = (type, handler, options) => {
            canvas.addEventListener(type, handler, options);
            window.cleanups.push(() => canvas.removeEventListener(type, handler, options));
        };
        const position = (event) => {
            const bounds = canvas.getBoundingClientRect();
            return [
                bounds.width ? (event.clientX - bounds.left) * canvas.width / bounds.width : 0,
                bounds.height ? (event.clientY - bounds.top) * canvas.height / bounds.height : 0,
            ];
        };
        for (const [eventName, type] of [["keydown", 1], ["keyup", 2]]) {
            listen(eventName, (event) => {
                const key = keyCode(event.code);
                if (!key || event.repeat) return;
                event.preventDefault();
                window.events.push(inputEvent(type, { key }));
            });
        }
        const button = (value) => value === 0 ? 0 : value === 2 ? 1 : value === 1 ? 2 : 3;
        listen("pointerdown", (event) => {
            canvas.focus();
            canvas.setPointerCapture(event.pointerId);
            window.events.push(inputEvent(event.detail === 2 ? 13 : 3, {
                button: button(event.button), position: position(event),
            }));
        });
        listen("pointerup", (event) => {
            window.events.push(inputEvent(4, {
                button: button(event.button), position: position(event),
            }));
            if (canvas.hasPointerCapture(event.pointerId)) canvas.releasePointerCapture(event.pointerId);
        });
        listen("pointermove", (event) => {
            window.events.push(inputEvent(5, { position: position(event) }));
        });
        listen("wheel", (event) => {
            event.preventDefault();
            // HGVM scroll values are wheel steps. DOM pixel deltas use a conventional
            // 100-pixel step; line deltas use three lines per wheel step.
            const divisor = event.deltaMode === 1 ? 3 : event.deltaMode === 2 ? 1 : 100;
            window.events.push(inputEvent(6, {
                position: position(event), scroll: [-event.deltaX / divisor, -event.deltaY / divisor],
            }));
        }, { passive: false });
        for (const [eventName, type] of [
            ["pointerenter", 8], ["pointerleave", 9], ["focus", 10], ["blur", 11],
            ["lostpointercapture", 12],
        ]) {
            listen(eventName, () => window.events.push(inputEvent(type)));
        }
        listen("contextmenu", (event) => event.preventDefault());
        this.#resize(window);
        return id;
    }

    createWindow(title, width, height) {
        const canvas = this.#defaultCanvas ?? document.createElement("canvas");
        const owned = !this.#defaultCanvas;
        this.#defaultCanvas = undefined;
        if (owned) {
            canvas.width = width;
            canvas.height = height;
            canvas.style.width = `${width}px`;
            canvas.style.height = `${height}px`;
            document.body.append(canvas);
        }
        const id = this.registerCanvas(canvas, owned);
        this.setWindowTitle(id, title);
        return id;
    }

    getCanvasSelector(id) {
        const window = this.#window(id);
        return `[${window.attribute}="${id}"]`;
    }

    getWindowSize(id) {
        const window = this.#window(id);
        this.#resize(window);
        return [window.canvas.width, window.canvas.height];
    }

    didPresent(id) {
        // Canvas image sources must be copied before the browser expires this frame's texture.
        // Like print, this callback is synchronous; returned Promises are not awaited.
        this.#onPresent?.(this.#window(id).canvas);
    }

    processWindowEvents(id) {
        const window = this.#window(id);
        if (!window.open || this.#closed) return false;
        const browser = window.canvas.ownerDocument.defaultView;
        return new Promise((resolve, reject) => {
            const waiter = { frame: undefined, finish: undefined };
            waiter.finish = (open, error) => {
                if (!window.waiters.delete(waiter)) return;
                browser.cancelAnimationFrame(waiter.frame);
                if (error) reject(error);
                else resolve(open);
            };
            window.waiters.add(waiter);
            waiter.frame = browser.requestAnimationFrame(() => {
                try {
                    // Apply embedding-page size changes before exposing resize events to Slang.
                    this.#beforeFrame?.(window.canvas);
                    this.#resize(window);
                    waiter.finish(window.open && !this.#closed);
                } catch (error) {
                    waiter.finish(false, error);
                }
            });
        });
    }

    pollInputEvent(id) { return this.#window(id).events.shift() ?? null; }

    closeWindow(id) {
        const window = this.#window(id);
        if (!window.open) return;
        window.open = false;
        for (const waiter of [...window.waiters]) waiter.finish(false);
        for (const cleanup of window.cleanups.splice(0)) cleanup();
        window.events.length = 0;
    }

    setWindowTitle(id, title) {
        this.#window(id).canvas.setAttribute("aria-label", title);
    }

    unregisterCanvas(id) {
        const window = this.#window(id);
        this.closeWindow(id);
        window.canvas.removeAttribute(window.attribute);
        if (window.tabIndex === null) window.canvas.removeAttribute("tabindex");
        else window.canvas.setAttribute("tabindex", window.tabIndex);
        window.canvas.style.touchAction = window.touchAction;
        if (window.label === null) window.canvas.removeAttribute("aria-label");
        else window.canvas.setAttribute("aria-label", window.label);
        if (window.owned) window.canvas.remove();
        this.#canvasIds.delete(window.canvas);
        canvasOwners.delete(window.canvas);
        this.#windows.delete(id);
    }

    close() {
        if (this.#closed) return;
        this.#closed = true;
        for (const id of this.#windows.keys()) this.closeWindow(id);
    }

    dispose() {
        this.close();
        for (const id of [...this.#windows.keys()]) this.unregisterCanvas(id);
    }
}
