import slangvm from "slangvm";
import { HTTPSlangFileSystem, standardLibraryURL } from "slangvm/http-file-system";
import { MemorySlangFileSystem, OverlaySlangFileSystem } from "slangvm/file-system";

import { testOwningValues } from "./owning-values.js";

const state = globalThis.__slangvmTest = { status: "running", tests: [], output: [] };
const baseURL = new URL("./slang/", import.meta.url);
function assert(condition, message) {
    if (!condition) throw new Error(message);
    state.tests.push(message);
}
async function rejects(operation, pattern, message) {
    try { await operation(); }
    catch (error) { assert(pattern.test(error.message), message); return; }
    throw new Error(`${message}: expected rejection`);
}

const opened = [];
let passed = false;
async function open(settings = {}) {
    const { baseURL: sourceURL = baseURL, sources = {}, ...options } = settings;
    const vm = await slangvm.init({ fileSystem: new OverlaySlangFileSystem([
        new MemorySlangFileSystem(sources), new HTTPSlangFileSystem(sourceURL),
        new HTTPSlangFileSystem(standardLibraryURL),
    ]), print: (line) => state.output.push(line), ...options });
    opened.push(vm);
    return vm;
}

try {
    // Exercise the native adapter with no HTTP filesystem or implicit package imports.
    const edits = { "main.slang": "import helper; [host] int main() { return answer(); }" };
    const memory = new MemorySlangFileSystem(edits);
    edits["main.slang"] = "invalid edited source";
    const custom = await open({ fileSystem: new OverlaySlangFileSystem([memory, {
        async loadFile(path) {
            return path === "/helper.slang" ? new TextEncoder().encode("public int answer() { return 42; }") : null;
        },
    }]) });
    assert(await (await custom.load("/main.slang")).main() === 42, "custom async filesystem and memory snapshot");
    await custom.close();

    let available = false;
    const later = await open({ fileSystem: {
        loadFile(path) {
            return available && path === "/later.slang"
                ? new TextEncoder().encode("[host] int main() { return 43; }") : null;
        },
    } });
    await rejects(() => later.load("later.slang"), /find|load|open|exist/i, "missing custom file reports diagnostics");
    available = true;
    assert(await (await later.load("later.slang")).main() === 43, "native bridge does not retain missing files");
    await later.close();

    const dataFiles = await open({ sources: {
        "read.slang": `import slang.runtime.io.byte_stream;
            [host] uint main() {
                ByteStream stream = openByteStream("numbers.bin", ByteStreamMode.read);
                uint value = stream.read<uint>();
                closeByteStream(stream);
                return value;
            }`,
        "numbers.bin": new Uint8Array([120, 86, 52, 18]),
    } });
    assert(await (await dataFiles.load("read.slang")).main() === 0x12345678,
        "runtime binary file reads use the injected filesystem");
    await dataFiles.close();

    await testOwningValues(slangvm, assert);
    const vm = await open();
    const m = await vm.load("main.slang");
    assert(await m.add(2, 3) === 5, "scalar invocation");
    assert(await m.roundHalf(1.5) === 1.5, "half marshalling");
    const printed = m.printAndAdd(4, 5);
    assert(state.output.at(-1) === "entered", "Slang executes before the JS call returns");
    const another = m.add(6, 7);
    assert(await printed === 9 && await another === 13, "consecutive synchronous calls");
    await rejects(() => m.add(1.5, 1), /integer|int|range/i, "integer argument validation");
    await rejects(() => m.add(true, 1), /bool|numeric|number|integer/i, "boolean argument validation");
    assert(m.then === undefined, "module is not accidentally thenable");
    assert(await m.getFunction("then")() === 17, "reserved export escape hatch");

    const producer = await vm.load("producer.slang");
    const point = await producer.makePoint(3, 4);
    const consumer = await vm.load("consumer.slang");
    assert(point.typeName.includes("Point"), "opaque native type name");
    assert(await consumer.lengthSquared(point) === 25, "same-VM cross-program value transfer");
    const cached = await vm.load("consumer.slang");
    assert(await cached.lengthSquared(point) === 25, "cached module loading preserves identity");

    const second = await open();
    const other = await second.load("consumer.slang");
    await rejects(() => other.lengthSquared(point), /another VM/i, "cross-VM values rejected");
    await second.close();
    await rejects(() => vm.load("invalid.slang"), /error|syntax|expected|undefined/i,
        "compiler diagnostics reach JavaScript");
    point.dispose();
    await rejects(() => consumer.lengthSquared(point), /disposed/i, "disposed values rejected");

    let managedProducer = await vm.load("managed_producer.slang");
    const managedConsumer = await vm.load("managed_consumer.slang");
    const original = await managedProducer.makeState(7);
    const copy = await managedConsumer.copyState(original);
    assert(original.typeName.includes("State"), "managed aggregate retains its type name");
    await managedConsumer.advance(copy, 3);
    assert(await managedConsumer.readState(original) === 40,
        "cross-program managed copies preserve duplicate object aliases");
    const counter = await managedProducer.getCounter(original);
    const counters = await managedProducer.getCounters(original);
    const label = await managedProducer.getLabel(original);
    const literal = await managedProducer.makeLiteral();
    await rejects(() => managedProducer.makeThenFail(99, 0), /division by zero/i,
        "managed allocation failure reaches JavaScript");
    assert(await managedConsumer.readState(copy) === 40, "managed input survives failed invocation");
    const foreignVM = await open();
    const foreignConsumer = await foreignVM.load("managed_consumer.slang");
    await rejects(() => foreignConsumer.readState(original), /another VM/i,
        "managed values cannot cross VMs");
    await foreignVM.close();
    await rejects(() => managedConsumer.readState(counter), /type.*match/i,
        "managed nominal types remain checked");
    original.dispose();
    original.dispose();
    copy.dispose();
    managedProducer = undefined;
    await managedConsumer.advanceCounter(counter, 2);
    assert(await managedConsumer.readArray(counters) === 24,
        "direct class and runtime-array handles survive aggregate disposal");
    counter.dispose();
    await managedConsumer.advanceArray(counters, 1);
    assert(await managedConsumer.readArray(counters) === 26,
        "runtime array owns its managed elements independently");
    assert(await managedConsumer.textBytes(label) === 7,
        "dynamic String survives its parent aggregate");
    const labelCopy = await managedConsumer.copyText(label);
    label.dispose();
    assert(await managedConsumer.textBytes(labelCopy) === 7 &&
        await managedConsumer.textBytes(literal) === 12, "owned and literal Strings round trip");
    await rejects(() => managedConsumer.readState(original), /disposed/i,
        "disposed managed aggregate is rejected");
    await vm.close();
    assert(!point.isValid && point.typeName.includes("Point"), "close preserves cached type names");
    assert(!counters.isValid && !labelCopy.isValid && !literal.isValid,
        "close invalidates surviving managed handles");
    counters.dispose();
    labelCopy.dispose();
    literal.dispose();
    await rejects(() => managedConsumer.readArray(counters), /closed/i,
        "closed VM rejects managed calls after late disposal");

    if (new URL(location.href).searchParams.get("gpu") === "1") {
        await testOwningValues(slangvm, assert, "webgpu");
        // Exercise the actual example modules, including the GPU resource inside the struct.
        const resources = await open({
            backend: "webgpu",
            baseURL: new URL("../../../examples/slangvm/js/", import.meta.url),
        });
        const resourceProducer = await resources.load("gpu-producer.slang");
        const gpuState = await resourceProducer.makeState(4, 10);
        const resourceConsumer = await resources.load("gpu-consumer.slang");
        const gpuCopy = await resourceConsumer.copyState(gpuState);
        assert(gpuState.typeName.includes("GpuState"), "GPU resource struct retains its type name");
        assert(await resourceConsumer.sum(gpuState) === 46,
            "returned GPU buffer survives producer invocation and later program loading");
        await resourceConsumer.add(gpuCopy, 1);
        assert(await resourceConsumer.sum(gpuState) === 50,
            "cross-program struct copies share the GPU resource");
        gpuState.dispose();
        assert(await resourceConsumer.sum(gpuCopy) === 50,
            "GPU resource survives disposal of another owning value");
        await rejects(() => resourceConsumer.sum(gpuState), /disposed/i,
            "disposed GPU resource values are rejected");
        const temporary = await resourceConsumer.copyState(gpuCopy);
        const readback = resourceConsumer.sum(temporary);
        temporary.dispose();
        assert(await readback === 50, "GPU readback retains input while its JS wrapper is disposed");
        await resourceConsumer.add(gpuCopy, 1);
        await resources.close();
        assert(!gpuCopy.isValid && gpuCopy.typeName.includes("GpuState"),
            "close drains GPU work and invalidates surviving resource values");
        gpuCopy.dispose();

        let signalPresented;
        const presentationReady = new Promise((resolve) => { signalPresented = resolve; });
        const gpu = await open({
            backend: "webgpu",
            print: (line) => {
                state.output.push(line);
                if (line === "presented") signalPresented();
            },
        });
        const canvas = document.querySelector("canvas");
        const window = await gpu.createWindow(canvas);
        const graphics = await gpu.load("graphics.slang");
        assert(await graphics.runReadBack() === 42, "WebGPU dispatch and JSPI readback");
        const presentation = graphics.present(window);
        await Promise.race([
            presentationReady,
            presentation.then(() => { throw new Error("Presentation ended before its snapshot signal"); }),
        ]);
        // Snapshot the WebGPU canvas through the standard Canvas image-source API. This
        // checks the actual presented pixels, including the host's intermediate-texture blit.
        const snapshot = document.createElement("canvas");
        snapshot.width = canvas.width;
        snapshot.height = canvas.height;
        const context = snapshot.getContext("2d", { willReadFrequently: true });
        context.drawImage(canvas, 0, 0);
        const pixel = context.getImageData(canvas.width >> 1, canvas.height >> 1, 1, 1).data;
        state.canvasPixel = [...pixel];
        assert([64, 128, 191, 255].every((expected, i) => Math.abs(pixel[i] - expected) <= 2),
            `Canvas pixels match the Slang compute output (${[...pixel].join(", ")})`);
        assert(await presentation === 1, "Canvas compute, blit, and presentation");
        const wait = graphics.waitForFrame(window);
        assert(state.output.at(-1) === "before-frame", "frame call enters synchronously");
        await rejects(() => graphics.runReadBack(), /busy/i, "overlap rejected while suspended");
        assert(await wait === true, "frame wait resumes through JSPI");
        assert(state.output.at(-1) === "after-frame", "frame continuation executes");
        const suspended = graphics.waitForFrame(window);
        const closing = gpu.close();
        assert(gpu.closed && !window.isValid, "close invalidates suspended VM handles immediately");
        await rejects(() => suspended, /closed|cancel|abort/i, "close unwinds frame suspension");
        await closing;
    } else {
        state.skipped = ["WebGPU tests: append ?gpu=1 to enable"];
    }
    passed = true;
} catch (error) {
    state.error = error.stack ?? String(error);
    console.error(error);
} finally {
    for (const vm of opened) {
        try { await vm.close(); }
        catch (error) { state.cleanupError = String(error); passed = false; }
    }
    state.status = passed ? "passed" : "failed";
    document.querySelector("#result").textContent = JSON.stringify(state, null, 2);
}
