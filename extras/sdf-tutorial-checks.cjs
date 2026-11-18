const assert = require("node:assert/strict");
const path = require("node:path");

exports.checkSdfTutorial = async function (browser, baseURL, outputDirectory) {
    const { cells, sources } = require("./sdf-tutorial-sources.cjs").readSdfTutorial();
    const names = ["render-sphere", "render-intersection", "render-shading", "render-blending", "render-environment", "render-mirror", "render-diffuse", "render-accumulation", "render-cosine-sampling", "render-glossy", "render-final-scene", "render-interactive"];
    assert.deepEqual(cells.filter(cell => cell.kind === "runnable").map(cell => cell.name), names);
    for (const cell of cells.filter(cell => cell.kind === "module")) {
        for (const [, dependency] of cell.source.matchAll(/^import\s+"?([\w-]+)"?;/gm)) {
            assert.ok(!names.includes(dependency), `${cell.name} must not depend on runnable lesson ${dependency}`);
        }
    }
    const context = await browser.newContext({ viewport: { width: 1360, height: 1000 } });
    try {
        const page = await context.newPage();
        const errors = [];
        page.on("pageerror", error => errors.push(error.message));
        await page.goto(baseURL + "sdf-path-tracer/");
        const card = name => page.locator(`[data-snippet="${name}"]`);
        async function run(name) {
            await card(name).locator(".snippet-monaco textarea").focus();
            await card(name).locator(".snippet-monaco textarea").press("Control+Enter");
            await page.waitForFunction(name => /^(Finished|Could not run)/.test(
                document.querySelector(`[data-snippet="${name}"] .snippet-status`).textContent), name, { timeout: 90000 });
            assert.equal(await card(name).locator(".snippet-status").textContent(), "Finished",
                await card(name).locator(".snippet-output").textContent());
            await card(name).locator(".snippet-display canvas").waitFor();
        }
        async function edit(name, text) {
            await card(name).evaluate(element => {
                for (let parent = element.parentElement; parent; parent = parent.parentElement)
                    if (parent.tagName === "DETAILS") parent.open = true;
            });
            const input = card(name).locator(".snippet-monaco textarea");
            await input.focus();
            await input.press("Control+Home");
            await input.press("Control+A");
            await input.evaluate((element, source) => {
                const clipboardData = new DataTransfer();
                clipboardData.setData("text/plain", source);
                element.dispatchEvent(new ClipboardEvent("paste", { clipboardData, bubbles: true, cancelable: true }));
            }, text);
        }
        async function pixels(name) {
            return card(name).locator(".snippet-display canvas").evaluate(canvas => {
                const context = canvas.getContext("2d");
                const sample = (x, y) => Array.from(context.getImageData(
                    Math.floor(canvas.width * x), Math.floor(canvas.height * y), 1, 1).data);
                const data = context.getImageData(0, 0, canvas.width, canvas.height).data;
                let white = 0;
                for (let i = 0; i < data.length; i += 4) if (data[i] > 250 && data[i + 1] > 250 && data[i + 2] > 250) white++;
                return { center: sample(0.5, 0.5), corner: sample(0.05, 0.05),
                    left: sample(0.43, 0.4), right: sample(0.57, 0.6), white };
            });
        }
        async function reset(name) {
            await card(name).locator(".snippet-monaco textarea").focus();
            const button = card(name).locator(".snippet-reset");
            await button.evaluate(element => element.scrollIntoView({ block: "center" }));
            await button.click();
        }
        async function blendShape() {
            return card("render-blending").locator(".snippet-display canvas").evaluate(canvas => {
                const data = canvas.getContext("2d").getImageData(0, 0, canvas.width, canvas.height).data;
                const isSurface = (x, y) => {
                    const i = 4 * (y * canvas.width + x);
                    return data[i + 1] > data[i] + 20 && data[i + 1] > data[i + 2] + 10;
                };
                let neckPixels = 0, rowSegments = 0, previous = false;
                for (let y = 0; y < canvas.height; y++) {
                    if (isSurface(Math.floor(canvas.width / 2), y)) neckPixels++;
                }
                for (let x = 0; x < canvas.width; x++) {
                    const current = isSurface(x, Math.floor(canvas.height / 2));
                    if (current && !previous) rowSegments++;
                    previous = current;
                }
                return { neckPixels, rowSegments };
            });
        }
        for (const { name, kind } of cells) {
            await card(name).locator(".snippet-monaco textarea").waitFor({ state: "attached" });
            assert.equal(await card(name).locator(".snippet-filename").textContent(), `${name}.slang`);
            if (kind === "module") {
                assert.equal(await card(name).locator(".snippet-run").isHidden(), true);
                assert.ok(await card(name).evaluate(element => !element.closest('details')?.open));
            }
        }
        // Run a dependent cell while its support extras are still collapsed.
        await run("render-blending");
        for (const name of ["render-sphere", "render-intersection", "render-shading"]) {
            assert.equal(await card(name).locator(".snippet-result").isVisible(), false);
        }
        const blended = await blendShape();
        assert.equal(blended.rowSegments, 1, "Blended spheres must form a connected silhouette");
        assert.ok(blended.neckPixels > 0, "The blended shape must have a visible neck");
        await card("render-blending").locator(".snippet-display canvas").screenshot({
            path: path.join(outputDirectory, "sdf-blending.png"),
        });
        await edit("render-blending", sources["render-blending.slang"].replace("blendWidth = 0.6", "blendWidth = 0.0"));
        await run("render-blending");
        const hardUnion = await blendShape();
        assert.equal(hardUnion.rowSegments, 1, "The default spheres overlap even without smoothing");
        assert.ok(blended.neckPixels > hardUnion.neckPixels * 1.25,
            "Smooth union must visibly fill out the join compared with an ordinary union");
        await edit("render-blending", sources["render-blending.slang"].replace("halfSeparation = 0.9", "halfSeparation = 1.2"));
        await run("render-blending");
        assert.equal((await blendShape()).rowSegments, 2, "Moving the spheres apart must break the neck");
        await reset("render-blending");

        await run("render-shading");
        assert.equal(await card("render-sphere").locator(".snippet-result").isVisible(), false);
        assert.equal(await card("render-intersection").locator(".snippet-result").isVisible(), false);
        const shaded = await pixels("render-shading");
        assert.ok(shaded.center[1] > shaded.center[0] + 30, "Sphere must be green");
        assert.ok(shaded.left[1] > shaded.right[1] + 25, "Upper-left side must face the light");
        await card("render-shading").screenshot({ path: path.join(outputDirectory, "sdf-shading.png") });
        await run("render-sphere");
        const slice = await pixels("render-sphere");
        assert.ok(slice.center[0] > slice.center[2] + 100, "Inside slice must be orange");
        assert.ok(slice.corner[2] > slice.corner[0] + 100, "Outside slice must be blue");
        await card("render-sphere").screenshot({ path: path.join(outputDirectory, "sdf-sphere.png") });
        await run("render-intersection");
        const mask = await pixels("render-intersection");
        assert.deepEqual(mask.center, [255, 255, 255, 255]);
        assert.deepEqual(mask.corner, [0, 0, 0, 255]);
        await card("render-intersection").screenshot({ path: path.join(outputDirectory, "sdf-tracing.png") });

        // Both execution and the visible language service must consume helper edits.
        await edit("sphere-distance", sources["sphere-distance.slang"].replaceAll("sphereDistance", "ballDistance"));
        await card("simple-sphere-tracer").locator(".snippet-monaco textarea").focus();
        await card("simple-sphere-tracer").locator(".squiggly-error").first().waitFor({ state: "attached", timeout: 90000 });
        assert.equal(await card("render-intersection").locator(".snippet-source-status").isVisible(), true);
        await reset("sphere-distance");
        await page.waitForFunction(() => !document.querySelector('[data-snippet="simple-sphere-tracer"] .squiggly-error'), null, { timeout: 90000 });

        // Lesson settings are independent; only shared algorithm edits cross cell boundaries.
        await edit("render-sphere", sources["render-sphere.slang"].replace("radius = 1.0", "radius = 0.7"));
        await run("render-intersection");
        assert.equal((await pixels("render-intersection")).white, mask.white,
            "Changing the slice's radius must not configure the intersection lesson");
        await reset("render-sphere");
        await edit("render-intersection", sources["render-intersection.slang"].replace("radius = 1.0", "radius = 0.7"));
        await run("render-intersection");
        const smaller = await pixels("render-intersection");
        assert.ok(smaller.white < mask.white * 0.6 && smaller.white > mask.white * 0.35,
            "The intersection lesson's own radius must shrink its mask");
        assert.equal(await card("render-intersection").locator(".snippet-source-status").isVisible(), false);
        await reset("render-intersection");
        await edit("sphere-distance", sources["sphere-distance.slang"].replace("- radius;", "- radius * 0.7;"));
        await run("render-intersection");
        assert.equal((await pixels("render-intersection")).white, smaller.white, "Shared geometry edits must reach consumers");
        await reset("sphere-distance");
        await run("render-intersection");
        assert.equal((await pixels("render-intersection")).white, mask.white, "Reset must restore imported code");

        await edit("render-shading", sources["render-shading.slang"].replace("float3(-0.6, 0.8, -1.0)", "float3(0.6, -0.8, -1.0)"));
        await run("render-shading");
        const movedLight = await pixels("render-shading");
        assert.ok(movedLight.right[1] > movedLight.left[1] + 25, "Changing the light must move the highlight");
        await reset("render-shading");

        const normalError = await page.evaluate(async ({ sources, baseURL }) => {
            const { init } = await import(new URL("assets/slangvm/src/index.js", baseURL));
            const { MemorySlangFileSystem, OverlaySlangFileSystem } = await import(new URL("assets/slangvm/src/file-system.js", baseURL));
            const { HTTPSlangFileSystem, standardLibraryURL } = await import(new URL("assets/slangvm/src/http-file-system.js", baseURL));
            const vm = await init({ backend: "webgpu", fileSystem: new OverlaySlangFileSystem([new MemorySlangFileSystem({
                ...sources,
                "check.slang": `import slang.runtime; import "sphere-sdf"; import "sphere-tracer";
                    [host] float main() {
                        let errors = RWStructuredBuffer<float>::alloc(4);
                        Grid(4).dispatch((uint i) => {
                            float3 p = normalize(float3(0.2 + float(i), 0.4, -0.7));
                            let center = float3(1, -2, 3);
                            let sphere = SphereSDF(center, 2.0);
                            errors[i] = length(surfaceNormal(sphere, center + 2.0 * p, 0.002) - p);
                        });
                        return max(max(errors[0], errors[1]), max(errors[2], errors[3]));
                    }`,
            }), new HTTPSlangFileSystem(standardLibraryURL)]) });
            try { return await (await vm.load("check.slang")).main(); }
            finally { await vm.close(); }
        }, { sources, baseURL });
        assert.ok(normalError < 0.001, `Finite-difference normal error: ${normalError}`);

        // Asset mappings must work even when this imported dependency has never been run.
        await run("render-mirror");
        assert.equal(await card("render-environment").locator(".snippet-result").isVisible(), false);
        const mirror = await pixels("render-mirror");
        await run("render-environment");
        const environment = await pixels("render-environment");
        assert.deepEqual(mirror.corner, environment.corner, "Misses must show the same environment");
        assert.notDeepEqual(mirror.left, environment.left, "Hits must reflect another direction");
        await edit("render-environment", sources["render-environment.slang"].replace("exposure = 1.0", "exposure = 0.5"));
        await run("render-environment");
        const darker = await pixels("render-environment");
        assert.ok(darker.corner.slice(0, 3).reduce((a, b) => a + b) < environment.corner.slice(0, 3).reduce((a, b) => a + b),
            "The visible exposure global must control display brightness");
        await reset("render-environment");
        await run("render-environment");

        async function imageData(name) {
            return card(name).locator(".snippet-display canvas").evaluate(canvas => ({
                width: canvas.width, height: canvas.height,
                data: Array.from(canvas.getContext("2d").getImageData(0, 0, canvas.width, canvas.height).data),
            }));
        }
        function noise(image) {
            // Neighbour differences in a small, smoothly shaded patch inside the sphere.
            let sum = 0, count = 0;
            for (let y = Math.floor(image.height * 0.4); y < image.height * 0.6; y++) {
                for (let x = Math.floor(image.width * 0.47); x < image.width * 0.53; x++) {
                    const i = 4 * (y * image.width + x) + 1;
                    sum += Math.abs(image.data[i] - image.data[i + 4]);
                    count++;
                }
            }
            return sum / count;
        }
        await run("render-diffuse");
        const oneSample = await imageData("render-diffuse");
        await edit("prng", sources["prng.slang"].replace("sampleIndex + 1u", "sampleIndex + 2u"));
        await card("prng").evaluate(element => { element.closest("details").open = false; });
        await run("render-diffuse");
        const changedSeed = await imageData("render-diffuse");
        assert.ok(changedSeed.data.some((value, i) => value !== oneSample.data[i]),
            "The renderer must consume edits to its PRNG module while collapsed");
        await card("prng").evaluate(element => { element.closest("details").open = true; });
        await reset("prng");
        assert.equal(await card("prng").locator(".snippet-run").isHidden(), true);
        await run("render-diffuse");
        assert.deepEqual(await imageData("render-diffuse"), oneSample);
        await edit("render-accumulation", sources["render-accumulation.slang"].replace("sampleLimit = 4096", "sampleLimit = 1"));
        await run("render-accumulation");
        assert.deepEqual(await imageData("render-accumulation"), oneSample,
            "The first accumulated frame must equal the standalone sample, without uninitialized history");
        // A partial final batch must use exactly the same samples/weights as single-sample frames.
        const partialBatch = sources["render-accumulation.slang"].replace("sampleLimit = 4096", "sampleLimit = 47");
        await edit("render-accumulation", partialBatch);
        await run("render-accumulation");
        const batched = await imageData("render-accumulation");
        await edit("render-accumulation", partialBatch.replace("samplesPerFrame = 32", "samplesPerFrame = 1"));
        await run("render-accumulation");
        const unbatched = await imageData("render-accumulation");
        assert.equal(batched.width, unbatched.width);
        assert.equal(batched.height, unbatched.height);
        assert.ok(batched.data.every((value, i) => Math.abs(value - unbatched.data[i]) <= 1),
            "32+15 samples must match 47 single-sample frames to display precision");

        // Use the lesson's full budget: the rotated HDR window needs more than 128 samples
        // for a reliable noise comparison in the now brightly lit central patch.
        await edit("render-accumulation", sources["render-accumulation.slang"]);
        await run("render-accumulation");
        const averaged = await imageData("render-accumulation");
        assert.ok(noise(averaged) < noise(oneSample) * 0.2,
            `4096 samples must visibly reduce noise: ${noise(oneSample)} -> ${noise(averaged)}`);

        // At the lesson's 32 samples, cosine-weighted sampling must be less noisy than uniform.
        await run("render-cosine-sampling");
        const cosineSamples = await imageData("render-cosine-sampling");
        await edit("render-cosine-sampling", sources["render-cosine-sampling.slang"]
            .replace("= HemisphereSampling.Cosine;", "= HemisphereSampling.Uniform;"));
        await run("render-cosine-sampling");
        const uniformSamples = await imageData("render-cosine-sampling");
        await reset("render-cosine-sampling");
        await run("render-cosine-sampling");
        assert.ok(noise(cosineSamples) < noise(uniformSamples),
            `Cosine sampling must reduce noise at 32 samples: ${noise(uniformSamples)} -> ${noise(cosineSamples)}`);
        for (const name of ["render-environment", "render-mirror", "render-diffuse", "render-accumulation", "render-cosine-sampling"]) {
            await card(name).locator(".snippet-display canvas").screenshot({
                path: path.join(outputDirectory, `sdf-${name}.png`),
            });
        }

        const samplingError = await page.evaluate(async ({ sources, baseURL }) => {
            const { init } = await import(new URL("assets/slangvm/src/index.js", baseURL));
            const { MemorySlangFileSystem, OverlaySlangFileSystem } = await import(new URL("assets/slangvm/src/file-system.js", baseURL));
            const { preloadRuntime } = await import(new URL("scripts/snippet-filesystem.js", baseURL));
            const { preloadedFiles } = await preloadRuntime();
            const vm = await init({ backend: "webgpu", fileSystem: new OverlaySlangFileSystem([
                new MemorySlangFileSystem({ ...sources, "check.slang": `
                    import slang.runtime; import "environment-map"; import lambertian; import hemisphere;
                    import prng; import scene; import "path-tracer"; import "sphere-sdf"; import "sphere-tracer";
                    [host] float main() {
                        let environmentMap = RWTexture2D<float4>::alloc(8, 4);
                        Grid(uint2(8u, 4u)).dispatch<8, 8>((uint2 p) => {
                            environmentMap[p] = float4(1.0);
                        });
                        let values = RWStructuredBuffer<float4>::alloc(4096);
                        Grid(4096).dispatch((uint i) => {
                            float3 albedo = float3(0.25, 0.65, 0.35);
                            uint uniformState = pixelSeed(uint2(32u), uint2(65u), i);
                            uint cosineState = uniformState;
                            let sphere = SphereSDF(float3(0), 1.0);
                            let tracing = TraceSettings(0.001, 10.0, 96);
                            let uniformScene = SingleMaterialScene<SphereSDF, Lambertian>(
                                sphere, Lambertian(albedo, HemisphereSampling.Uniform));
                            let cosineScene = SingleMaterialScene<SphereSDF, Lambertian>(
                                sphere, Lambertian(albedo, HemisphereSampling.Cosine));
                            float3 uniform = tracePath(uniformScene, environmentMap, float3(0, 0, -3),
                                float3(0, 0, 1), uniformState, 1u, tracing, 0.002, 0.004);
                            float3 cosine = tracePath(cosineScene, environmentMap, float3(0, 0, -3),
                                float3(0, 0, 1), cosineState, 1u, tracing, 0.002, 0.004);
                            uint state = mixBits(i);
                            float u = nextRandom(state);
                            float v = nextRandom(state);
                            // Include both poles, where constructing a tangent is easy to get wrong.
                            float3 normal = i % 2u == 0u ? float3(0, 1, 0) : float3(0, -1, 0);
                            float3 direction = i % 2u == 0u ? uniformHemisphere(normal, float2(u, v))
                                : cosineHemisphere(normal, float2(u, v));
                            float error = length(cosine - albedo) + abs(length(direction) - 1.0)
                                + max(0.0, -dot(normal, direction));
                            values[i] = float4(uniform, error);
                        });
                        let result = RWStructuredBuffer<float>::alloc(1);
                        Grid(1).dispatch((uint i) => {
                            float3 sum = float3(0.0);
                            float worst = 0.0;
                            for (uint j = 0; j < 4096; j++) {
                                sum += values[j].rgb;
                                worst = max(worst, values[j].a);
                            }
                            result[0] = worst + length(sum / 4096.0 - float3(0.25, 0.65, 0.35));
                        });
                        return result[0];
                    }`,
                }), new MemorySlangFileSystem(preloadedFiles),
            ]) });
            try { return await (await vm.load("check.slang")).main(); }
            finally { await vm.close(); }
        }, { sources, baseURL });
        assert.ok(samplingError < 0.015,
            `Uniform and cosine sampling must recover albedo under white light: ${samplingError}`);

        await require("./sdf-ggx-checks.cjs").checkGGXNumerics(page, sources, baseURL);
        for (const name of ["render-glossy"]) {
            await run(name);
            await card(name).locator(".snippet-display canvas").screenshot({
                path: path.join(outputDirectory, `sdf-${name}.png`),
            });
            // The smooth limit must reproduce the original mirror, including background misses.
            await edit(name, sources[`${name}.slang`]
                .replace("roughness = 0.35", "roughness = 0.0")
                .replace("F0 = float3(0.9)", "F0 = float3(1.0)")
                .replace("sampleLimit = 4096", "sampleLimit = 1"));
            await run(name);
            const smooth = await imageData(name);
            const mirror = await imageData("render-mirror");
            assert.deepEqual(smooth, mirror, `${name}: zero roughness and F0=1 must match the mirror`);
            await reset(name);
        }

        await require("./sdf-scene-checks.cjs").checkSceneNumerics(page, sources, baseURL);
        await run("render-final-scene");
        const finalPixels = await card("render-final-scene").locator(".snippet-display canvas").evaluate(canvas => {
            const data = canvas.getContext("2d").getImageData(0, 0, canvas.width, canvas.height).data;
            let blue = 0;
            for (let i = 0; i < data.length; i += 4) {
                if (data[i + 2] > data[i] + 35 && data[i + 1] > data[i] + 25) blue++;
            }
            // Camera framing is relative to image height, independent of aspect ratio.
            return blue / (canvas.height * canvas.height);
        });
        assert.ok(finalPixels > 0.12 && finalPixels < 0.4, "The blue sphere must occupy a substantial part of the final image");
        await card("render-final-scene").locator(".snippet-display canvas").screenshot({
            path: path.join(outputDirectory, "sdf-final-scene.png"),
        });

        // The interactive example keeps running past its sample limit until Stop, which leaves a
        // 2D snapshot of the last frame. Dragging with a real mouse must orbit the camera.
        await require("./sdf-orbit-checks.cjs").checkOrbitController(page, sources, baseURL);
        async function runInteractive(drag) {
            await card("render-interactive").locator(".snippet-monaco textarea").focus();
            await card("render-interactive").locator(".snippet-monaco textarea").press("Control+Enter");
            await page.waitForFunction(() => document.querySelector('[data-snippet="render-interactive"] .snippet-status')
                .textContent === "Running…", null, { timeout: 90000 });
            const live = card("render-interactive").locator(".snippet-display canvas");
            await live.waitFor();
            await page.waitForTimeout(3000);
            if (drag) {
                await live.scrollIntoViewIfNeeded();
                const box = await live.boundingBox();
                const x = box.x + box.width / 2, y = box.y + box.height / 2;
                await page.mouse.move(x, y);
                await page.mouse.down();
                await page.mouse.move(x + 150, y + 30, { steps: 10 });
                await page.mouse.up();
                await page.waitForTimeout(2000);
            }
            assert.equal(await card("render-interactive").locator(".snippet-status").textContent(), "Running…",
                "The interactive example must keep running after its sample limit");
            await card("render-interactive").locator(".snippet-stop").click();
            await page.waitForFunction(() => document.querySelector('[data-snippet="render-interactive"] .snippet-status')
                .textContent === "Stopped", null, { timeout: 30000 });
            return imageData("render-interactive");
        }
        function blockAverages(image, blocks = 8) {
            const sums = new Array(blocks * blocks * 3).fill(0), counts = new Array(blocks * blocks).fill(0);
            for (let y = 0; y < image.height; y++) {
                for (let x = 0; x < image.width; x++) {
                    const block = Math.floor(y * blocks / image.height) * blocks + Math.floor(x * blocks / image.width);
                    const i = 4 * (y * image.width + x);
                    for (let c = 0; c < 3; c++) sums[3 * block + c] += image.data[i + c];
                    counts[block]++;
                }
            }
            return sums.map((sum, i) => sum / counts[Math.floor(i / 3)]);
        }
        await edit("render-interactive", sources["render-interactive.slang"].replace("sampleLimit = 4096", "sampleLimit = 64"));
        const stillView = blockAverages(await runInteractive(false));
        const orbitedView = blockAverages(await runInteractive(true));
        await reset("render-interactive");
        const viewChange = stillView.reduce((sum, value, i) => sum + Math.abs(value - orbitedView[i]), 0) / stillView.length;
        assert.ok(viewChange > 15, `Dragging must orbit the interactive view: block difference ${viewChange}`);

        // Observe sample indices across a real resize, then exercise the reader's Stop button.
        await edit("frame-loop", sources["frame-loop.slang"]
            .replace("frame.firstSample += frame.sampleCount;", "frame.firstSample += frame.sampleCount; print(frame.firstSample);"));
        await edit("render-accumulation", sources["render-accumulation.slang"]
            .replace("sampleLimit = 4096", "sampleLimit = 65536"));
        await card("render-accumulation").locator(".snippet-monaco textarea").press("Control+Enter");
        await page.waitForFunction(() => /\b64\b/.test(
            document.querySelector('[data-snippet="render-accumulation"] .snippet-output').textContent), null, { timeout: 90000 });
        await page.setViewportSize({ width: 1000, height: 800 });
        await page.waitForFunction(() => {
            const counts = document.querySelector('[data-snippet="render-accumulation"] .snippet-output')
                .textContent.trim().split(/\s+/).map(Number);
            return counts.some((count, i) => i > 0 && count === 32 && counts[i - 1] > 32);
        }, null, { timeout: 90000 });
        await card("render-accumulation").locator(".snippet-stop").click();
        await page.waitForFunction(() => document.querySelector('[data-snippet="render-accumulation"] .snippet-status')
            .textContent === "Stopped", null, { timeout: 30000 });
        await reset("render-accumulation");
        await reset("frame-loop");
        await page.setViewportSize({ width: 390, height: 844 });
        assert.ok(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth), "Tutorial mobile overflow");
        assert.deepEqual(errors, []);
        await require("./sdf-module-checks.cjs").checkSdfModuleFallback(browser, baseURL, sources);
        console.log("PASS: SDF geometry, editable imports, environment assets, mirror, hemisphere weights, GGX renders/mirror limit, 32-sample frames/partial batches, noise reduction, resize reset, Stop, and mobile layout");
    } finally { await context.close(); }
};
