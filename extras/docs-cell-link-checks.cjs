const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");
const { execFileSync } = require("node:child_process");
const repo = path.resolve(__dirname, "..");

// Render a dedicated Markdown fixture through the real docs hook, never alter tutorial prose.
function fixtureHTML() {
    const localPython = path.join(repo, ".temp/docs-venv", process.platform === "win32" ? "Scripts/python.exe" : "bin/python");
    const python = process.env.DOCS_PYTHON || (fs.existsSync(localPython) ? localPython : "python");
    const body = execFileSync(python, ["-c",
        "from extras.tests.test_docs_cell_links import render; from pathlib import Path; " +
        "print(render(Path('extras/tests/fixtures/docs-cell-links.md').read_text(encoding='utf-8'))[0])"],
        { cwd: repo, encoding: "utf8" });
    return `<!doctype html><html><head><meta charset="utf-8">
        <link rel="stylesheet" href="../stylesheets/snippets.css">
        <style>body { max-width: 850px; margin: 40px auto; font: 16px/1.5 sans-serif; }
        details { margin: 20px 0; } a { margin-right: 12px; }</style></head>
        <body class="md-typeset">${body}<script src="../scripts/snippets.js"></script></body></html>`;
}

exports.checkDocsCellLinks = async function (browser, baseURL) {
    const html = fixtureHTML();
    const fixtureURL = baseURL + "cell-links-fixture/";
    async function context(options = {}) {
        const result = await browser.newContext({ viewport: { width: 1200, height: 1000 }, ...options });
        await result.route(fixtureURL, route => route.fulfill({ contentType: "text/html", body: html }));
        await result.route("**/scripts/snippet-files.json", route => route.fulfill({
            contentType: "application/json", body: JSON.stringify({ "link-types": {}, "link-caller": {} }),
        }));
        return result;
    }
    const primary = await context();
    try {
        const page = await primary.newPage();
        const errors = [];
        const requests = [];
        page.on("pageerror", error => errors.push(error.message));
        page.on("request", request => requests.push(request.url()));
        await page.goto(fixtureURL);
        const card = name => page.locator(`[data-snippet="${name}"]`);
        const input = name => card(name).locator(".snippet-monaco textarea");
        const link = name => page.getByRole("link", { name, exact: true });
        const status = card("link-types").locator(".snippet-navigation-status");
        async function selected(name, symbol) {
            await page.waitForFunction(({ name, symbol }) => {
                const focused = document.activeElement;
                return focused.closest("[data-snippet]")?.dataset.snippet === name &&
                    (symbol ? focused.value?.slice(focused.selectionStart, focused.selectionEnd).startsWith(symbol) :
                        focused.selectionStart === focused.selectionEnd);
            }, { name, symbol }, { timeout: 90000 }).catch(async error => {
                console.error("Cell navigation", await page.locator(".snippet-navigation").allTextContents());
                console.error("Language status", await page.locator(".snippet-language-status").allTextContents(), errors);
                throw error;
            });
        }
        async function edit(source) {
            await input("link-types").focus();
            await input("link-types").press("Control+A");
            await input("link-types").evaluate((element, source) => {
                const clipboardData = new DataTransfer();
                clipboardData.setData("text/plain", source);
                element.dispatchEvent(new ClipboardEvent("paste", { clipboardData, bubbles: true, cancelable: true }));
            }, source);
        }
        await input("link-caller").waitFor({ state: "attached" });
        assert.equal(await page.locator("details").getAttribute("open"), null);
        assert.equal(requests.some(url => url.endsWith("slang-wasm.wasm")), false, "No symbol lookup before navigation");
        await link("Function").click();
        await selected("link-types", "target");
        assert.notEqual(await page.locator("details").getAttribute("open"), null);
        await link("Constant").click();
        await selected("link-types", "setting");
        await link("Interface").click();
        await selected("link-types", "IShape");
        await link("Generic").click();
        await selected("link-types", "identity");
        await input("link-types").press("Control+k");
        await input("link-types").press("Control+0");
        await card("link-types").locator(".codicon-folding-collapsed").first().waitFor();
        await link("Member").click();
        await selected("link-types", "single");
        assert.ok(await card("link-types").locator(".view-line").filter({ hasText: "public float single" }).count());
        await link("Overloads").click();
        await card("link-types").locator(".snippet-navigation-choices button").first().waitFor();
        const choices = card("link-types").locator(".snippet-navigation-choices button");
        assert.equal(await choices.count(), 2);
        assert.notEqual(await choices.nth(0).textContent(), await choices.nth(1).textContent());
        await choices.nth(1).click();
        await selected("link-types", "work");
        await page.screenshot({ path: path.join(repo, "build/docs-cell-links.png") });
        await link("Function").click();
        await selected("link-types", "target");
        await link("Caller").click();
        await selected("link-caller", "main");
        await page.goBack();
        await selected("link-types", "target");
        await page.goForward();
        await selected("link-caller", "main");
        await link("Whole cell").click();
        await selected("link-types", "");
        await input("link-types").press("Control+A");
        const original = await input("link-types").evaluate(element => {
            const clipboardData = new DataTransfer();
            element.dispatchEvent(new ClipboardEvent("copy", { clipboardData, bubbles: true, cancelable: true }));
            return clipboardData.getData("text/plain");
        });
        await edit("// Added a line above the definitions.\n\n" + original);
        await link("Function").click();
        await selected("link-types", "target");
        await edit(original.replace("float target(", "float renamed("));
        await link("Function").click(); // Same URL must resolve again after editing.
        await page.waitForFunction(() => document.querySelector(".snippet-navigation-status")
            .textContent.includes("not defined"));
        assert.match(await status.textContent(), /target is not defined/);
        await edit(original);
        await link("Missing").click();
        await page.waitForFunction(() => document.querySelector(".snippet-navigation-status")
            .textContent.includes("removed is not defined"));
        await link("Member").click();
        await selected("link-types", "single");
        const sharedURL = page.url();
        await page.reload();
        await selected("link-types", "single");
        assert.equal(page.url(), sharedURL);
        assert.equal(await page.locator(".snippet-result:not([hidden])").count(), 0);
        assert.equal(requests.some(url => url.endsWith("slangvm-wasm.wasm")), false, "Navigation must never execute code");
        assert.deepEqual(errors, []);
        console.log("Cell links: functions, constants, interfaces, members, overloads, folds, edits, history and shared URLs passed");
    } finally { await primary.close(); }

    const fallback = await context();
    try {
        await fallback.route("**/assets/editor/editor.js", route => route.abort());
        const page = await fallback.newPage();
        await page.goto(fixtureURL + "#cell-link-types.slang--target");
        await page.waitForFunction(() => document.activeElement.matches("textarea.snippet-editor"));
        assert.notEqual(await page.locator("details").getAttribute("open"), null);
        assert.match(await page.locator('[data-snippet="link-types"] .snippet-navigation-status').textContent(), /unavailable/);
        console.log("Cell links: plain-text editor fallback passed");
    } finally { await fallback.close(); }
};

if (require.main === module) {
    const { chromium } = require(path.join(repo, "javascript/docs-editor/node_modules/playwright"));
    (async () => {
        const root = path.join(repo, "build/heterogeneity-docs");
        const server = require("node:http").createServer((request, response) => {
            const url = new URL(request.url, "http://localhost");
            let file = path.resolve(root, "." + decodeURIComponent(url.pathname));
            if (!file.startsWith(root + path.sep)) { response.writeHead(404).end(); return; }
            if (fs.existsSync(file) && fs.statSync(file).isDirectory()) file = path.join(file, "index.html");
            if (!fs.existsSync(file)) { response.writeHead(404).end(); return; }
            const mime = { ".js": "text/javascript", ".css": "text/css", ".html": "text/html", ".json": "application/json", ".wasm": "application/wasm" };
            response.setHeader("Content-Type", mime[path.extname(file)] || "application/octet-stream");
            fs.createReadStream(file).pipe(response);
        });
        await new Promise(resolve => server.listen(0, "127.0.0.1", resolve));
        const browser = await chromium.launch({ headless: true, executablePath: process.env.CHROME_PATH || undefined });
        try {
            const baseURL = `http://127.0.0.1:${server.address().port}/`;
            await exports.checkDocsCellLinks(browser, baseURL);
            if (process.argv.includes("--definitions")) {
                await require("./docs-definition-checks.cjs").checkDocsDefinitions(browser, baseURL);
            }
        }
        finally { await browser.close(); await new Promise(resolve => server.close(resolve)); }
    })().catch(error => { console.error(error); process.exitCode = 1; });
}
