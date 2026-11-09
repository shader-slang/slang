const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

exports.checkSdfTutorial = async function (browser, baseURL, outputDirectory) {
    const markdown = fs.readFileSync(path.join(__dirname, "../docs/heterogeneity/sdf-path-tracer.md"), "utf8");
    const sources = Object.fromEntries([...markdown.matchAll(/<!-- runnable: (\w+) canvas=16:9 -->\s*```(?:hlsl|slang)\r?\n([\s\S]*?)\r?\n```/g)]
        .map(([, name, text]) => [`${name}.slang`, text.replaceAll("\r\n", "\n").replaceAll(" //!FOLD", "")]));
    assert.equal(Object.keys(sources).length, 4);
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
            return card("blending").locator(".snippet-display canvas").evaluate(canvas => {
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
        for (const name of ["sphere", "tracing", "shading", "blending"]) {
            await card(name).locator(".snippet-monaco textarea").waitFor({ state: "attached" });
            assert.equal(await card(name).locator(".snippet-filename").textContent(), `${name}.slang`);
        }
        // The first cell's folded main is in view; longer cells virtualize it offscreen.
        await card("sphere").locator(".codicon-folding-collapsed").first().waitFor();

        // Run the last cell first: imports compile code without executing earlier mains.
        await run("blending");
        for (const name of ["sphere", "tracing", "shading"]) {
            assert.equal(await card(name).locator(".snippet-result").isVisible(), false);
        }
        const blended = await blendShape();
        assert.equal(blended.rowSegments, 1, "Blended spheres must form a connected silhouette");
        assert.ok(blended.neckPixels > 0, "The blended shape must have a visible neck");
        await card("blending").locator(".snippet-display canvas").screenshot({
            path: path.join(outputDirectory, "sdf-blending.png"),
        });
        await edit("blending", sources["blending.slang"].replace("blendWidth = 0.6", "blendWidth = 0.0"));
        await run("blending");
        const hardUnion = await blendShape();
        assert.equal(hardUnion.rowSegments, 1, "The default spheres overlap even without smoothing");
        assert.ok(blended.neckPixels > hardUnion.neckPixels * 1.25,
            "Smooth union must visibly fill out the join compared with an ordinary union");
        await edit("blending", sources["blending.slang"].replace("halfSeparation = 0.9", "halfSeparation = 1.2"));
        await run("blending");
        assert.equal((await blendShape()).rowSegments, 2, "Moving the spheres apart must break the neck");
        await reset("blending");

        await run("shading");
        assert.equal(await card("sphere").locator(".snippet-result").isVisible(), false);
        assert.equal(await card("tracing").locator(".snippet-result").isVisible(), false);
        const shaded = await pixels("shading");
        assert.ok(shaded.center[1] > shaded.center[0] + 30, "Sphere must be green");
        assert.ok(shaded.left[1] > shaded.right[1] + 25, "Upper-left side must face the light");
        await card("shading").screenshot({ path: path.join(outputDirectory, "sdf-shading.png") });
        await run("sphere");
        const slice = await pixels("sphere");
        assert.ok(slice.center[0] > slice.center[2] + 100, "Inside slice must be orange");
        assert.ok(slice.corner[2] > slice.corner[0] + 100, "Outside slice must be blue");
        await card("sphere").screenshot({ path: path.join(outputDirectory, "sdf-sphere.png") });
        await run("tracing");
        const mask = await pixels("tracing");
        assert.deepEqual(mask.center, [255, 255, 255, 255]);
        assert.deepEqual(mask.corner, [0, 0, 0, 255]);
        await card("tracing").screenshot({ path: path.join(outputDirectory, "sdf-tracing.png") });

        // Both execution and the visible language service must consume dependency edits.
        await edit("sphere", sources["sphere.slang"].replaceAll("sphereDistance", "ballDistance"));
        await card("tracing").locator(".squiggly-error").first().waitFor({ state: "attached", timeout: 90000 });
        assert.equal(await card("tracing").locator(".snippet-source-status").isVisible(), true);
        await edit("sphere", sources["sphere.slang"].replace("radius = 1.0", "radius = 0.7"));
        await page.waitForFunction(() => !document.querySelector('[data-snippet="tracing"] .squiggly-error'), null, { timeout: 90000 });
        await run("tracing");
        const smaller = await pixels("tracing");
        assert.ok(smaller.white < mask.white * 0.6 && smaller.white > mask.white * 0.35,
            "An edited imported radius must shrink the mask");
        assert.equal(await card("tracing").locator(".snippet-source-status").isVisible(), false);
        await reset("sphere");
        await run("tracing");
        assert.equal((await pixels("tracing")).white, mask.white, "Reset must restore imported code");

        await edit("shading", sources["shading.slang"].replace("float3(-0.6, 0.8, -1.0)", "float3(0.6, -0.8, -1.0)"));
        await run("shading");
        const movedLight = await pixels("shading");
        assert.ok(movedLight.right[1] > movedLight.left[1] + 25, "Changing the light must move the highlight");
        await reset("shading");

        const normalError = await page.evaluate(async ({ sources, baseURL }) => {
            const { init } = await import(new URL("assets/slangvm/src/index.js", baseURL));
            const { MemorySlangFileSystem, OverlaySlangFileSystem } = await import(new URL("assets/slangvm/src/file-system.js", baseURL));
            const { HTTPSlangFileSystem, standardLibraryURL } = await import(new URL("assets/slangvm/src/http-file-system.js", baseURL));
            const vm = await init({ backend: "webgpu", fileSystem: new OverlaySlangFileSystem([new MemorySlangFileSystem({
                ...sources,
                "check.slang": `import slang.runtime; import shading;
                    [host] float main() {
                        let errors = RWStructuredBuffer<float>::alloc(4);
                        Grid(4).dispatch((uint i) => {
                            float3 p = normalize(float3(0.2 + float(i), 0.4, -0.7));
                            errors[i] = length(surfaceNormal(p) - p);
                        });
                        return max(max(errors[0], errors[1]), max(errors[2], errors[3]));
                    }`,
            }), new HTTPSlangFileSystem(standardLibraryURL)]) });
            try { return await (await vm.load("check.slang")).main(); }
            finally { await vm.close(); }
        }, { sources, baseURL });
        assert.ok(normalError < 0.001, `Finite-difference normal error: ${normalError}`);
        await page.setViewportSize({ width: 390, height: 844 });
        assert.ok(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth), "Tutorial mobile overflow");
        assert.deepEqual(errors, []);
        console.log("PASS: SDF slice, mask, shaded sphere, smooth union/zero blend/separation, private mains, editable imports/IntelliSense, folds, reset, light direction, normals, and mobile layout");
    } finally { await context.close(); }
};
