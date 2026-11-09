// Exercise the actual preview server, including its HTTP behavior. Unlike the snippet
// regression test, this does not substitute a Node static server or intercept requests.
// Usage: node extras/test-docs-loading.cjs [http://127.0.0.1:8000/grid-dispatch/]
const assert = require("node:assert/strict");
const { chromium } = require("playwright");

(async () => {
    const browser = await chromium.launch({
        headless: true, executablePath: process.env.CHROME_PATH || undefined,
    });
    try {
        // Each context starts with an empty browser cache; reloads also exercise warm loads.
        for (let session = 0; session < 3; session++) {
            const context = await browser.newContext();
            const pending = new Map();
            context.on("request", request => pending.set(request, { url: request.url(), start: Date.now() }));
            context.on("requestfinished", request => pending.delete(request));
            context.on("requestfailed", request => pending.delete(request));
            try {
                const page = await context.newPage();
                const card = page.locator('.slang-snippet[data-snippet="grid-dispatch"]');
                const errors = [];
                page.on("pageerror", error => errors.push(error.message));
                for (let reload = 0; reload < 3; reload++) {
                    const start = Date.now();
                    await page.goto(process.argv[2] || "http://127.0.0.1:8000/grid-dispatch/", {
                        waitUntil: "domcontentloaded", timeout: 30000,
                    });
                    await card.locator(".snippet-monaco textarea").focus({ timeout: 30000 });
                    await card.locator(".snippet-run").click();
                    await page.waitForFunction(() => {
                        const language = document.querySelector(".snippet-language-status").textContent;
                        const runtime = document.querySelector(".snippet-status").textContent;
                        return /ready|unavailable/.test(language) && /Finished|Could not/.test(runtime);
                    }, null, { timeout: 30000 }).catch(async error => {
                        console.error(await card.locator(".snippet-language-status").textContent(),
                            await card.locator(".snippet-status").textContent());
                        console.error([...pending.values()].map(request => ({
                            url: request.url, waitingMilliseconds: Date.now() - request.start,
                        })));
                        throw error;
                    });
                    assert.equal(await card.locator(".snippet-language-status").textContent(), "IntelliSense ready");
                    assert.equal(await card.locator(".snippet-status").textContent(), "Finished",
                        await card.locator(".snippet-output").textContent());
                    assert.equal((await card.locator(".snippet-output").textContent()).trim(), "513.000000");
                    console.log(`Session ${session + 1}, load ${reload + 1}: ${Date.now() - start} ms`);
                }
                assert.deepEqual(errors, []);
            } finally {
                await context.close();
            }
        }
        console.log("PASS: nine cold/warm loads against the running preview server.");
    } finally {
        await browser.close();
    }
})().catch(error => { console.error(error); process.exitCode = 1; });
