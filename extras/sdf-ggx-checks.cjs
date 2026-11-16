// Integrate the actual tutorial shaders under white light, independent of the panorama.
const assert = require("node:assert/strict");

exports.checkGGXNumerics = async function (page, sources, baseURL) {
    const result = await page.evaluate(async ({ sources, baseURL }) => {
        const { init } = await import(new URL("assets/slangvm/src/index.js", baseURL));
        const { MemorySlangFileSystem, OverlaySlangFileSystem } = await import(new URL("assets/slangvm/src/file-system.js", baseURL));
        const { preloadRuntime } = await import(new URL("scripts/snippet-filesystem.js", baseURL));
        const { preloadedFiles } = await preloadRuntime();
        const checks = `import slang.runtime; import ggx; import "ggx-sampling"; import hemisphere; import prng;
            [host] float integrate(uint testCase) {
                float alpha = testCase % 3u == 0u ? 0.25 : (testCase % 3u == 1u ? 0.49 : 1.0);
                float cosine = testCase / 3u == 0u ? 1.0 : (testCase / 3u == 1u ? 0.5 : 0.1);
                float3 n = float3(0, 0, 1);
                float3 v = float3(sqrt(1.0 - cosine * cosine), 0, cosine);
                let values = RWStructuredBuffer<float4>::alloc(65536);
                Grid(uint2(256u)).dispatch<8, 8>((uint2 pixel) => {
                    uint index = pixel.x + 256u * pixel.y;
                    float2 u = (float2(pixel) + 0.5) / 256.0;
                    float3 l = uniformHemisphere(n, u);
                    float uniform = evaluateGGX(n, v, l, alpha, float3(1)).r * l.z * (2 * 3.14159265);
                    float pdfIntegral = ggxReflectionPDF(n, v, l, alpha) * (2 * 3.14159265);
                    float3 h = sampleGGXHalfVector(n, u, alpha);
                    float importance = 0.0;
                    float accepted = 0.0;
                    if (dot(v, h) > 0.0) {
                        float3 sampled = normalize(reflect(-v, h));
                        if (sampled.z > 0.0) {
                            float pdf = ggxReflectionPDF(n, v, sampled, alpha);
                            importance = evaluateGGX(n, v, sampled, alpha, float3(1)).r * sampled.z / pdf;
                            accepted = 1.0;
                        }
                    }
                    float4 value = float4(uniform, importance, pdfIntegral, accepted);
                    values[index] = all(isfinite(value)) && all(value >= 0.0) ? value : float4(1e6);
                });
                let result = RWStructuredBuffer<float>::alloc(1);
                Grid(1).dispatch((uint i) => {
                    float4 sum = float4(0);
                    for (uint j = 0; j < 65536; j++) sum += values[j];
                    float4 mean = sum / 65536.0;
                    float error = max(abs(mean.x - mean.y), abs(mean.z - mean.w));
                    // The PDF's integral over valid directions equals the acceptance probability.
                    if (max(mean.x, mean.y) > 1.002) error += 1000;
                    // At alpha=1, normal view, F=1: integral c/(1+c) dc = 1 - ln(2).
                    if (testCase == 2u) error = max(error, abs(mean.x - (1.0 - log(2.0))));
                    result[0] = error;
                });
                return result[0];
            }
            [host] float invariants() {
                let values = RWStructuredBuffer<float>::alloc(4096);
                Grid(4096).dispatch((uint i) => {
                    float3 n = i % 2u == 0u ? float3(0, 1, 0) : float3(0, -1, 0);
                    uint state = mixBits(i);
                    float u = nextRandom(state);
                    float w = nextRandom(state);
                    float roughness = float(i % 11u) / 10.0;
                    float alpha = ggxAlpha(roughness);
                    float3 h = sampleGGXHalfVector(n, float2(u, w), alpha);
                    float c = i % 3u == 0u ? 0.000001 : 0.7;
                    float3 v = orientHemisphere(float3(sqrt(1.0-c*c), 0, c), n);
                    float3 l = uniformHemisphere(n, float2(w, u));
                    float3 F0 = float3(0.9, 0.55, 0.2);
                    float3 a = evaluateGGX(n, v, l, alpha, F0);
                    float3 b = evaluateGGX(n, l, v, alpha, F0);
                    float pdf = ggxReflectionPDF(n, v, l, alpha);
                    float error = length((a-b) / (float3(1)+a));
                    error += abs(length(h)-1.0) + max(0.0, -dot(n, h));
                    error += length(evaluateGGX(n, v, -n, alpha, F0));
                    error += ggxReflectionPDF(n, v, -n, alpha);
                    error += length(fresnelSchlick(1.0, F0)-F0);
                    error += length(fresnelSchlick(0.0, F0)-float3(1.0));
                    if (!all(isfinite(a)) || any(a < 0.0) || !isfinite(pdf) || pdf < 0.0)
                        error += 1e6;
                    values[i] = isfinite(error) ? error : 1e6;
                });
                let result = RWStructuredBuffer<float>::alloc(1);
                Grid(1).dispatch((uint i) => {
                    float worst = 0;
                    for (uint j = 0; j < 4096; j++) worst = max(worst, values[j]);
                    result[0] = worst;
                });
                return result[0];
            }`;
        const vm = await init({ backend: "webgpu", fileSystem: new OverlaySlangFileSystem([
            new MemorySlangFileSystem({ ...sources, "ggx-check.slang": checks }),
            new MemorySlangFileSystem(preloadedFiles),
        ]) });
        try {
            const module = await vm.load("ggx-check.slang");
            const integration = [];
            for (let i = 0; i < 9; i++) integration.push(await module.integrate(i));
            return { integration, invariants: await module.invariants() };
        } finally { await vm.close(); }
    }, { sources, baseURL });
    assert.ok(result.integration.every(error => Number.isFinite(error) && error < 0.012),
        `GGX estimators/PDF mass/white-light energy disagree: ${JSON.stringify(result)}`);
    assert.ok(Number.isFinite(result.invariants) && result.invariants < 0.0001,
        `GGX reciprocity, Fresnel, hemisphere, or grazing-angle check failed: ${JSON.stringify(result)}`);
    console.log(`PASS: GGX integration across nine roughness/view combinations; maximum error ${Math.max(...result.integration).toFixed(6)}`);
};
