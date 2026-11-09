import assert from "node:assert/strict";
import { test } from "node:test";
import { BrowserHost } from "../src/browser-host.js";

function fixture() {
    const frames = new Map();
    let nextFrame = 1;
    const browser = {
        HTMLCanvasElement: null,
        requestAnimationFrame(callback) { const id = nextFrame++; frames.set(id, callback); return id; },
        cancelAnimationFrame(id) { frames.delete(id); },
    };
    class Canvas extends EventTarget {
        width = 320;
        height = 200;
        tabIndex = -1;
        style = { touchAction: "pan-y" };
        ownerDocument = { defaultView: browser };
        attributes = new Map([["aria-label", "Original"]]);
        hasAttribute(name) { return this.attributes.has(name); }
        getAttribute(name) { return this.attributes.get(name) ?? null; }
        setAttribute(name, value) { this.attributes.set(name, String(value)); }
        removeAttribute(name) { this.attributes.delete(name); }
        getBoundingClientRect() { return { left: 10, top: 20, width: 160, height: 100 }; }
        focus() {}
        setPointerCapture() {}
        hasPointerCapture() { return false; }
        remove() { this.removed = true; }
    }
    browser.HTMLCanvasElement = Canvas;
    const host = new BrowserHost({});
    const canvas = new Canvas();
    const id = host.registerCanvas(canvas);
    return { host, canvas, id, frames, frame: () => {
        for (const callback of [...frames.values()]) callback();
    } };
}

function event(type, properties) { return Object.assign(new Event(type, { cancelable: true }), properties); }

test("frame waits suspend while event polling and closed windows complete synchronously", async () => {
    const f = fixture();
    assert.equal(f.host.pollInputEvent(f.id), null);
    const waiting = f.host.processWindowEvents(f.id);
    assert.ok(waiting instanceof Promise);
    f.frame();
    assert.equal(await waiting, true);
    const closingWait = f.host.processWindowEvents(f.id);
    f.host.closeWindow(f.id);
    assert.equal(await closingWait, false);
    assert.equal(f.host.processWindowEvents(f.id), false);
    assert.equal(f.frames.size, 0);
    f.host.dispose();
});

test("backing-pixel changes generate resize events without changing page layout", () => {
    const f = fixture();
    assert.deepEqual(f.host.getWindowSize(f.id), [320, 200]);
    assert.equal(f.host.pollInputEvent(f.id), null);
    f.canvas.width = 640;
    assert.deepEqual(f.host.getWindowSize(f.id), [640, 200]);
    const resized = f.host.pollInputEvent(f.id);
    assert.equal(resized.type, 7);
    assert.deepEqual(resized.size, [640, 200]);
    assert.equal(f.canvas.style.width, undefined);
    f.host.dispose();
});

test("input maps pointer positions into backing pixels and uses native HGVM key codes", () => {
    const f = fixture();
    f.canvas.dispatchEvent(event("pointermove", { clientX: 50, clientY: 45 }));
    const pointer = f.host.pollInputEvent(f.id);
    assert.equal(pointer.type, 5);
    assert.deepEqual(pointer.position, [80, 50]);
    f.canvas.dispatchEvent(event("keydown", { code: "KeyW", repeat: false }));
    const key = f.host.pollInputEvent(f.id);
    assert.equal(key.type, 1);
    assert.equal(key.key, 87);
    f.canvas.dispatchEvent(event("keydown", { code: "KeyW", repeat: true }));
    assert.equal(f.host.pollInputEvent(f.id), null);
    f.host.dispose();
});

test("registration is unique and disposal restores attributes and removes listeners", () => {
    const f = fixture();
    assert.throws(() => f.host.registerCanvas(f.canvas), /already registered/);
    assert.match(f.host.getCanvasSelector(f.id), /^\[data-slangvm-/);
    f.host.setWindowTitle(f.id, "Slang");
    assert.equal(f.canvas.getAttribute("aria-label"), "Slang");
    f.host.dispose();
    assert.equal(f.canvas.getAttribute("aria-label"), "Original");
    assert.equal(f.canvas.style.touchAction, "pan-y");
    assert.equal(f.canvas.getAttribute("tabindex"), null);
    assert.equal([...f.canvas.attributes.keys()].some((name) => name.startsWith("data-slangvm")), false);
    assert.equal(f.canvas.removed, undefined);
    assert.throws(() => f.host.registerCanvas(f.canvas), /closed/);
});

test("unregistering a failed adoption permits a later registration", () => {
    const f = fixture();
    f.host.unregisterCanvas(f.id);
    const next = f.host.registerCanvas(f.canvas);
    assert.notEqual(next, f.id);
    f.host.dispose();
});

test("a Canvas has one VM owner until its native Window is released", () => {
    const f = fixture();
    const other = new BrowserHost({});
    assert.throws(() => other.registerCanvas(f.canvas), /another slangvm VM/);
    f.host.unregisterCanvas(f.id);
    const adopted = other.registerCanvas(f.canvas);
    assert.match(other.getCanvasSelector(adopted), /^\[data-slangvm-/);
    f.host.dispose();
    other.dispose();
});
