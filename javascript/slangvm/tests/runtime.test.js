import assert from "node:assert/strict";
import { test } from "node:test";
import { createVM, Value, Window } from "../src/runtime.js";

function fixture() {
    const calls = [];
    let busy = false;
    let finish;
    let nextValue = 1;
    const host = {
        close() { calls.push("host.close"); finish?.(); },
        dispose() { calls.push("host.dispose"); },
        registerCanvas() { return 17; },
        unregisterCanvas(id) { calls.push(["unregisterCanvas", id]); },
        closeWindow(id) { calls.push(["host.closeWindow", id]); },
    };
    const native = {
        isBusy() { return busy; },
        load(path) {
            calls.push(["load", path]);
            return Promise.resolve({ moduleId: 1, functions: [
                { name: "add", functionId: 1 }, { name: "makePoint", functionId: 2 },
                { name: "lengthSquared", functionId: 3 }, { name: "wait", functionId: 4 },
                { name: "then", functionId: 5 }, { name: "getFunction", functionId: 6 },
                { name: "main", functionId: 7 },
            ] });
        },
        invoke(id, args) {
            calls.push(["invoke", id, args]);
            if (id === 1) return Promise.resolve({ kind: "scalar", value: args[0] + args[1] });
            if (id === 2) return Promise.resolve({ kind: "value", id: nextValue++, typeName: "Point" });
            if (id === 3) return Promise.resolve({ kind: "scalar", value: 25 });
            if (id === 4) {
                busy = true;
                return new Promise((resolve) => {
                    finish = () => { busy = false; resolve({ kind: "void" }); };
                });
            }
            return Promise.resolve({ kind: "void" });
        },
        createWindow(id) { return Promise.resolve({ windowId: id + 100 }); },
        releaseValue(id) { calls.push(["releaseValue", id]); },
        releaseWindow(id) { calls.push(["releaseWindow", id]); },
        close() { calls.push("native.close"); return Promise.resolve({}); },
        delete() { calls.push("native.delete"); },
    };
    return { vm: createVM(native, host), native, host, calls, finish: () => finish() };
}

test("calls enter native code before returning, including consecutive synchronous calls", async () => {
    const f = fixture();
    const module = await f.vm.load("main.slang");
    const first = module.add(2, 3);
    assert.deepEqual(f.calls.at(-1), ["invoke", 1, [2, 3]]);
    const second = module.add(4, 5);
    assert.deepEqual(f.calls.at(-1), ["invoke", 1, [4, 5]]);
    assert.deepEqual(await Promise.all([first, second]), [5, 9]);
    await f.vm.close();
});

test("reserved exports do not make a module thenable and remain callable explicitly", async () => {
    const f = fixture();
    const module = await f.vm.load("main.slang");
    assert.equal(module.then, undefined);
    assert.ok(module.functionNames.includes("then"));
    assert.equal(f.calls.filter((call) => call[0] === "invoke").length, 0);
    await module.getFunction("then")();
    await module.getFunction("getFunction")();
    assert.throws(() => module.getFunction("missing"), /No exported function/);
    await f.vm.close();
});

test("values pass between programs in the same VM and reject other VMs", async () => {
    const f = fixture();
    const other = fixture();
    const producer = await f.vm.load("producer.slang");
    const point = await producer.makePoint(3, 4);
    const consumer = await f.vm.load("consumer.slang");
    assert.ok(point instanceof Value);
    assert.equal(point.typeName, "Point");
    assert.equal(await consumer.lengthSquared(point), 25);
    assert.deepEqual(f.calls.at(-1), ["invoke", 3, [{ kind: "value", id: 1 }]]);
    const otherModule = await other.vm.load("consumer.slang");
    await assert.rejects(otherModule.lengthSquared(point), /another VM/);
    await f.vm.close();
    assert.equal(point.isValid, false);
    assert.equal(point.typeName, "Point");
    assert.equal(String(point), "Point (closed)");
    await other.vm.close();
});

test("disposed values cannot be passed and dispose is idempotent", async () => {
    const f = fixture();
    const module = await f.vm.load("main.slang");
    const point = await module.makePoint(3, 4);
    point.dispose();
    point.dispose();
    assert.equal(point.isValid, false);
    assert.deepEqual(f.calls.filter((call) => call[0] === "releaseValue"), [["releaseValue", 1]]);
    await assert.rejects(module.lengthSquared(point), /disposed/);
    await f.vm.close();
});

test("a suspended native invocation rejects overlap and defers native disposal", async () => {
    const f = fixture();
    const module = await f.vm.load("main.slang");
    const point = await module.makePoint(3, 4);
    const wait = module.wait();
    point.dispose();
    assert.equal(f.calls.some((call) => call[0] === "releaseValue"), false);
    await assert.rejects(module.add(1, 2), /busy/);
    await assert.rejects(f.vm.load("other.slang"), /busy/);
    f.finish();
    await wait;
    assert.deepEqual(f.calls.at(-1), ["releaseValue", 1]);
    assert.equal(await module.add(1, 2), 3);
    await f.vm.close();
});

test("close invalidates handles immediately and destroys native state only after unwind", async () => {
    const f = fixture();
    const module = await f.vm.load("main.slang");
    const point = await module.makePoint(3, 4);
    const invocation = module.wait();
    const close = f.vm.close();
    assert.equal(f.vm.closed, true);
    assert.equal(point.isValid, false);
    assert.equal(f.calls.includes("native.delete"), false);
    assert.equal(f.vm.close(), close);
    await assert.rejects(invocation, /closed/);
    await close;
    assert.deepEqual(f.calls.slice(-4), ["host.close", "native.close", "native.delete", "host.dispose"]);
    await assert.rejects(module.add(1, 2), /closed/);
    assert.throws(() => module.getFunction("add"), /closed/);
    point.dispose();
});

test("window arguments use owned native handles and close through the DOM host", async () => {
    const f = fixture();
    const module = await f.vm.load("main.slang");
    const window = await f.vm.createWindow({});
    assert.ok(window instanceof Window);
    await module.main(window);
    assert.deepEqual(f.calls.at(-1), ["invoke", 7, [{ kind: "window", id: 117 }]]);
    window.close();
    assert.deepEqual(f.calls.slice(-2), [["host.closeWindow", 17], ["releaseWindow", 117]]);
    await assert.rejects(module.main(window), /disposed/);
    await f.vm.close();
    window.dispose();
});

test("forged handles and arbitrary objects are rejected before native entry", async () => {
    const f = fixture();
    const module = await f.vm.load("main.slang");
    for (const argument of [{ kind: "value", id: 1 }, {}, null, "3", 3n, undefined]) {
        await assert.rejects(module.lengthSquared(argument), /Expected a number/);
    }
    assert.throws(() => new Value(), /created by Slang/);
    assert.throws(() => new Window(), /createWindow/);
    assert.equal(f.calls.filter((call) => call[0] === "invoke").length, 0);
    await f.vm.close();
});

test("native diagnostics propagate without losing status codes", async () => {
    const f = fixture();
    f.native.load = () => Promise.resolve({ error: "Invalid Slang source", code: -42 });
    await assert.rejects(f.vm.load("bad.slang"), (error) => {
        assert.equal(error.message, "Invalid Slang source");
        assert.equal(error.code, -42);
        return true;
    });
    await f.vm.close();
});

test("module completion does not reenter native code while another load is suspended", async () => {
    const f = fixture();
    const first = f.vm.load("main.slang");
    f.native.load = () => new Promise((resolve) => { f.completeLoad = resolve; });
    const second = f.vm.load("other.slang");
    assert.equal(typeof (await first).add, "function");
    f.completeLoad({ moduleId: 2, functions: [] });
    await second;
    await f.vm.close();
});

test("reentrant close from a print callback waits for a subsequent native suspension", async () => {
    const f = fixture();
    const module = await f.vm.load("main.slang");
    let close;
    let finishNative;
    f.native.invoke = () => {
        close = f.vm.close(); // Simulates user-provided print calling close inside WASM.
        return new Promise((resolve) => { finishNative = resolve; });
    };
    const invocation = module.add(2, 3);
    await Promise.resolve();
    assert.equal(f.calls.includes("native.delete"), false);
    finishNative({ kind: "scalar", value: 5 });
    await assert.rejects(invocation, /closed/);
    await close;
    assert.equal(f.calls.includes("native.delete"), true);
});

test("failed Window creation unregisters Canvas listeners and attributes", async () => {
    const f = fixture();
    f.native.createWindow = () => Promise.resolve({ error: "No GPU device", code: -1 });
    await assert.rejects(f.vm.createWindow({}), /No GPU device/);
    assert.deepEqual(f.calls.at(-1), ["unregisterCanvas", 17]);
    await f.vm.close();
});
