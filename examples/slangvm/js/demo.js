import slangvm from "slangvm";
import { HTTPSlangFileSystem, standardLibraryURL } from "slangvm/http-file-system";
import { OverlaySlangFileSystem } from "slangvm/file-system";


const output = document.querySelector("#output");
const print = (line) => { output.textContent += `${line}\n`; };
const fileSystem = new OverlaySlangFileSystem([
    new HTTPSlangFileSystem(new URL("./", import.meta.url)),
    new HTTPSlangFileSystem(standardLibraryURL),
]);

async function scalarExample() {
    const vm = await slangvm.init({ fileSystem, print });
    try {
        const m = await vm.load("main.slang");
        print(`add(2, 3) = ${await m.add(2, 3)}`);
        print(`scale(1.5, 2) = ${await m.scale(1.5, 2)}`);

        const producer = await vm.load("producer.slang");
        const point = await producer.makePoint(3, 4);
        const consumer = await vm.load("consumer.slang");
        print(`${point.typeName}: lengthSquared = ${await consumer.lengthSquared(point)}`);
        point.dispose();
    } finally {
        await vm.close();
    }
}

async function gpuValueExample(vm) {
    const producer = await vm.load("gpu-producer.slang");
    const original = await producer.makeState(4, 10);
    const consumer = await vm.load("gpu-consumer.slang");
    const copy = await consumer.copyState(original);
    print(`${original.typeName}: initial GPU sum = ${await consumer.sum(original)}`); // 46
    await consumer.add(copy, 1);
    print(`GPU sum through the original = ${await consumer.sum(original)}`); // 50
    original.dispose();
    print(`GPU sum after disposing the original = ${await consumer.sum(copy)}`); // 50
    // Keep this handle to demonstrate that VM close invalidates a surviving GPU value.
    return copy;
}

const run = document.querySelector("#run");
const stop = document.querySelector("#stop");
const canvas = document.querySelector("#canvas");
let activeWindow;
stop.addEventListener("click", () => activeWindow?.close());

run.addEventListener("click", async () => {
    run.disabled = true;
    let vm;
    let gpuState;
    // The embedding page owns Canvas sizing. Slang receives backing-pixel dimensions.
    const resize = () => {
        const bounds = canvas.getBoundingClientRect();
        canvas.width = Math.max(1, Math.round(bounds.width * devicePixelRatio));
        canvas.height = Math.max(1, Math.round(bounds.height * devicePixelRatio));
    };
    const observer = new ResizeObserver(resize);
    observer.observe(canvas);
    resize();
    try {
        vm = await slangvm.init({ fileSystem, backend: "webgpu", print });
        gpuState = await gpuValueExample(vm);
        activeWindow = await vm.createWindow(canvas);
        stop.disabled = false;
        const m = await vm.load("viewer.slang");
        await m.main(activeWindow);
        print("Canvas example completed.");
    } catch (error) {
        print(error.message);
    } finally {
        activeWindow?.close();
        activeWindow = undefined;
        stop.disabled = true;
        observer.disconnect();
        if (vm) await vm.close();
        if (gpuState) {
            print(`After VM close: ${gpuState}, valid = ${gpuState.isValid}`);
            gpuState.dispose(); // Safe after close; the VM already released the payload.
        }
        run.disabled = false;
    }
});

scalarExample().catch((error) => print(error.message));
