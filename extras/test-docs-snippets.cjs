// Run after staging assets and `mkdocs build`: npm run test:snippets --prefix javascript/docs-editor
// Set CHROME_PATH to use a system Chrome/Edge instead of Playwright's bundled Chromium.
const assert = require("node:assert/strict");
const fs = require("node:fs");
const http = require("node:http");
const path = require("node:path");
const { createHash } = require("node:crypto");
const { createRequire } = require("node:module");
const { chromium } = createRequire(path.resolve(__dirname, "../javascript/docs-editor/package.json"))("playwright");
const { checkSdfTutorial } = require("./sdf-tutorial-checks.cjs");

(async () => {
    const root = path.resolve(__dirname, "../build/heterogeneity-docs");
    const manifest = JSON.parse(fs.readFileSync(path.join(root, "assets/runtime-manifest.json"), "utf8"));
    assert.equal(manifest.editor, true, "The published site must include the editor and language service");
    assert.ok(Object.keys(manifest.files).length, "The published site must include its runtime inventory");
    assert.ok(manifest.precompiled_modules?.length, "The published site must include precompiled standard modules");
    for (const name of manifest.precompiled_modules) {
        assert.ok(Object.hasOwn(manifest.files, name), name);
        assert.ok(Object.hasOwn(manifest.files, name.replace(/\.slang-module$/, ".slang")), name + " source");
    }
    for (const [name, digest] of Object.entries(manifest.files)) {
        assert.equal(createHash("sha256").update(fs.readFileSync(path.join(root, name))).digest("hex"), digest, name);
    }
    const prefix = "/preview/docs/"; // Exercise deployment below a URL prefix, too.
    const server = http.createServer((request, response) => {
        const pathname = decodeURIComponent(new URL(request.url, "http://localhost").pathname);
        const file = path.resolve(root, pathname.slice(prefix.length));
        if (!pathname.startsWith(prefix) || (file !== root && !file.startsWith(root + path.sep))) {
            response.writeHead(404).end();
            return;
        }
        const target = fs.existsSync(file) && fs.statSync(file).isDirectory() ? path.join(file, "index.html") : file;
        if (!fs.existsSync(target)) { response.writeHead(404).end(); return; }
        const mime = { ".js": "text/javascript", ".wasm": "application/wasm", ".html": "text/html", ".css": "text/css", ".json": "application/json" };
        response.setHeader("Content-Type", mime[path.extname(target)] || "application/octet-stream");
        fs.createReadStream(target).pipe(response);
    });
    await new Promise(resolve => server.listen(0, "127.0.0.1", resolve));
    let browser;
    try {
        browser = await chromium.launch({
            headless: true,
            executablePath: process.env.CHROME_PATH || undefined,
        });
        const baseURL = `http://127.0.0.1:${server.address().port}${prefix}`;
        // Only the execution worker rejects sources. The language worker must retain
        // them for IntelliSense. A successful Run now proves binary imports work,
        // rather than silently compiling sources after a bad precompiled-module load.
        async function requirePrecompiledModules(context) {
            await context.route("**/scripts/snippet-worker.js", async route => {
                const response = await route.fetch();
                const check = `
                    const originalFetch = globalThis.fetch.bind(globalThis);
                    globalThis.fetch = (input, options) => {
                        const url = new URL(input instanceof Request ? input.url : input, self.location.href);
                        if (url.pathname.includes('/assets/slangvm/slang/slang/') && url.pathname.endsWith('.slang')) {
                            throw new Error('Execution fell back to standard-module source: ' + url.pathname);
                        }
                        return originalFetch(input, options);
                    };
                `;
                await route.fulfill({ response, body: check + "\n" + await response.text() });
            });
        }
        const context = await browser.newContext({ viewport: { width: 1440, height: 1000 } });
        await requirePrecompiledModules(context);
        const requests = [];
        context.on("request", request => requests.push(request.url()));
        const missingRequests = [];
        context.on("response", response => {
            if (response.status() === 404 && response.url().includes("/assets/slangvm/slang/")) {
                missingRequests.push(response.url());
            }
        });
        const page = await context.newPage();
        const errors = [];
        page.on("pageerror", error => errors.push(error.message));
        assert.equal((await page.goto(baseURL)).status(), 200, "Site root must open below a URL prefix");
        await page.goto(baseURL + "grid-dispatch/");
        const card = page.locator('.slang-snippet[data-snippet="grid-dispatch"]');
        const editor = card.locator(".snippet-monaco textarea");
        const output = card.locator(".snippet-output");
        await editor.waitFor({ state: "attached" });
        assert.equal(requests.filter(url => url.includes("/assets/slangvm/")).length, 0, "Runtime fetched before Run");
        assert.equal(requests.filter(url => url.endsWith("slang-wasm.wasm")).length, 0, "slangd fetched before editor focus");
        const markdown = fs.readFileSync(path.join(__dirname, "../docs/heterogeneity/grid-dispatch.md"), "utf8");
        const original = markdown.match(/<!-- runnable: grid-dispatch -->\s*```slang\r?\n([\s\S]*?)\r?\n```/)[1].replaceAll("\r\n", "\n");
        async function edit(text) {
            await editor.focus();
            await editor.press("Control+Home");
            await editor.press("Control+A");
            // Use paste semantics: typing an entire program as one textInput event can trigger
            // Monaco's automatic closing bracket for the leading '[' in a [host] declaration.
            await editor.evaluate((element, source) => {
                const clipboardData = new DataTransfer();
                clipboardData.setData("text/plain", source);
                element.dispatchEvent(new ClipboardEvent("paste", { clipboardData, bubbles: true, cancelable: true }));
            }, text);
        }
        await page.waitForFunction(() => new Set(Array.from(document.querySelectorAll(".snippet-monaco .view-lines span"), node => node.className)).size > 2);
        await editor.focus();
        await page.waitForFunction(() => document.querySelector(".snippet-language-status").textContent !== "Loading IntelliSense…", null, { timeout: 90000 });
        assert.equal(await card.locator(".snippet-language-status").textContent(), "IntelliSense ready");
        assert.ok(requests.some(url => url.endsWith("slang-wasm.wasm")));
        assert.ok(requests.some(url => url.endsWith("/runtime/compute/grid.slang")));
        assert.ok(requests.some(url => url.endsWith("/runtime/core/simple_assert.slang")));

        // Ask the real WASM worker directly too: this exercises mapped FS imports and isolation
        // independently of whether Monaco chooses to show a suggestion widget.
        const languageResults = await page.evaluate(async () => {
            const configURL = new URL("../scripts/snippet-files.json", location.href);
            const mapping = (await (await fetch(configURL)).json())["grid-dispatch"];
            const files = Object.fromEntries(Object.entries(mapping).map(([path, url]) => [path, new URL(url, configURL).href]));
            const makeClient = () => {
                const worker = new Worker(new URL("../assets/editor/language.worker.js", location.href), { type: "module" });
                let id = 0;
                const pending = new Map();
                worker.onmessage = ({ data }) => {
                    const promise = pending.get(data.id);
                    pending.delete(data.id);
                    if (data.error) promise.reject(new Error(data.error)); else promise.resolve(data.result);
                };
                return { worker, call: (method, args) => new Promise((resolve, reject) => {
                    pending.set(++id, { resolve, reject }); worker.postMessage({ id, method, ...args });
                }) };
            };
            const first = makeClient();
            const second = makeClient();
            try {
                const source = "import slang.runtime.compute.grid;\n[host] void main() { let grid = Grid(8); grid. }";
                await first.call("initialize", { files, uri: "file:///example.slang", source });
                const items = await first.call("completion", { source, position: { line: 1, character: source.split("\n")[1].indexOf("grid.") + 5 }, context: { triggerKind: 1, triggerCharacter: "" } });
                const valid = "import slang.runtime.compute.grid;\n[host] void main() { let grid = Grid(8); }";
                const diagnostics = await first.call("diagnostics", { source: valid });
                const hover = await first.call("hover", { source: valid, position: { line: 1, character: valid.split("\n")[1].indexOf("Grid") + 1 } });
                const signatureSource = "[host] void main() { print(1); }";
                const signature = await first.call("signatureHelp", { source: signatureSource, position: { line: 0, character: signatureSource.indexOf("print(") + 6 } });
                const invalid = await first.call("diagnostics", { source: "[host] void main() { notDefined(); }" });
                const restored = await first.call("diagnostics", { source: valid });
                await second.call("initialize", { files: {}, uri: "file:///example.slang", source: valid });
                const missing = await second.call("diagnostics", { source: valid });
                return { labels: items.map(item => item.label), diagnostics, hover, signature, invalid, restored, missing };
            } finally { first.worker.terminate(); second.worker.terminate(); }
        });
        assert.ok(languageResults.labels.includes("dispatch"), JSON.stringify(languageResults.labels));
        assert.deepEqual(languageResults.diagnostics, []);
        assert.match(languageResults.hover?.contents.value || "", /Grid/);
        assert.ok(languageResults.signature?.signatures.length, "Signature help missing");
        assert.ok(languageResults.invalid.some(item => /notDefined/.test(item.message)));
        assert.deepEqual(languageResults.restored, []);
        assert.ok(languageResults.missing.some(item => /cannot|could not|unable/i.test(item.message)), "FS leaked between snippets");

        await edit("import slang.runtime.compute.grid;\n[host] void main() { let grid = Grid(8); grid.");
        await editor.press("Control+Space");
        await page.getByRole("option", { name: /dispatch/ }).first().waitFor({ timeout: 30000 });
        await editor.press("Enter");
        await page.waitForFunction(() => document.querySelector(".snippet-monaco .view-lines").textContent.includes("grid.dispatch"));
        await edit("[host] void main() { undefinedFunction(); }");
        await card.locator(".snippet-monaco .squiggly-error").first().waitFor({ timeout: 30000 });
        await card.locator(".snippet-reset").click();
        await page.waitForFunction(() => !document.querySelector(".snippet-monaco .squiggly-error"));
        async function run(expected, keyboard = false) {
            if (keyboard) await editor.press("Control+Enter");
            else await card.locator(".snippet-run").click();
            await page.waitForFunction(() => /^(Finished|Could not run)/.test(document.querySelector(".snippet-status").textContent), null, { timeout: 90000 });
            assert.equal(await card.locator(".snippet-status").textContent(), "Finished", await output.textContent());
            assert.equal((await output.textContent()).trim(), expected);
        }
        await run("513.000000");
        assert.ok(requests.some(url => url.endsWith("slangvm-wasm.wasm")));
        assert.ok(requests.some(url => url.endsWith("/runtime/compute/grid.slang-module")));
        const runtimeBinaryRequests = () => requests.filter(url =>
            /\/slang\/runtime(?:\/.*)?\.slang-module$/.test(url));
        const afterFirstPreload = runtimeBinaryRequests().length;
        assert.ok(afterFirstPreload > 1, "The page must preload runtime implementation modules");
        assert.deepEqual(missingRequests, [], "The first run must not fetch unknown import candidates");
        await edit(original.replace("scale = 2.0", "scale = 3.0"));
        await run("769.000000", true);
        assert.equal(runtimeBinaryRequests().length, afterFirstPreload,
            "A replacement worker must reuse preloaded runtime bytes without downloading them again");
        assert.deepEqual(missingRequests, [], "Fresh workers must reject unknown candidates without HTTP");
        await card.screenshot({ path: path.join(root, "../snippet-desktop.png") });

        // A second editor on the same page owns its source, worker, and result panel.
        const raw = page.locator('.slang-snippet[data-snippet="raw-dispatch"]');
        await raw.locator(".snippet-source").hover();
        await raw.locator(".snippet-run").click();
        await page.waitForFunction(() => document.querySelector('[data-snippet="raw-dispatch"] .snippet-status').textContent === "Finished", null, { timeout: 30000 });
        assert.equal((await raw.locator(".snippet-output").textContent()).trim(), "999");
        await raw.locator(".snippet-reset").click();
        assert.equal(await raw.locator(".snippet-result").isVisible(), false);
        assert.equal((await output.textContent()).trim(), "769.000000");

        await edit("[host] void main() { undefinedFunction(); }");
        await card.locator(".snippet-run").click();
        await page.waitForSelector(".snippet-error", { timeout: 90000 });
        assert.match(await output.textContent(), /undefinedFunction/);
        await card.locator(".snippet-reset").click();
        assert.equal(await output.isVisible(), false);
        await run("513.000000");

        await edit("[host] void main() { while (true) { print(1); } }");
        await card.locator(".snippet-run").click();
        await page.waitForFunction(() => /^(Running|Could not run)/.test(document.querySelector(".snippet-status").textContent), null, { timeout: 90000 });
        assert.equal(await card.locator(".snippet-status").textContent(), "Running…", await output.textContent());
        await card.locator(".snippet-stop").click();
        assert.equal(await card.locator(".snippet-status").textContent(), "Stopped");
        await card.locator(".snippet-reset").click();
        await run("513.000000");

        await edit('[host] void main() { for (int i = 0; i < 20000; ++i) print("0123456789"); }');
        await card.locator(".snippet-run").click();
        await page.waitForFunction(() => document.querySelector(".snippet-status").textContent === "Finished", null, { timeout: 90000 });
        assert.match(await output.textContent(), /Output limited/);
        assert.ok((await output.textContent()).length < 65700);
        await card.locator(".snippet-reset").click();
        await page.setViewportSize({ width: 390, height: 844 });
        assert.ok(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth), "Mobile overflow");
        await card.screenshot({ path: path.join(root, "../snippet-mobile.png") });
        assert.deepEqual(errors, []);
        await page.reload();
        await editor.waitFor({ state: "attached" });
        await editor.focus();
        await run("513.000000", true);
        assert.deepEqual(missingRequests, [], "A cold page must also reject unknown candidates without HTTP");
        console.log("PASS: zero missing import requests on first run, replacement workers, and reload");
        await context.close();

        // Exercise every packaged module, including tensor/numerics imports that
        // aren't used by the runnable documentation examples yet.
        const importsContext = await browser.newContext();
        try {
            const importsPage = await importsContext.newPage();
            await importsPage.goto(baseURL);
            const imported = await importsPage.evaluate(async modules => {
                const fetchFile = globalThis.fetch.bind(globalThis);
                globalThis.fetch = (input, options) => {
                    const url = new URL(input instanceof Request ? input.url : input, location.href);
                    if (url.pathname.includes('/assets/slangvm/slang/slang/') && url.pathname.endsWith('.slang')) {
                        throw new Error('Execution fell back to standard-module source: ' + url.pathname);
                    }
                    return fetchFile(input, options);
                };
                const { init } = await import(new URL('assets/slangvm/src/index.js', location.href));
                const sources = Object.fromEntries(modules.map((name, index) => [
                    `probe-${index}.slang`,
                    `import ${name.replace('assets/slangvm/slang/', '').replace(/\.slang-module$/, '').replaceAll('/', '.')};\n[host] int answer() { return 42; }`,
                ]));
                const { MemorySlangFileSystem, OverlaySlangFileSystem } = await import(new URL('assets/slangvm/src/file-system.js', location.href));
                const { HTTPSlangFileSystem, standardLibraryURL } = await import(new URL('assets/slangvm/src/http-file-system.js', location.href));
                const vm = await init({ fileSystem: new OverlaySlangFileSystem([
                    new MemorySlangFileSystem(sources), new HTTPSlangFileSystem(standardLibraryURL),
                ]) });
                try {
                    for (let index = 0; index < modules.length; ++index) {
                        const module = await vm.load(`probe-${index}.slang`);
                        if (await module.answer() !== 42) throw new Error(modules[index]);
                    }
                    return modules.length;
                } finally { await vm.close(); }
            }, manifest.precompiled_modules);
            assert.equal(imported, manifest.precompiled_modules.length);
            console.log(`PASS: all ${imported} precompiled standard modules import without sources`);
        } finally { await importsContext.close(); }

        async function errorCase(name, setup, expected) {
            const context = await browser.newContext();
            await setup(context);
            const page = await context.newPage();
            await page.goto(baseURL + "grid-dispatch/");
            const card = page.locator('.slang-snippet[data-snippet="grid-dispatch"]');
            await card.locator(".snippet-source").hover();
            await card.locator(".snippet-run").click();
            await page.waitForSelector(".snippet-error", { timeout: 90000 });
            assert.match(await card.locator(".snippet-output").textContent(), expected, name);
            assert.equal(await card.locator(".snippet-run").isEnabled(), true);
            await context.close();
        }
        const workerOverride = prefix => async context => {
            await context.route("**/scripts/snippet-worker.js", async route => {
                const response = await route.fetch();
                await route.fulfill({ response, body: prefix + "\n" + await response.text() });
            });
        };

        // Keep this inventory complete when docs gain, move, or remove runnable examples.
        await checkSdfTutorial(browser, baseURL, path.resolve(root, ".."));
        const examples = [
            ["invoking", "first-program", "5"],
            ["host-functions", "host-answer", "42"],
            ["host-functions", "allocate-resources", "Allocated one buffer and one texture."],
            ["resources", "host-element-access", "8.000000"],
            ["resources", "upload-buffer", "4.000000\n9.000000"],
            ["resources", "buffer-range-copy", "17\n23\n17\n23"],
            ["grid-dispatch", "grid-2d", null],
            ["grid-dispatch", "packed-captures", "255.000000"],
            ["grid-dispatch", "raw-dispatch", "999"],
            ["grid-dispatch", "specialized-dispatch", "7.000000"],
        ];
        const docs = path.resolve(__dirname, "../docs/heterogeneity");
        const documented = fs.readdirSync(docs).filter(name => name.endsWith(".md") && name !== "contributing.md")
            .flatMap(name => [...fs.readFileSync(path.join(docs, name), "utf8")
                .matchAll(/^<!-- runnable: ([a-z0-9-]+)(?: [^>]*)? -->/gm)]
                .map(match => `${name.slice(0, -3)}/${match[1]}`));
        assert.deepEqual(documented.sort(), ["grid-dispatch/grid-dispatch",
            "sdf-path-tracer/sphere", "sdf-path-tracer/tracing", "sdf-path-tracer/shading", "sdf-path-tracer/blending",
            ...examples.map(([route, name]) => `${route}/${name}`)].sort(), "Every runnable example needs browser coverage");
        for (const [route, name, expected] of examples) {
            const context = await browser.newContext();
            try {
                if (name === "first-program") {
                    await workerOverride('Object.defineProperty(navigator, "gpu", {value: undefined});')(context);
                }
                const page = await context.newPage();
                const pageErrors = [];
                const failedAssets = [];
                page.on("pageerror", error => pageErrors.push(error.message));
                page.on("response", response => {
                    const relative = new URL(response.url()).pathname.slice(prefix.length);
                    // Import lookup probes several paths and compiled-module spellings before
                    // finding the module. A 404 is expected for candidates outside the package inventory.
                    const missingCandidate = response.status() === 404 &&
                        relative.startsWith("assets/slangvm/slang/") && !Object.hasOwn(manifest.files, relative);
                    if (response.status() >= 400 && !missingCandidate) {
                        failedAssets.push(`${response.status()} ${response.url()}`);
                    }
                });
                await page.goto(baseURL + route + "/");
                const card = page.locator(`[data-snippet="${name}"]`);
                assert.equal(await card.locator(".snippet-result").isVisible(), false);
                await card.locator(".snippet-monaco textarea").focus();
                await page.waitForFunction(name => /ready|unavailable/.test(document.querySelector(`[data-snippet="${name}"] .snippet-language-status`).textContent), name, { timeout: 90000 });
                assert.equal(await card.locator(".snippet-language-status").textContent(), "IntelliSense ready", name);
                if (name !== "first-program") await requirePrecompiledModules(context);
                await card.locator(".snippet-run").click();
                await page.waitForFunction(name => /^(Finished|Could not run)/.test(document.querySelector(`[data-snippet="${name}"] .snippet-status`).textContent), name, { timeout: 90000 });
                assert.equal(await card.locator(".snippet-status").textContent(), "Finished", await card.locator(".snippet-output").textContent());
                if (expected === null) {
                    const pixels = await card.locator(".snippet-display canvas").evaluate(canvas => {
                        const ctx = canvas.getContext("2d");
                        return [0.25, 0.75].map(fraction => Array.from(ctx.getImageData(
                            Math.floor(canvas.width * fraction), Math.floor(canvas.height * fraction), 1, 1).data));
                    });
                    assert.ok(pixels[1][0] - pixels[0][0] > 80, "Canvas must contain the red gradient");
                    assert.ok(pixels[1][1] - pixels[0][1] > 80, "Canvas must contain the green gradient");
                    assert.equal(pixels[0][3], 255, "Canvas must retain the rendered frame after the VM closes");
                    await card.screenshot({ path: path.join(root, "../snippet-canvas.png") });
                } else {
                    assert.equal((await card.locator(".snippet-output").textContent()).trim(), expected, name);
                }
                assert.equal(await card.locator(".squiggly-error").count(), 0, name + " language diagnostics");
                await card.locator(".snippet-reset").click();
                assert.equal(await card.locator(".snippet-result").isVisible(), false);
                assert.deepEqual(pageErrors, []);
                assert.deepEqual(failedAssets, [], "All runtime and editor assets must be served successfully");
                console.log(`PASS: ${name}: ${expected ?? "canvas gradient"}`);
            } finally { await context.close(); }
        }

        await errorCase("WebGPU unavailable", workerOverride('Object.defineProperty(navigator, "gpu", {value: undefined});'), /WebGPU is unavailable/);
        await errorCase("JSPI unavailable", workerOverride('WebAssembly.promising = undefined;'), /WebAssembly JSPI/);
        await errorCase("GPU initialization fails", workerOverride('navigator.gpu.requestAdapter = async () => null;'), /Initializing compiler and WebGPU/);
        await errorCase("Runtime package missing", context => context.route("**/assets/slangvm/src/index.js", route => route.fulfill({ status: 404, body: "missing" })), /runtime assets may be missing/);
        await errorCase("WASM download fails", context => context.route("**/slangvm-wasm.wasm", route => route.abort()), /Initializing compiler and WebGPU/);

        for (const failedURL of ["**/assets/slangd/slang-wasm.wasm", "**/runtime/compute/grid.slang"]) {
            const context = await browser.newContext();
            await context.route(failedURL, route => route.fulfill({ status: 404, body: "missing" }));
            const page = await context.newPage();
            await page.goto(baseURL + "grid-dispatch/");
            const card = page.locator('.slang-snippet[data-snippet="grid-dispatch"]');
            await card.locator(".snippet-monaco textarea").focus();
            await page.waitForSelector(".snippet-language-error", { timeout: 90000 });
            assert.match(await card.locator(".snippet-language-status").textContent(), /IntelliSense unavailable/);
            assert.equal(await card.locator(".snippet-run").isEnabled(), true);
            await context.close();
        }
        const fallbackContext = await browser.newContext();
        await fallbackContext.route("**/assets/editor/editor.js", route => route.fulfill({ status: 404, body: "missing" }));
        const fallbackPage = await fallbackContext.newPage();
        await fallbackPage.goto(baseURL + "grid-dispatch/");
        await fallbackPage.waitForSelector(".snippet-language-error");
        assert.equal(await fallbackPage.locator('.slang-snippet[data-snippet="grid-dispatch"] textarea.snippet-editor').inputValue(), original);
        await fallbackContext.close();

        const plain = await browser.newContext({ javaScriptEnabled: false });
        const plainPage = await plain.newPage();
        await plainPage.goto(baseURL + "grid-dispatch/");
        assert.match(await plainPage.locator(".codehilite").first().textContent(), /float scale = 2.0/);
        await plain.close();
        console.log(`PASS: all ${documented.length} examples, precompiled standard modules without source fallback, source-based IntelliSense, CPU execution without WebGPU, canvas output, independent editors, run/edit/reset, Stop/retry, mobile, and loading/browser fallbacks.`);
    } finally {
        await browser?.close();
        server.close();
    }
})().catch(error => { console.error(error); process.exitCode = 1; });
