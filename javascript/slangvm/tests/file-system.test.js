import assert from "node:assert/strict";
import { test } from "node:test";
import { MemorySlangFileSystem, OverlaySlangFileSystem, normalizePath } from "../src/file-system.js";
import { FileSystemHost } from "../src/file-system-host.js";
import { HTTPSlangFileSystem } from "../src/http-file-system.js";

const decode = bytes => new TextDecoder().decode(bytes);
const deferred = () => {
    let resolve;
    const promise = new Promise(r => { resolve = r; });
    return { promise, resolve };
};

test("virtual paths normalize aliases, reject traversal and encode URL syntax as filenames", async () => {
    assert.equal(normalizePath("/lib/../main.slang"), "/main.slang");
    assert.equal(normalizePath("lib\\main.slang"), "/lib/main.slang");
    for (const path of ["../bad", "/../../bad", "https://example.test/a", "C:\\file", "a\0b"]) {
        assert.throws(() => normalizePath(path));
    }
    const requested = [];
    const files = new HTTPSlangFileSystem("https://example.test/prefix", { fetch(url) {
        assert.equal(this, globalThis);
        requested.push(url);
        return Promise.resolve(new Response("file"));
    } });
    await files.loadFile("/a#b?c.slang");
    assert.deepEqual(requested, ["https://example.test/prefix/a%23b%3Fc.slang"]);
});

test("memory files snapshot text and binary inputs and preserve empty files", () => {
    const binary = new Uint8Array([0, 255]);
    const source = { "main.slang": "source", "library.slang-module": binary, "empty": "" };
    const files = new MemorySlangFileSystem(source);
    source["main.slang"] = "changed";
    binary[0] = 10;
    assert.equal(decode(files.loadFile("./main.slang")), "source");
    assert.deepEqual(files.loadFile("library.slang-module"), new Uint8Array([0, 255]));
    assert.equal(files.loadFile("empty").length, 0);
    assert.equal(files.loadFile("missing"), null);
    assert.throws(() => new MemorySlangFileSystem({ "a": "x", "./a": "y" }), /Duplicate/);
    assert.throws(() => new MemorySlangFileSystem({ "a": 42 }), /string or Uint8Array/);
});

test("overlay only falls through on null and keeps synchronous hits synchronous", async () => {
    const memory = new MemorySlangFileSystem({ "edited.slang": "edited", "empty": "" });
    let reads = 0;
    const overlay = new OverlaySlangFileSystem([memory, { async loadFile() { reads++; return null; } },
        new MemorySlangFileSystem({ "import.slang": "import" })]);
    assert.equal(decode(overlay.loadFile("edited.slang")), "edited");
    assert.equal(overlay.loadFile("empty").length, 0);
    assert.equal(reads, 0);
    assert.equal(decode(await overlay.loadFile("import.slang")), "import");
    assert.equal(await overlay.loadFile("missing"), null);
    const broken = new OverlaySlangFileSystem([{ loadFile() { throw new Error("network"); } }, memory]);
    assert.throws(() => broken.loadFile("edited.slang"), /network/);
});

test("HTTP deduplicates requests and returns successful cache hits synchronously", async () => {
    const response = deferred();
    let requests = 0;
    const files = new HTTPSlangFileSystem("https://example.test/", {
        fetch: () => { requests++; return response.promise; },
    });
    const first = files.loadFile("module.slang-module");
    assert.equal(files.loadFile("./module.slang-module"), first);
    response.resolve(new Response(new Uint8Array([0, 255, 128])));
    const bytes = await first;
    assert.equal(files.loadFile("/module.slang-module"), bytes);
    assert.equal(requests, 1);
    const snapshot = files.snapshot();
    snapshot["/module.slang-module"][0] = 20;
    assert.equal(bytes[0], 0, "worker snapshots must own their buffers");
});

for (const status of [404, 410]) {
    test(`HTTP ${status} caching is off by default, even with an injected cache`, async () => {
        let requests = 0;
        const cache = new Set(["https://example.test/file"]);
        const files = new HTTPSlangFileSystem("https://example.test/", {
            missingFileCache: cache,
            fetch: async () => ++requests === 1 ? new Response(null, { status }) : new Response("new"),
        });
        assert.equal(await files.loadFile("file"), null);
        assert.equal(decode(await files.loadFile("file")), "new");
        assert.equal(requests, 2);
    });

    test(`HTTP ${status} caching can be shared by URL and explicitly reset`, async () => {
        let requests = 0;
        const missing = new Set();
        const options = { cacheNotFound: true, missingFileCache: missing,
            fetch: async () => { requests++; return new Response(null, { status }); } };
        const first = new HTTPSlangFileSystem("https://example.test/a/", options);
        assert.equal(await first.loadFile("file"), null);
        assert.equal(first.loadFile("file"), null);
        const second = new HTTPSlangFileSystem("https://example.test/a/", options);
        assert.equal(second.loadFile("file"), null);
        assert.equal(requests, 1);
        await new HTTPSlangFileSystem("https://example.test/b/", options).loadFile("file");
        assert.equal(requests, 2);
        missing.clear();
        await second.loadFile("file");
        assert.equal(requests, 3);
    });
}

test("HTTP failures remain retryable and editor bytes override cached misses", async () => {
    for (const fail of [() => { throw new Error("offline"); },
        () => new Response(null, { status: 503 }), () => { throw new DOMException("abort", "AbortError"); }]) {
        let requests = 0;
        const files = new HTTPSlangFileSystem("https://example.test/", { cacheNotFound: true,
            fetch: async () => ++requests === 1 ? fail() : new Response("recovered") });
        await assert.rejects(files.loadFile("file"));
        assert.equal(decode(await files.loadFile("file")), "recovered");
    }
    const files = new HTTPSlangFileSystem("https://example.test/", { cacheNotFound: true,
        fetch: async () => new Response(null, { status: 404 }) });
    await files.loadFile("edited");
    assert.equal(decode(new OverlaySlangFileSystem([new MemorySlangFileSystem({ edited: "new" }), files])
        .loadFile("edited")), "new");
});

test("closing one VM cancels its read without poisoning another VM's shared download", async () => {
    const response = deferred();
    let requests = 0;
    const files = new HTTPSlangFileSystem("https://example.test/", {
        fetch: () => { requests++; return response.promise; },
    });
    const first = new FileSystemHost(files);
    const second = new FileSystemHost(files);
    const cancelled = first.loadFile("shared");
    const active = second.loadFile("shared");
    first.close();
    await assert.rejects(cancelled, /closed/);
    assert.throws(() => first.loadFile("shared"), /closed/);
    response.resolve(new Response("available"));
    assert.equal(decode(await active), "available");
    assert.equal(requests, 1);
    assert.equal(decode(second.loadFile("shared")), "available");
    second.close();
    assert.equal(decode(files.loadFile("shared")), "available");
});

test("native adapter validates results, accepts thenables, and cancels custom pending reads", async () => {
    for (const value of [undefined, "text", [1, 2], { error: "not bytes" }]) {
        assert.throws(() => new FileSystemHost({ loadFile: () => value }).loadFile("file"), /Uint8Array/);
    }
    const thenable = new FileSystemHost({ loadFile: () => ({ then(resolve) { resolve(new Uint8Array([1])); } }) });
    assert.deepEqual(await thenable.loadFile("file"), new Uint8Array([1]));
    const host = new FileSystemHost({ loadFile: () => new Promise(() => {}) });
    const pending = host.loadFile("file");
    host.close();
    await assert.rejects(pending, /closed/);
});

test("preload bounds concurrency, deduplicates paths, and directory selection respects boundaries", async () => {
    let active = 0;
    let peak = 0;
    const requested = [];
    const files = new HTTPSlangFileSystem("https://example.test/", { fetch: async url => {
        active++; peak = Math.max(peak, active); requested.push(url);
        await new Promise(resolve => setTimeout(resolve, 2));
        active--;
        return new Response("bytes");
    } });
    const manifest = ["runtime/a", "runtime/b", "runtime/nested/c", "runtime-other/d", "tensor"];
    await files.preloadDirectory("runtime", { manifest, concurrency: 2 });
    assert.equal(peak, 2);
    assert.equal(requested.length, 3);
    await files.preload(["runtime/a", "./runtime/a"]);
    assert.equal(requested.length, 3);
    assert.ok(files.loadFile("runtime/a") instanceof Uint8Array);
    await assert.rejects(files.preload([], { concurrency: 0 }), /concurrency/);
    files.clearCache();
    await files.loadFile("runtime/a");
    assert.equal(requested.length, 4);
});

test("fixed HTTP inventory rejects unlisted paths synchronously and snapshots normalized names", async () => {
    const allowed = new Set(["./runtime/../runtime/module.slang-module", "data.bin"]);
    const requests = [];
    const files = new HTTPSlangFileSystem("https://example.test/", {
        allowedPaths: allowed,
        fetch: async url => { requests.push(url); return new Response("bytes"); },
    });
    allowed.add("other.slang");
    allowed.delete("data.bin");
    for (const name of ["other.slang", "runtime", "runtime/module.slang", "runtime/nested/module.slang-module"]) {
        assert.equal(files.loadFile(name), null, name);
    }
    assert.deepEqual(requests, []);
    await files.preload(["runtime\\module.slang-module", "/data.bin"]);
    assert.equal(decode(files.loadFile("/runtime/module.slang-module")), "bytes");
    assert.equal(requests.length, 2);
    files.clearCache();
    assert.equal(files.loadFile("other.slang"), null);
    await assert.rejects(files.preload(["other.slang"]), /missing file/);
    assert.equal(requests.length, 2);
    const overlay = new OverlaySlangFileSystem([
        new MemorySlangFileSystem({ "edited.slang": "editor" }), files,
    ]);
    assert.equal(decode(overlay.loadFile("edited.slang")), "editor");
});

test("empty HTTP inventories deny all reads and inventory arguments are validated", () => {
    const files = new HTTPSlangFileSystem("https://example.test/", {
        allowedPaths: [], fetch: () => { throw new Error("must not fetch"); },
    });
    assert.equal(files.loadFile("anything"), null);
    for (const allowedPaths of [null, "file.slang", {}, ["../escape"]]) {
        assert.throws(() => new HTTPSlangFileSystem("https://example.test/", { allowedPaths }));
    }
});

test("listed HTTP files retain retry and cancellation behavior", async () => {
    let requests = 0;
    const files = new HTTPSlangFileSystem("https://example.test/", {
        allowedPaths: ["module.slang"],
        fetch: async () => ++requests === 1 ? new Response(null, { status: 404 }) : new Response("ready"),
    });
    assert.equal(await files.loadFile("module.slang"), null);
    assert.equal(decode(await files.loadFile("module.slang")), "ready");
    assert.equal(requests, 2);
    const controller = new AbortController();
    controller.abort();
    assert.throws(() => files.loadFile("unlisted", { signal: controller.signal }), { name: "AbortError" });
});

test("preload reports missing files and supports cancellation without cancelling shared cache fills", async () => {
    const missing = new HTTPSlangFileSystem("https://example.test/", {
        fetch: async () => new Response(null, { status: 404 }),
    });
    await assert.rejects(missing.preload(["required"]), /missing file/);
    const response = deferred();
    const files = new HTTPSlangFileSystem("https://example.test/", { fetch: () => response.promise });
    const controller = new AbortController();
    const pending = files.preload(["required"], { signal: controller.signal });
    controller.abort();
    await assert.rejects(pending, { name: "AbortError" });
    response.resolve(new Response("bytes"));
    assert.equal(decode(await files.loadFile("required")), "bytes");
});
