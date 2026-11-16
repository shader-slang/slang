const assert = require("node:assert/strict");

// Support modules must work even when Monaco cannot load and their extras stay closed.
exports.checkSdfModuleFallback = async function (browser, baseURL, sources) {
    const context = await browser.newContext({ viewport: { width: 1100, height: 850 } });
    try {
        await context.route("**/assets/editor/editor.js", route => route.abort());
        const page = await context.newPage();
        await page.goto(baseURL + "sdf-path-tracer/");
        const card = name => page.locator(`[data-snippet="${name}"]`);
        await page.waitForFunction(() => document.querySelector('[data-snippet="camera"] .snippet-language-status')
            .textContent.startsWith("Editor unavailable"));
        async function runMask() {
            await card("intersection").locator("textarea.snippet-editor").press("Control+Enter");
            await page.waitForFunction(() => /^(Finished|Could not run)/.test(
                document.querySelector('[data-snippet="intersection"] .snippet-status').textContent), null, { timeout: 90000 });
            assert.equal(await card("intersection").locator(".snippet-status").textContent(), "Finished",
                await card("intersection").locator(".snippet-output").textContent());
            return card("intersection").locator("canvas").evaluate(canvas => {
                const pixels = canvas.getContext("2d").getImageData(0, 0, canvas.width, canvas.height).data;
                let count = 0;
                for (let i = 0; i < pixels.length; i += 4) if (pixels[i] === 255) count++;
                return count;
            });
        }
        const original = await runMask();
        assert.ok(original > 0);
        await card("camera").evaluate(element => { element.closest("details").open = true; });
        assert.equal(await card("camera").locator(".snippet-run").isHidden(), true);
        await card("camera").locator("textarea.snippet-editor")
            .fill(sources["camera.slang"].replace("focalLength * forward", "(focalLength * 0.5) * forward"));
        await card("camera").locator("textarea.snippet-editor").press("Control+Enter");
        assert.equal(await card("camera").locator(".snippet-result").isHidden(), true,
            "A module's shortcut must not try to execute a nonexistent main");
        await card("camera").evaluate(element => { element.closest("details").open = false; });
        const edited = await runMask();
        assert.ok(edited < original * 0.35 && edited > original * 0.15,
            "Editing a collapsed camera module must change the consuming image");
        await card("camera").evaluate(element => { element.closest("details").open = true; });
        await card("camera").locator("textarea.snippet-editor").focus();
        await card("camera").locator(".snippet-reset").evaluate(element => element.scrollIntoView({ block: "center" }));
        await card("camera").locator(".snippet-reset").click();
        assert.equal(await card("camera").locator(".snippet-run").isHidden(), true);
        await card("camera").evaluate(element => { element.closest("details").open = false; });
        assert.equal(await runMask(), original, "Reset must restore the shared camera");
        console.log("PASS: collapsed support-module edits, Reset, and plain-text editor fallback");
    } finally { await context.close(); }
};
