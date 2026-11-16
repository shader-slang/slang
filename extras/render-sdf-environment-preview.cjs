// Render the bundled EXR using the tutorial's display transform, without duplicating it.
const fs = require("node:fs");
const path = require("node:path");
const http = require("node:http");
const { createRequire } = require("node:module");
const repo = path.resolve(__dirname, "..");
const { chromium } = createRequire(path.join(repo, "javascript/docs-editor/package.json"))("playwright");
const docs = path.join(repo, "docs/heterogeneity");
const runtime = path.resolve(repo, process.env.HSLANG_DOCS_RUNTIME_DIR || "javascript/docs-runtime");
const comparison = process.argv.includes("--ggx-comparison");
const { sources } = require("./sdf-tutorial-sources.cjs").readSdfTutorial();
sources["preview.slang"] = `import slang.runtime; import "environment-map"; import display;
    [host] void main(Window w) {
        let environmentMap = loadEnvironment("/relax_inn_seaview_suite_front_1k.exr");
        uint width, height;
        environmentMap.GetDimensions(width, height);
        let image = RWTexture2D<float4>::alloc(width, height);
        Grid(uint2(width, height)).dispatch<8, 8>((uint2 p) => {
            image[p] = displayColor(environmentMap[p].rgb, 1.0);
        });
        let surface = createSurface<TextureFormat::rgba8Unorm>(w);
        let presented = acquireSurfaceTexture(surface);
        blitTexture2D(presented, image);
        presentSurface(surface);
    }`;
if (comparison) sources["preview.slang"] = `import slang.runtime; import "environment-map"; import "ggx-sampling";
    import scene; import "path-tracer"; import "sphere-sdf"; import "sphere-tracer"; import camera; import prng;
    import "progressive-renderer";
    [host] void main(Window w) {
        let environmentMap = loadEnvironment("/relax_inn_seaview_suite_front_1k.exr");
        let camera = Camera(float3(0, 0, -3), float3(0), 1.5);
        let sphere = SphereSDF(float3(0), 1.0);
        let tracing = TraceSettings(0.001, 10.0, 96);
        runProgressive(w, (uint2 p, uint2 resolution, uint index) => {
            uint panel = p.x / 512u;
            uint2 pixel = uint2(p.x % 512u, p.y);
            float roughness = panel == 0u ? 0.1 : (panel == 1u ? 0.35 : 0.7);
            uint state = pixelSeed(pixel, uint2(512u), index);
            float3 direction = camera.direction(float2(pixel) + 0.5, uint2(512u));
            let scene = SingleMaterialScene<SphereSDF, GGXBRDF>(sphere, GGXBRDF(roughness, float3(0.9)));
            return tracePath(scene, environmentMap, camera.position, direction,
                state, 1u, tracing, 0.002, 0.004);
        }, 1.0, 32u, 4096u);
    }`;

const server = http.createServer((request, response) => {
    const pathname = decodeURIComponent(new URL(request.url, "http://localhost").pathname);
    if (pathname === "/") {
        response.setHeader("Content-Type", "text/html");
        response.end(`<!doctype html><canvas width="${comparison ? 1536 : 1024}" height="512"></canvas>`);
        return;
    }
    const root = pathname.startsWith("/assets/") ? runtime : docs;
    const file = path.resolve(root, "." + pathname);
    if (!file.startsWith(root + path.sep) || !fs.existsSync(file) || !fs.statSync(file).isFile()) {
        response.writeHead(404).end();
        return;
    }
    response.setHeader("Content-Type", { ".js": "text/javascript", ".wasm": "application/wasm" }[path.extname(file)] || "application/octet-stream");
    fs.createReadStream(file).pipe(response);
});

(async () => {
    await new Promise(resolve => server.listen(0, "127.0.0.1", resolve));
    let browser;
    try {
        browser = await chromium.launch({ headless: true, executablePath: process.env.CHROME_PATH || undefined });
        const page = await browser.newPage();
        await page.goto(`http://127.0.0.1:${server.address().port}/`);
        const png = await page.evaluate(async ({ sources, comparison }) => {
            const { init } = await import("/assets/slangvm/src/index.js");
            const { MemorySlangFileSystem, OverlaySlangFileSystem } = await import("/assets/slangvm/src/file-system.js");
            const { preloadRuntime } = await import("/scripts/snippet-filesystem.js");
            const mappingURL = new URL("/scripts/snippet-files.json", location.href);
            const mapping = (await (await fetch(mappingURL)).json()).environment;
            const files = Object.fromEntries(Object.entries(mapping).map(([name, url]) => [name, new URL(url, mappingURL).href]));
            const { preloadedFiles } = await preloadRuntime(files);
            let result;
            const vm = await init({ backend: "webgpu", fileSystem: new OverlaySlangFileSystem([
                new MemorySlangFileSystem(sources), new MemorySlangFileSystem(preloadedFiles),
            ]), onPresent: canvas => {
                const snapshot = document.createElement("canvas");
                snapshot.width = canvas.width;
                snapshot.height = canvas.height + (comparison ? 56 : 0);
                const context = snapshot.getContext("2d");
                context.drawImage(canvas, 0, 0);
                if (comparison) {
                    context.fillStyle = "#26382f";
                    context.font = "26px Segoe UI, sans-serif";
                    context.textAlign = "center";
                    ["0.1", "0.35", "0.7"].forEach((value, index) =>
                        context.fillText(`Roughness ${value}`, 256 + index * 512, canvas.height + 38));
                }
                result = snapshot.toDataURL("image/png").split(",")[1];
            } });
            try {
                const window = await vm.createWindow(document.querySelector("canvas"));
                await (await vm.load("preview.slang")).main(window);
                if (!result) throw new Error("The environment preview did not present an image");
                return result;
            } finally { await vm.close(); }
        }, { sources, comparison });
        const output = comparison ? "sdf-ggx-roughness.png" : "sdf-environment.png";
        fs.writeFileSync(path.join(docs, "images", output), Buffer.from(png, "base64"));
        console.log(`Rendered images/${output} using the tutorial's current shader helpers.`);
    } finally {
        await browser?.close();
        server.close();
    }
})().catch(error => { console.error(error); process.exitCode = 1; });
