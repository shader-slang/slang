const assert = require("node:assert/strict");

// The orbit controller must reproduce the final scene's view, ignore moves without a drag,
// clamp pitch and distance, and report exactly the events that change the view.
exports.checkOrbitController = async function (page, sources, baseURL) {
    const result = await page.evaluate(async ({ sources, baseURL }) => {
        const { init } = await import(new URL("assets/slangvm/src/index.js", baseURL));
        const { MemorySlangFileSystem, OverlaySlangFileSystem } = await import(new URL("assets/slangvm/src/file-system.js", baseURL));
        const { preloadRuntime } = await import(new URL("scripts/snippet-filesystem.js", baseURL));
        const { preloadedFiles } = await preloadRuntime();
        const checks = `import slang.runtime; import camera; import "orbit-controller";
            InputEvent makeEvent(InputEventType type, float2 pos, float2 scroll)
            {
                InputEvent event;
                event.type = type;
                event.key = KeyCode.unknown;
                event.button = MouseButton.left;
                event.pos = pos;
                event.scroll = scroll;
                event.size = uint2(0u, 0u);
                return event;
            }
            [host] float orbit(int which) {
                var controller = OrbitController(float3(0.0, 1.5, 0.0), 5.55, 0.46, 0.255, 2.6);
                float3 start = controller.camera().position;
                float2 zero = float2(0.0, 0.0);
                bool idleMove = controller.handle(makeEvent(InputEventType.mouseMove, float2(10.0, 10.0), zero));
                controller.handle(makeEvent(InputEventType.mouseButtonDown, float2(10.0, 10.0), zero));
                bool drag = controller.handle(makeEvent(InputEventType.mouseMove, float2(110.0, 2000.0), zero));
                controller.handle(makeEvent(InputEventType.mouseCaptureLost, zero, zero));
                bool afterRelease = controller.handle(makeEvent(InputEventType.mouseMove, float2(500.0, 500.0), zero));
                bool scroll = controller.handle(makeEvent(InputEventType.mouseScroll, zero, float2(0.0, 100.0)));
                if (which == 0) return start.x;
                if (which == 1) return start.y;
                if (which == 2) return start.z;
                if (which == 3) return controller.yaw;
                if (which == 4) return controller.pitch;
                if (which == 5) return controller.distance;
                return (idleMove ? 1.0 : 0.0) + (drag ? 10.0 : 0.0) + (afterRelease ? 100.0 : 0.0) + (scroll ? 1000.0 : 0.0);
            }`;
        const vm = await init({ backend: "webgpu", fileSystem: new OverlaySlangFileSystem([
            new MemorySlangFileSystem({ ...sources, "orbit-check.slang": checks }),
            new MemorySlangFileSystem(preloadedFiles),
        ]) });
        try {
            const module = await vm.load("orbit-check.slang");
            const values = [];
            for (let i = 0; i < 7; i++) values.push(await module.orbit(i));
            return values;
        } finally { await vm.close(); }
    }, { sources, baseURL });
    const [x, y, z, yaw, pitch, distance, flags] = result;
    // The final scene's camera sits at (2.4, 2.9, -4.8), looking at (0, 1.5, 0).
    assert.ok(Math.hypot(x - 2.4, y - 2.9, z - 4.8 * -1) < 0.05, `Orbit start must match the final view: ${result}`);
    assert.ok(Math.abs(yaw - (0.46 - 0.5)) < 1e-5, `A 100-pixel drag must turn yaw by 0.5 radians: ${yaw}`);
    assert.ok(Math.abs(pitch - 1.4) < 1e-5, `Pitch must clamp at 1.4 radians: ${pitch}`);
    assert.ok(Math.abs(distance - 1.5) < 1e-5, `Distance must clamp at 1.5: ${distance}`);
    assert.equal(flags, 1010, "Only a drag move and a scroll may report a view change");
    console.log("PASS: orbit controller start view, drag, clamps, and change reporting");
};
