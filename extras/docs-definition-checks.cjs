const assert = require("node:assert/strict");
const { readSdfTutorial } = require("./sdf-tutorial-sources.cjs");

// Exercise the real language worker and Monaco gestures, without editor test hooks.
exports.checkDocsDefinitions = async function (browser, baseURL) {
    const context = await browser.newContext({ viewport: { width: 1300, height: 1000 } });
    try {
        const page = await context.newPage();
        const errors = [];
        page.on("pageerror", error => errors.push(error.message));
        const { sources } = readSdfTutorial();
        await page.goto(baseURL + "sdf-path-tracer/");
        const card = name => page.locator(`[data-snippet="${name}"]`);
        const input = name => card(name).locator(".snippet-monaco textarea");
        async function open(name) {
            await card(name).evaluate(element => {
                for (let parent = element.parentElement; parent; parent = parent.parentElement) {
                    if (parent.matches("details")) parent.open = true;
                }
            });
            await input(name).focus();
        }
        async function at(name, needle, offset = 0) {
            const source = sources[`${name}.slang`];
            const index = source.indexOf(needle);
            assert.ok(index >= 0, `${name}: ${needle}`);
            const before = source.slice(0, index + offset).split("\n");
            await open(name);
            await input(name).press("Control+k");
            await input(name).press("Control+j"); // Unfold the source before walking its lines.
            await input(name).press("Control+Home");
            for (let i = 1; i < before.length; i++) await input(name).press("ArrowDown");
            await input(name).press("Home");
            await input(name).press("Home"); // Toggle past indentation back to column one.
            for (let i = 0; i < before.at(-1).length; i++) await input(name).press("ArrowRight");
            await page.waitForFunction(name => /ready|unavailable/.test(
                document.querySelector(`[data-snippet="${name}"] .snippet-language-status`).textContent),
                name, { timeout: 90000 });
            assert.equal(await card(name).locator(".snippet-language-status").textContent(), "IntelliSense ready");
            assert.ok(await input(name).evaluate((element, text) =>
                element.value.slice(element.selectionStart).startsWith(text), needle.slice(offset)),
                `${name}: cursor must be at ${needle.slice(offset)}`);
        }
        async function destination(name, symbol) {
            await page.waitForFunction(({ name, symbol }) => {
                const element = document.activeElement;
                return element.closest("[data-snippet]")?.dataset.snippet === name &&
                    element.value?.slice(element.selectionStart).startsWith(symbol);
            }, { name, symbol }, { timeout: 30000 }).catch(async error => {
                console.error("Definition destination", name, symbol, await page.evaluate(() => ({
                    focused: document.activeElement.closest("[data-snippet]")?.dataset.snippet,
                    text: document.activeElement.value?.slice(document.activeElement.selectionStart),
                    element: document.activeElement.outerHTML.slice(0, 300),
                })));
                await page.screenshot({ path: require("node:path").resolve(__dirname, "../build/docs-definition-failure.png") });
                throw error;
            });
            assert.equal(await card(name).evaluate(element => {
                for (let parent = element.parentElement; parent; parent = parent.parentElement) {
                    if (parent.matches("details") && !parent.open) return false;
                }
                const bounds = element.querySelector(".snippet-monaco").getBoundingClientRect();
                return bounds.top >= 50 && bounds.bottom <= window.innerHeight;
            }), true, `${name}: destination must be open and in view`);
        }
        async function jump(name, needle, target, symbol, offset = 0) {
            await at(name, needle, offset);
            await input(name).press("F12");
            await destination(target, symbol);
        }
        async function cursorPoint(name) {
            await card(name).locator(".snippet-monaco").evaluate(element => element.scrollIntoView({ block: "center" }));
            const bounds = await card(name).locator(".cursors-layer .cursor").first().boundingBox();
            assert.ok(bounds);
            return { x: bounds.x + 4, y: bounds.y + bounds.height / 2 };
        }
        async function edit(name, source) {
            await open(name);
            await input(name).press("Control+A");
            await input(name).evaluate((element, source) => {
                const clipboardData = new DataTransfer();
                clipboardData.setData("text/plain", source);
                element.dispatchEvent(new ClipboardEvent("paste", { clipboardData, bubbles: true, cancelable: true }));
            }, source);
            sources[`${name}.slang`] = source;
        }
        async function selectedText(name) {
            // The accessibility textarea abbreviates long selections. Copy through
            // Monaco into an isolated DataTransfer, without touching the OS clipboard.
            return input(name).evaluate(element => {
                const clipboardData = new DataTransfer();
                element.dispatchEvent(new ClipboardEvent("copy", { clipboardData, bubbles: true, cancelable: true }));
                return clipboardData.getData("text/plain").replaceAll("\r\n", "\n");
            });
        }

        await jump("sphere-sdf", "import sdf;", "sdf", "public interface", 9);
        await jump("blending", 'import "sphere-tracer";', "sphere-tracer", "import", 10);
        await jump("simple-sphere-tracer", "sphereDistance(hitPosition,", "sphere-distance", "sphereDistance");
        await jump("blending", "BlendedSDF : ISDF", "sdf", "ISDF", "BlendedSDF : ".length);
        await jump("blending", "SphereSDF left", "sphere-sdf", "SphereSDF");
        // There are two sphereTrace definitions in the tutorial. Resolve the imported
        // generic overload, rather than choosing a cell by matching the symbol text.
        await jump("blending", "sphereTrace(shape", "sphere-tracer", "sphereTrace");

        assert.equal(await card("camera").evaluate(element => element.closest("details").open), false);
        await at("intersection", "camera.direction(", "camera.".length);
        const point = await cursorPoint("intersection");
        await page.mouse.click(point.x, point.y);
        assert.equal(await page.evaluate(() => document.activeElement.closest("[data-snippet]")?.dataset.snippet),
            "intersection", "Plain click must place the cursor without navigating");
        await page.keyboard.down("Control");
        try { await page.mouse.click(point.x, point.y); }
        finally { await page.keyboard.up("Control"); }
        await destination("camera", "direction");

        await at("intersection", "runFrames(w");
        const menuPoint = await cursorPoint("intersection");
        await page.mouse.click(menuPoint.x, menuPoint.y, { button: "right" });
        // Monaco intentionally ignores mouse-up for the first 100 ms of a menu's life.
        await page.getByRole("menuitem", { name: /^Go to Definition/ }).click({ delay: 150 });
        await destination("frame-loop", "runFrames");

        // A declaration inside a folded struct must be revealed, too.
        await open("ggx-sampling");
        await input("ggx-sampling").press("Control+Home");
        await input("ggx-sampling").press("Control+k");
        await input("ggx-sampling").press("Control+0");
        await card("ggx-sampling").locator(".codicon-folding-collapsed").first().waitFor();
        await jump("coated-diffuse", "specular.evaluate(normal", "ggx-sampling", "evaluate", "specular.".length);
        assert.ok(await card("ggx-sampling").locator(".view-line").filter({ hasText: "public float3 evaluate(" }).count() > 0,
            "The method declaration must be visible inside its unfolded struct");

        // Renaming and shifting a declaration must affect other workers immediately.
        const edited = "// Navigation follows live edits.\n\n" + sources["camera.slang"]
            .replace("public float3 direction", "/* π 🪟 */ public float3 editedDirection");
        await edit("camera", edited);
        await card("camera").evaluate(element => { element.closest("details").open = false; });
        await at("intersection", "camera.direction(", "camera.".length);
        await input("intersection").press("F12");
        await card("intersection").getByText(/No definition found for 'direction'/).waitFor();
        assert.equal(await card("camera").evaluate(element => element.closest("details").open), false,
            "An old symbol name must not navigate to a stale definition");
        await input("intersection").press("Escape");
        await edit("intersection", sources["intersection.slang"].replace("camera.direction", "camera.editedDirection"));
        await jump("intersection", "camera.editedDirection(", "camera", "editedDirection", "camera.".length);
        await input("camera").press("Control+z");
        sources["camera.slang"] = readSdfTutorial().sources["camera.slang"];
        await input("camera").press("Control+A");
        assert.equal(await selectedText("camera"), sources["camera.slang"], "Destination undo history must survive the jump");
        await open("intersection");
        await input("intersection").press("Control+z");
        await input("intersection").press("Control+A");
        assert.equal(await selectedText("intersection"), readSdfTutorial().sources["intersection.slang"],
            "Source contents and undo history must remain in their original editor");

        // Imported files without an open cell also need the destination's text for
        // UTF-16 column conversion. This exercises the language server's fallback.
        const library = "// Imported library\n\n/* π 🪟 */ public float importedValue(float p) { return p; }\n";
        const source = "import library;\nfloat caller() { return importedValue(1.0); }\n";
        const locations = await page.evaluate(async ({ baseURL, library, source }) => {
            const worker = new Worker(new URL("assets/editor/language.worker.js", baseURL), { type: "module" });
            let id = 0;
            function request(method, args) {
                return new Promise((resolve, reject) => {
                    const timeout = setTimeout(() => reject(new Error("Definition worker timed out")), 90000);
                    worker.onmessage = ({ data }) => {
                        clearTimeout(timeout);
                        if (data.error) reject(new Error(data.error));
                        else resolve(data.result);
                    };
                    worker.onerror = event => { clearTimeout(timeout); reject(new Error(event.message)); };
                    worker.postMessage({ id: ++id, method, ...args });
                });
            }
            try {
                await request("initialize", { uri: "file:///caller.slang", source,
                    files: { "/library.slang": "data:text/plain," + encodeURIComponent(library) } });
                return await request("definition", { source,
                    position: { line: 1, character: source.split("\n")[1].indexOf("importedValue") + 1 } });
            } finally { worker.terminate(); }
        }, { baseURL, library, source });
        const column = library.split("\n")[2].indexOf("importedValue");
        assert.deepEqual(locations, [{ uri: "file:///library.slang", range: {
            start: { line: 2, character: column }, end: { line: 2, character: column + "importedValue".length },
        } }], "Closed imports must use the definition's UTF-16 coordinates");
        assert.deepEqual(errors, []);
        console.log("PASS: cross-cell definitions, imports, generic functions, types, Ctrl+click, context menu, folded targets, live edits, and undo");
    } finally { await context.close(); }
};
