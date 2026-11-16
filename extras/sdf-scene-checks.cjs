const assert = require("node:assert/strict");

exports.checkSceneNumerics = async function (page, sources, baseURL) {
    const result = await page.evaluate(async ({ sources, baseURL }) => {
        const { init } = await import(new URL("assets/slangvm/src/index.js", baseURL));
        const { MemorySlangFileSystem, OverlaySlangFileSystem } = await import(new URL("assets/slangvm/src/file-system.js", baseURL));
        const { preloadRuntime } = await import(new URL("scripts/snippet-filesystem.js", baseURL));
        const { preloadedFiles } = await preloadRuntime();
        const checks = `import slang.runtime; import "checker-scene"; import "coated-diffuse";
            import "sphere-sdf"; import "box-sdf"; import "sphere-tracer"; import hemisphere;
            [host] float integrate(uint testCase) {
                let values = RWStructuredBuffer<float>::alloc(65536);
                Grid(uint2(256u)).dispatch<8, 8>((uint2 p) => {
                    float roughness = testCase == 0u ? 0.35 : 0.7;
                    if (testCase == 2u) roughness = 0.0;
                    let material = CoatedDiffuseBRDF(float3(0.2, 0.4, 0.6), roughness, float3(0.04), 1.0);
                    float3 n = float3(0, 1, 0);
                    float3 v = n;
                    float2 u = (float2(p) + 0.5) / 256.0;
                    float3 l = uniformHemisphere(n, u);
                    float estimate = material.evaluate(n, v, l).g * dot(n, l) * (2.0 * 3.14159265);
                    float sampled = material.sample(n, v, u).weight.g;
                    if (testCase == 2u) estimate = 0.04 + 0.4 * 0.96 * 0.96 * (20.0 / 21.0);
                    values[p.x + 256u * p.y] = isfinite(sampled) && sampled >= 0.0 ? sampled - estimate : 1e6;
                });
                let result = RWStructuredBuffer<float>::alloc(1);
                Grid(1).dispatch((uint i) => {
                    float sum = 0.0;
                    for (uint j = 0; j < 65536; j++) sum += values[j];
                    result[0] = abs(sum / 65536.0);
                });
                return result[0];
            }
            [host] float geometry() {
                let result = RWStructuredBuffer<float>::alloc(1);
                Grid(1).dispatch((uint i) => {
                    let material = CoatedDiffuseBRDF(float3(0.07, 0.28, 0.36), 0.12, float3(0.04), 1.0);
                    let sphere = SphereSDF(float3(0, 1, 0), 1.0);
                    let board = BoxSDF(float3(0, -0.04, 0), float3(4.0, 0.04, 4.0));
                    let scene = SphereCheckerScene(sphere, board, material, 0.5, float3(0.1), float3(0.3));
                    let tracing = TraceSettings(0.001, 10.0, 96);
                    float error = abs(scene.distance(float3(0, 2, 0)));
                    error += abs(board.distance(float3(4.5, -0.04, 0)) - 0.5);
                    error += abs(board.distance(float3(2, -0.04, 0)) + 0.04);
                    float3 hit;
                    if (!sphereTrace(scene, float3(0, 1, -4), float3(0, 0, 1), hit, tracing)) error += 1;
                    error += length(hit - float3(0, 1, -1));
                    if (!sphereTrace(scene, float3(2, 3, 0), float3(0, -1, 0), hit, tracing)) error += 1;
                    error += abs(hit.y);
                    if (sphereTrace(scene, float3(5, 3, 0), float3(0, -1, 0), hit, tracing)) error += 1;
                    error += length(surfaceNormal(scene, float3(2, 0, 0), 0.002) - float3(0, 1, 0));
                    error += length(scene.materialAt(float3(0, 1, -1)).albedo - material.albedo);
                    error += abs(scene.materialAt(float3(2.1, 0, 1.1)).albedo.r - 0.1);
                    error += abs(scene.materialAt(float3(2.6, 0, 1.1)).albedo.r - 0.3);
                    error += abs(scene.materialAt(float3(-2.1, 0, 1.1)).albedo.r - 0.3);
                    // Independently placed primitives must retain their own dimensions.
                    let small = SphereSDF(float3(3, 0, 0), 0.5);
                    let large = SphereSDF(float3(-3, 0, 0), 2.0);
                    error += abs(small.distance(float3(3.5, 0, 0)));
                    error += abs(large.distance(float3(-1, 0, 0)));
                    error += abs(small.distance(float3(3, 0, 0)) + 0.5);
                    let movedBoard = BoxSDF(float3(1, -0.04, 2), board.halfSize);
                    let movedScene = SphereCheckerScene(sphere, movedBoard, material, 0.5, float3(0.1), float3(0.3));
                    error += length(movedScene.materialAt(float3(3.1, 0, 3.1)).albedo
                        - scene.materialAt(float3(2.1, 0, 1.1)).albedo);
                    let matte = CoatedDiffuseBRDF(float3(0.2, 0.4, 0.6), 0.12, float3(0.04), 0.0);
                    error += length(matte.sample(float3(0, 1, 0), float3(0, 1, 0), float2(0.37, 0.61)).weight - matte.albedo);
                    result[0] = error;
                });
                return result[0];
            }`;
        const vm = await init({ backend: "webgpu", fileSystem: new OverlaySlangFileSystem([
            new MemorySlangFileSystem({ ...sources, "scene-check.slang": checks }),
            new MemorySlangFileSystem(preloadedFiles),
        ]) });
        try {
            const module = await vm.load("scene-check.slang");
            return { integration: [await module.integrate(0), await module.integrate(1), await module.integrate(2)],
                geometry: await module.geometry() };
        } finally { await vm.close(); }
    }, { sources, baseURL });
    assert.ok(result.integration.every(error => Number.isFinite(error) && error < 0.008),
        `Coated diffuse mixture and mirror-limit estimators disagree: ${JSON.stringify(result)}`);
    assert.ok(Number.isFinite(result.geometry) && result.geometry < 0.004,
        `Bounded floor, union, checker parity, or matte sampling failed: ${JSON.stringify(result)}`);
    console.log(`PASS: finite checkerboard scene and coated diffuse sampling; maximum integration error ${Math.max(...result.integration).toFixed(6)}`);
};
