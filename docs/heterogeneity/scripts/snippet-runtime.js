// Shared compilation/output path. Canvas runs use the DOM; text runs call this from a worker.
export async function runSnippet({ source, filename = "main.slang", sources = { [filename]: source },
  backend = "webgpu", canvas, sizeCanvas, missingFileURLs = [], preloadedFiles = {}, allowedPaths = [] }, send, signal) {
  const missingFiles = new Set(missingFileURLs);
  let vm;
  let window;
  let executing = false;
  let lastFrame;
  let phase = "Checking browser support";
  let output = "";
  let lastOutput = 0;
  let truncated = false;
  let failure;
  const status = (text) => {
    phase = text;
    send({ type: "status", text });
  };
  const print = (line) => {
    if (truncated) return;
    const remaining = 65536 - output.length;
    const text = String(line) + "\n";
    output += text.slice(0, remaining);
    if (text.length > remaining) {
      output += "\n[Output limited to 64 KiB of text]\n";
      truncated = true;
    }
    if (performance.now() - lastOutput > 100) {
      send({ type: "output", text: output });
      lastOutput = performance.now();
    }
  };
  const captureFrame = (presentedCanvas) => {
    if (presentedCanvas !== canvas) return;
    lastFrame ??= document.createElement("canvas");
    if (lastFrame.width !== canvas.width) lastFrame.width = canvas.width;
    if (lastFrame.height !== canvas.height) lastFrame.height = canvas.height;
    lastFrame.getContext("2d").drawImage(canvas, 0, 0);
  };
  const stop = () => {
    // Closing wakes a frame wait, allowing the Slang loop to unwind before VM teardown.
    if (executing && window) {
      window.close();
    } else {
      void vm?.close().catch(() => {});
    }
  };
  signal?.addEventListener("abort", stop, { once: true });
  try {
    signal?.throwIfAborted();
    if (backend === "webgpu") {
      if (!globalThis.isSecureContext) throw new Error("WebGPU needs HTTPS or localhost. Open the local documentation preview over http://localhost.");
      if (!navigator.gpu) throw new Error("WebGPU is unavailable in this browser. Try a browser with WebGPU enabled.");
    }
    if (canvas && backend !== "webgpu") throw new Error("Canvas examples require WebGPU.");
    if (typeof WebAssembly.promising !== "function" || typeof WebAssembly.Suspending !== "function") {
      throw new Error("This browser does not support WebAssembly JSPI, which slangvm needs. Try a current Chrome or Edge browser.");
    }
    status("Loading slangvm…");
    const { default: slangvm } = await import("../assets/slangvm/src/index.js");
    const { MemorySlangFileSystem, OverlaySlangFileSystem } = await import("../assets/slangvm/src/file-system.js");
    const { HTTPSlangFileSystem } = await import("../assets/slangvm/src/http-file-system.js");
    signal?.throwIfAborted();
    status(backend === "webgpu" ? "Initializing compiler and WebGPU…" : "Initializing compiler…");
    const libraries = new HTTPSlangFileSystem(new URL("../assets/slangvm/slang/", import.meta.url), {
      allowedPaths,
      cacheNotFound: true,
      missingFileCache: {
        has: url => missingFiles.has(url),
        add(url) {
          if (missingFiles.has(url)) return;
          missingFiles.add(url);
          // Report immediately so the page retains misses even if this run is stopped.
          send({ type: "missing-file", url });
        },
      },
    });
    vm = await slangvm.init({
      fileSystem: new OverlaySlangFileSystem([
        new MemorySlangFileSystem(sources),
        new MemorySlangFileSystem(preloadedFiles),
        libraries,
      ]),
      backend,
      print,
      printErr: print,
      onPresent: canvas ? captureFrame : undefined,
      beforeFrame: canvas ? sizeCanvas : undefined,
    });
    signal?.throwIfAborted();
    status("Compiling…");
    const module = await vm.load(filename);
    signal?.throwIfAborted();
    if (canvas) {
      sizeCanvas?.(canvas);
      window = await vm.createWindow(canvas);
    }
    signal?.throwIfAborted();
    status("Running…");
    executing = true;
    await module.main(...(window ? [window] : []));
  } catch (error) {
    if (!signal?.aborted) {
      failure = `${phase}\n${error.message || error}`;
      if (phase === "Loading slangvm…") {
        failure += "\nThe local runtime assets may be missing. See Contributing to the docs → Runnable snippets.";
      }
    }
  } finally {
    signal?.removeEventListener("abort", stop);
    if (lastFrame) send({ type: "snapshot", canvas: lastFrame });
    try { await vm?.close(); }
    catch (error) { failure = failure || `Closing slangvm: ${error.message || error}`; }
    if (signal?.aborted && signal.reason?.name !== "AbortError") {
      failure = String(signal.reason?.message || signal.reason);
    }
    if (output) send({ type: "output", text: output });
    send(failure ? { type: "error", text: failure } : { type: "done", stopped: !!signal?.aborted });
  }
}
