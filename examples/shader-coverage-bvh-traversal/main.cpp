// BVH-traversal coverage demo — raw-Vulkan host driver.
//
// Build a BVH over a procedural mesh on CPU, upload to GPU, trace
// rays through it via compute shader, read back coverage. Smoke mode
// uses a clean icosphere with one material; stress mode adds extra
// material kinds + degenerate triangles + a packed cluster.
//
// All GPU-runtime calls go through `vk_compute_demo.h`. See its
// file-level comment for the swap procedure when slang-rhi PR #739
// lands.

#include "vk_compute_demo.h"

#include <slang-com-ptr.h>
#include <slang.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <string_view>
#include <vector>

using Slang::ComPtr;

namespace
{

constexpr uint32_t kCoverageBinding = 0;
constexpr uint32_t kCoverageSet = 1;
constexpr uint32_t kRayGridDim = 512;
constexpr uint32_t kRayCount = kRayGridDim * kRayGridDim;

struct Vec3
{
    float x, y, z;
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
};

Vec3 cross(const Vec3& a, const Vec3& b)
{
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

float length(const Vec3& v) { return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }
Vec3 normalize(const Vec3& v)
{
    float l = length(v);
    return l > 0 ? Vec3{v.x / l, v.y / l, v.z / l} : Vec3{0, 0, 0};
}
Vec3 vmin(const Vec3& a, const Vec3& b) { return {std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)}; }
Vec3 vmax(const Vec3& a, const Vec3& b) { return {std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z)}; }

struct alignas(16) Triangle
{
    Vec3 v0;
    int materialId;
    Vec3 v1;
    int pad1;
    Vec3 v2;
    int pad2;
};
static_assert(sizeof(Triangle) == 48, "Triangle layout mismatch");

struct alignas(16) BVHNode
{
    Vec3 bmin;
    int leftIndex;
    Vec3 bmax;
    int rightIndex;
    int firstTri;
    int triCount;
    int pad0;
    int pad1;
};
static_assert(sizeof(BVHNode) == 48, "BVHNode layout mismatch");

struct alignas(16) Ray
{
    Vec3 origin;
    float tMin;
    Vec3 dir;
    float tMax;
};
static_assert(sizeof(Ray) == 32, "Ray layout mismatch");

struct alignas(16) Globals
{
    uint32_t rayCount;
    uint32_t triCount;
    uint32_t nodeCount;
    uint32_t pad;
};

[[noreturn]] void fail(const std::string& message)
{
    std::cerr << "error: " << message << "\n";
    std::exit(1);
}

void checkSlang(SlangResult result, const char* what)
{
    if (SLANG_FAILED(result))
        fail(std::string(what) + " failed");
}

void diagnoseIfNeeded(slang::IBlob* diagnostics)
{
    if (diagnostics && diagnostics->getBufferSize())
    {
        std::cerr.write(
            reinterpret_cast<const char*>(diagnostics->getBufferPointer()),
            diagnostics->getBufferSize());
        std::cerr << "\n";
    }
}

std::filesystem::path getDemoDirectory()
{
    return std::filesystem::path(__FILE__).parent_path();
}

void addQuad(std::vector<Triangle>& tris, Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3, int materialId)
{
    Triangle t1 = {}; t1.v0 = p0; t1.v1 = p1; t1.v2 = p2; t1.materialId = materialId;
    Triangle t2 = {}; t2.v0 = p0; t2.v1 = p2; t2.v2 = p3; t2.materialId = materialId;
    tris.push_back(t1);
    tris.push_back(t2);
}

std::vector<Triangle> buildIcosphere(int subdivisions, int materialId)
{
    const float t = (1.0f + std::sqrt(5.0f)) / 2.0f;
    std::vector<Vec3> verts = {
        normalize({-1, t, 0}), normalize({1, t, 0}), normalize({-1, -t, 0}), normalize({1, -t, 0}),
        normalize({0, -1, t}), normalize({0, 1, t}), normalize({0, -1, -t}), normalize({0, 1, -t}),
        normalize({t, 0, -1}), normalize({t, 0, 1}), normalize({-t, 0, -1}), normalize({-t, 0, 1}),
    };
    std::vector<std::array<int, 3>> faces = {
        {0, 11, 5}, {0, 5, 1}, {0, 1, 7}, {0, 7, 10}, {0, 10, 11},
        {1, 5, 9}, {5, 11, 4}, {11, 10, 2}, {10, 7, 6}, {7, 1, 8},
        {3, 9, 4}, {3, 4, 2}, {3, 2, 6}, {3, 6, 8}, {3, 8, 9},
        {4, 9, 5}, {2, 4, 11}, {6, 2, 10}, {8, 6, 7}, {9, 8, 1},
    };
    for (int s = 0; s < subdivisions; ++s)
    {
        std::vector<std::array<int, 3>> next;
        next.reserve(faces.size() * 4);
        for (auto& f : faces)
        {
            int a = (int)verts.size(); verts.push_back(normalize(verts[f[0]] + verts[f[1]]));
            int b = (int)verts.size(); verts.push_back(normalize(verts[f[1]] + verts[f[2]]));
            int c = (int)verts.size(); verts.push_back(normalize(verts[f[2]] + verts[f[0]]));
            next.push_back({f[0], a, c});
            next.push_back({f[1], b, a});
            next.push_back({f[2], c, b});
            next.push_back({a, b, c});
        }
        faces = std::move(next);
    }
    std::vector<Triangle> out;
    out.reserve(faces.size());
    for (auto& f : faces)
    {
        Triangle t = {};
        t.v0 = verts[f[0]]; t.v1 = verts[f[1]]; t.v2 = verts[f[2]];
        t.materialId = materialId;
        out.push_back(t);
    }
    return out;
}

std::vector<Triangle> buildSmokeMesh() { return buildIcosphere(1, 0); }

std::vector<Triangle> buildStressMesh()
{
    auto tris = buildIcosphere(2, 0);
    addQuad(tris, {1.5f, -0.5f, 0}, {2.5f, -0.5f, 0}, {2.5f, 0.5f, 0}, {1.5f, 0.5f, 0}, 1);
    addQuad(tris, {-2.5f, -0.5f, 0}, {-1.5f, -0.5f, 0}, {-1.5f, 0.5f, 0}, {-2.5f, 0.5f, 0}, 2);
    addQuad(tris, {-0.5f, 1.5f, 0}, {0.5f, 1.5f, 0}, {0.5f, 2.5f, 0}, {-0.5f, 2.5f, 0}, 3);
    for (int i = 0; i < 4; ++i)
    {
        Triangle t = {};
        t.v0 = {0.0f, -2.0f - i * 0.2f, 0.0f};
        t.v1 = {0.0f, -2.0f - i * 0.2f, 0.0f};
        t.v2 = {0.0f, -2.0f - i * 0.2f, 0.5f};
        t.materialId = 0;
        tris.push_back(t);
    }
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> u(0.0f, 0.02f);
    Vec3 clusterCentre{0.0f, 0.0f, -2.5f};
    for (int i = 0; i < 64; ++i)
    {
        Triangle t = {};
        t.v0 = clusterCentre + Vec3{u(rng), u(rng), u(rng)};
        t.v1 = clusterCentre + Vec3{u(rng), u(rng), u(rng)};
        t.v2 = clusterCentre + Vec3{u(rng), u(rng), u(rng)};
        t.materialId = 0;
        tris.push_back(t);
    }
    return tris;
}

struct BuildState
{
    std::vector<Triangle>& tris;
    std::vector<BVHNode>& nodes;
};

Vec3 triCentroid(const Triangle& t) { return (t.v0 + t.v1 + t.v2) * (1.0f / 3.0f); }
Vec3 triMin(const Triangle& t) { return vmin(vmin(t.v0, t.v1), t.v2); }
Vec3 triMax(const Triangle& t) { return vmax(vmax(t.v0, t.v1), t.v2); }

int buildBVHRec(BuildState& s, int triStart, int triEnd)
{
    Vec3 bmin = triMin(s.tris[triStart]);
    Vec3 bmax = triMax(s.tris[triStart]);
    for (int i = triStart + 1; i < triEnd; ++i)
    {
        bmin = vmin(bmin, triMin(s.tris[i]));
        bmax = vmax(bmax, triMax(s.tris[i]));
    }
    const int count = triEnd - triStart;
    const int kLeafThreshold = 4;
    if (count <= kLeafThreshold)
    {
        BVHNode leaf = {};
        leaf.bmin = bmin; leaf.bmax = bmax;
        leaf.leftIndex = -1; leaf.rightIndex = -1;
        leaf.firstTri = triStart; leaf.triCount = count;
        s.nodes.push_back(leaf);
        return (int)s.nodes.size() - 1;
    }
    Vec3 extent = bmax - bmin;
    int axis = 0;
    if (extent.y > extent.x) axis = 1;
    if (axis == 0 ? (extent.z > extent.x) : (extent.z > extent.y)) axis = 2;
    auto compareByAxis = [axis](const Triangle& a, const Triangle& b) {
        Vec3 ca = triCentroid(a);
        Vec3 cb = triCentroid(b);
        if (axis == 0) return ca.x < cb.x;
        if (axis == 1) return ca.y < cb.y;
        return ca.z < cb.z;
    };
    int mid = (triStart + triEnd) / 2;
    std::nth_element(s.tris.begin() + triStart, s.tris.begin() + mid, s.tris.begin() + triEnd, compareByAxis);
    int nodeIdx = (int)s.nodes.size();
    BVHNode internal = {};
    internal.bmin = bmin; internal.bmax = bmax;
    s.nodes.push_back(internal);
    int leftIdx = buildBVHRec(s, triStart, mid);
    int rightIdx = buildBVHRec(s, mid, triEnd);
    s.nodes[nodeIdx].leftIndex = leftIdx;
    s.nodes[nodeIdx].rightIndex = rightIdx;
    return nodeIdx;
}

void rerootNodes(std::vector<BVHNode>& nodes, int rootIndex)
{
    if (rootIndex == 0) return;
    std::swap(nodes[0], nodes[rootIndex]);
    for (auto& n : nodes)
    {
        if (n.leftIndex == 0) n.leftIndex = rootIndex;
        else if (n.leftIndex == rootIndex) n.leftIndex = 0;
        if (n.rightIndex == 0) n.rightIndex = rootIndex;
        else if (n.rightIndex == rootIndex) n.rightIndex = 0;
    }
}

std::vector<BVHNode> buildBVH(std::vector<Triangle>& tris)
{
    std::vector<BVHNode> nodes;
    nodes.reserve(tris.size() * 2);
    BuildState s{tris, nodes};
    int root = buildBVHRec(s, 0, (int)tris.size());
    rerootNodes(s.nodes, root);
    return s.nodes;
}

std::vector<Ray> generateRays(uint32_t gridDim)
{
    std::vector<Ray> rays(gridDim * gridDim);
    Vec3 camPos{0.0f, 0.0f, 4.0f};
    float fov = 1.0f;
    for (uint32_t j = 0; j < gridDim; ++j)
    {
        for (uint32_t i = 0; i < gridDim; ++i)
        {
            float u = (float(i) / float(gridDim - 1)) * 2.0f - 1.0f;
            float v = (float(j) / float(gridDim - 1)) * 2.0f - 1.0f;
            Vec3 dir = normalize({u * fov, v * fov, -1.0f});
            Ray r = {};
            r.origin = camPos;
            r.dir = dir;
            r.tMin = 0.001f;
            r.tMax = 100.0f;
            rays[j * gridDim + i] = r;
        }
    }
    return rays;
}

struct CompiledShader
{
    std::vector<uint8_t> spirv;
    ComPtr<slang::IMetadata> metadata;
    slang::ICoverageTracingMetadata* coverageMetadata = nullptr;
};

CompiledShader compileShader(bool enableCoverage)
{
    ComPtr<slang::IGlobalSession> globalSession;
    checkSlang(slang::createGlobalSession(globalSession.writeRef()), "createGlobalSession");

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    std::vector<slang::CompilerOptionEntry> options;
    auto pushBool = [&](slang::CompilerOptionName name) {
        slang::CompilerOptionEntry e = {};
        e.name = name;
        e.value.kind = slang::CompilerOptionValueKind::Int;
        e.value.intValue0 = 1;
        options.push_back(e);
    };
    pushBool(slang::CompilerOptionName::EmitSpirvDirectly);
    if (enableCoverage)
    {
        pushBool(slang::CompilerOptionName::TraceCoverage);
        pushBool(slang::CompilerOptionName::TraceFunctionCoverage);
        pushBool(slang::CompilerOptionName::TraceBranchCoverage);

        slang::CompilerOptionEntry pin = {};
        pin.name = slang::CompilerOptionName::TraceCoverageBinding;
        pin.value.kind = slang::CompilerOptionValueKind::Int;
        pin.value.intValue0 = kCoverageBinding;
        pin.value.intValue1 = kCoverageSet;
        options.push_back(pin);
    }

    const std::string searchPath = getDemoDirectory().string();
    const char* searchPaths[] = {searchPath.c_str()};

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;
    sessionDesc.searchPaths = searchPaths;
    sessionDesc.searchPathCount = 1;
    sessionDesc.compilerOptionEntries = options.data();
    sessionDesc.compilerOptionEntryCount = (uint32_t)options.size();

    ComPtr<slang::ISession> session;
    checkSlang(globalSession->createSession(sessionDesc, session.writeRef()), "createSession");

    ComPtr<slang::IBlob> diagnostics;
    const std::string modulePath = (getDemoDirectory() / "bvh_traverse.slang").string();
    slang::IModule* loaded = session->loadModule(modulePath.c_str(), diagnostics.writeRef());
    diagnoseIfNeeded(diagnostics);
    if (!loaded)
        fail("loadModule(bvh_traverse.slang)");
    ComPtr<slang::IModule> module(loaded);

    ComPtr<slang::IEntryPoint> entryPoint;
    checkSlang(module->findEntryPointByName("computeMain", entryPoint.writeRef()), "findEntryPointByName");

    slang::IComponentType* parts[] = {module.get(), entryPoint.get()};
    ComPtr<slang::IComponentType> composed;
    diagnostics.setNull();
    checkSlang(
        session->createCompositeComponentType(parts, 2, composed.writeRef(), diagnostics.writeRef()),
        "createCompositeComponentType");
    diagnoseIfNeeded(diagnostics);

    ComPtr<slang::IComponentType> linked;
    diagnostics.setNull();
    checkSlang(composed->link(linked.writeRef(), diagnostics.writeRef()), "link");
    diagnoseIfNeeded(diagnostics);

    ComPtr<slang::IBlob> code;
    diagnostics.setNull();
    checkSlang(linked->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef()), "getEntryPointCode");
    diagnoseIfNeeded(diagnostics);

    CompiledShader out;
    out.spirv.assign(
        (const uint8_t*)code->getBufferPointer(),
        (const uint8_t*)code->getBufferPointer() + code->getBufferSize());

    if (enableCoverage)
    {
        diagnostics.setNull();
        checkSlang(
            linked->getEntryPointMetadata(0, 0, out.metadata.writeRef(), diagnostics.writeRef()),
            "getEntryPointMetadata");
        diagnoseIfNeeded(diagnostics);
        out.coverageMetadata = (slang::ICoverageTracingMetadata*)out.metadata->castAs(
            slang::ICoverageTracingMetadata::getTypeGuid());
        if (!out.coverageMetadata)
            fail("expected coverage metadata");
    }
    return out;
}

void writeManifest(slang::ICoverageTracingMetadata* coverage, const std::filesystem::path& path)
{
    ComPtr<ISlangBlob> manifestBlob;
    checkSlang(
        slang_writeCoverageManifestJson(coverage, manifestBlob.writeRef()),
        "slang_writeCoverageManifestJson");
    std::ofstream out(path, std::ios::binary);
    out.write(
        static_cast<const char*>(manifestBlob->getBufferPointer()),
        (std::streamsize)manifestBlob->getBufferSize());
}

void writeCountersBinary(const std::vector<uint32_t>& hits, const std::filesystem::path& path)
{
    std::ofstream out(path, std::ios::binary);
    out.write(
        reinterpret_cast<const char*>(hits.data()),
        (std::streamsize)(hits.size() * sizeof(uint32_t)));
}

void writeLcov(
    slang::ICoverageTracingMetadata* coverage,
    const std::vector<uint32_t>& hits,
    const std::filesystem::path& path,
    const char* testName)
{
    const uint32_t counterCount = coverage->getCounterCount();
    std::map<std::string, std::map<uint32_t, uint64_t>> byFile;
    for (uint32_t i = 0; i < counterCount; ++i)
    {
        slang::CoverageEntryInfo entry = {};
        if (SLANG_FAILED(coverage->getEntryInfo(i, &entry)))
            continue;
        if (!entry.file || !*entry.file || entry.line == 0)
            continue;
        byFile[entry.file][entry.line] += hits[i];
    }
    std::ofstream f(path, std::ios::binary);
    f << "TN:" << testName << "\n";
    for (const auto& fp : byFile)
    {
        f << "SF:" << fp.first << "\n";
        for (const auto& lp : fp.second)
            f << "DA:" << lp.first << "," << lp.second << "\n";
        f << "end_of_record\n";
    }
}

struct CoverageSummary
{
    uint32_t lineCovered = 0;
    uint32_t lineTotal = 0;
    uint32_t functionCovered = 0;
    uint32_t functionTotal = 0;
    uint32_t branchCovered = 0;
    uint32_t branchTotal = 0;
};

CoverageSummary summarize(slang::ICoverageTracingMetadata* coverage, const std::vector<uint32_t>& hits)
{
    CoverageSummary s = {};
    const uint32_t n = coverage->getCounterCount();
    for (uint32_t i = 0; i < n; ++i)
    {
        slang::CoverageEntryInfo entry = {};
        if (SLANG_FAILED(coverage->getEntryInfo(i, &entry)))
            continue;
        const bool covered = hits[i] > 0;
        switch (entry.kind)
        {
        case slang::CoverageEntryKind::Line:
            ++s.lineTotal;
            if (covered) ++s.lineCovered;
            break;
        case slang::CoverageEntryKind::Function:
            ++s.functionTotal;
            if (covered) ++s.functionCovered;
            break;
        case slang::CoverageEntryKind::Branch:
            ++s.branchTotal;
            if (covered) ++s.branchCovered;
            break;
        default:
            break;
        }
    }
    return s;
}

void printSummary(const char* label, const CoverageSummary& s)
{
    std::cout << label << ":\n"
              << "  line     : " << s.lineCovered << " / " << s.lineTotal << "\n"
              << "  function : " << s.functionCovered << " / " << s.functionTotal << "\n"
              << "  branch   : " << s.branchCovered << " / " << s.branchTotal << "\n";
}

} // namespace

int main(int argc, char** argv)
{
    std::string mode = "smoke";
    bool enableCoverage = true;
    for (int i = 1; i < argc; ++i)
    {
        std::string_view a = argv[i];
        if (a == "--mode=smoke") mode = "smoke";
        else if (a == "--mode=stress") mode = "stress";
        else if (a == "--no-coverage") enableCoverage = false;
        else if (a == "--coverage") enableCoverage = true;
        else { std::cerr << "unknown arg: " << a << "\n"; return 1; }
    }

    std::cout << "compiling bvh_traverse.slang"
              << (enableCoverage ? " (coverage on)\n" : " (no coverage)\n");
    auto shader = compileShader(enableCoverage);

    uint32_t counterCount = 0;
    if (enableCoverage)
    {
        counterCount = shader.coverageMetadata->getCounterCount();
        std::cout << "coverage counter count: " << counterCount << "\n";
    }

    auto tris = (mode == "smoke") ? buildSmokeMesh() : buildStressMesh();
    std::cout << "mesh: " << tris.size() << " triangles (" << mode << ")\n";
    auto nodes = buildBVH(tris);
    std::cout << "BVH: " << nodes.size() << " nodes\n";
    auto rays = generateRays(kRayGridDim);
    std::cout << "rays: " << rays.size() << " (" << kRayGridDim << "x" << kRayGridDim << ")\n";

    Globals globalsData = {};
    globalsData.rayCount = (uint32_t)rays.size();
    globalsData.triCount = (uint32_t)tris.size();
    globalsData.nodeCount = (uint32_t)nodes.size();

    vkdemo::Context ctx;
    ctx.init();

    std::vector<std::vector<VkDescriptorSetLayoutBinding>> setBindings;
    setBindings.resize(2);
    auto pushBinding = [](std::vector<VkDescriptorSetLayoutBinding>& v, uint32_t b) {
        VkDescriptorSetLayoutBinding lb = {};
        lb.binding = b;
        lb.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        lb.descriptorCount = 1;
        lb.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        v.push_back(lb);
    };
    for (uint32_t i = 0; i < 5; ++i)
        pushBinding(setBindings[0], i);
    if (enableCoverage)
        pushBinding(setBindings[1], kCoverageBinding);
    else
        setBindings.pop_back();

    // Slang's SPIR-V emit renames the entry point to "main" by default.
    auto pipe = ctx.createComputePipeline(
        shader.spirv.data(),
        shader.spirv.size(),
        setBindings,
        "main");

    auto rayBuf = ctx.createBuffer(rays.size() * sizeof(Ray), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    ctx.upload(rayBuf, rays.data(), rayBuf.size);
    auto triBuf = ctx.createBuffer(tris.size() * sizeof(Triangle), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    ctx.upload(triBuf, tris.data(), triBuf.size);
    auto nodeBuf = ctx.createBuffer(nodes.size() * sizeof(BVHNode), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    ctx.upload(nodeBuf, nodes.data(), nodeBuf.size);
    auto globalsBuf = ctx.createBuffer(sizeof(Globals), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    ctx.upload(globalsBuf, &globalsData, sizeof(globalsData));
    auto outputBuf = ctx.createBuffer(rays.size() * 4 * sizeof(float), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    vkdemo::Buffer coverageBuf = {};
    if (enableCoverage)
    {
        coverageBuf = ctx.createBuffer(counterCount * sizeof(uint32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
        std::vector<uint32_t> zero(counterCount, 0u);
        ctx.upload(coverageBuf, zero.data(), coverageBuf.size);
    }

    VkDescriptorSet set0 = ctx.allocateDescriptorSet(pipe.setLayouts[0]);
    ctx.writeStorageBuffer(set0, 0, rayBuf);
    ctx.writeStorageBuffer(set0, 1, triBuf);
    ctx.writeStorageBuffer(set0, 2, nodeBuf);
    ctx.writeStorageBuffer(set0, 3, globalsBuf);
    ctx.writeStorageBuffer(set0, 4, outputBuf);

    std::vector<VkDescriptorSet> sets = {set0};
    if (enableCoverage)
    {
        VkDescriptorSet set1 = ctx.allocateDescriptorSet(pipe.setLayouts[1]);
        ctx.writeStorageBuffer(set1, kCoverageBinding, coverageBuf);
        sets.push_back(set1);
    }

    std::cout << "dispatching (coverage=" << (enableCoverage ? "on" : "off") << ")\n";
    const auto renderStart = std::chrono::steady_clock::now();
    const uint32_t groups = (kRayCount + 63) / 64;
    ctx.dispatch(pipe, sets, groups, 1, 1);
    const auto renderEnd = std::chrono::steady_clock::now();
    const double renderMs = std::chrono::duration<double, std::milli>(renderEnd - renderStart).count();
    std::cout << "render wall time: " << renderMs << " ms\n";

    if (enableCoverage)
    {
        std::vector<uint32_t> hits(counterCount);
        ctx.download(coverageBuf, hits.data(), coverageBuf.size);

        auto summary = summarize(shader.coverageMetadata, hits);
        printSummary(mode.c_str(), summary);

        const auto outDir = getDemoDirectory();
        writeManifest(shader.coverageMetadata, outDir / (mode + ".coverage-mapping.json"));
        writeLcov(shader.coverageMetadata, hits, outDir / (mode + ".lcov"),
                  ("bvh-" + mode).c_str());
        writeCountersBinary(hits, outDir / (mode + ".counters.bin"));
        std::cout << "wrote " << (outDir / (mode + ".coverage-mapping.json")) << "\n";
        std::cout << "wrote " << (outDir / (mode + ".lcov")) << "\n";
        std::cout << "wrote " << (outDir / (mode + ".counters.bin")) << "\n";
    }

    ctx.destroyBuffer(rayBuf);
    ctx.destroyBuffer(triBuf);
    ctx.destroyBuffer(nodeBuf);
    ctx.destroyBuffer(globalsBuf);
    ctx.destroyBuffer(outputBuf);
    if (enableCoverage)
        ctx.destroyBuffer(coverageBuf);
    ctx.destroyPipeline(pipe);
    std::cout << "done\n";
    return 0;
}
