/* Opt in with <!-- runnable: name [backend=none] [canvas=16:9] -->. */
(() => {
  "use strict";
  const workerURL = new URL("snippet-worker.js", document.currentScript.src);
  const runtimeURL = new URL("snippet-runtime.js", document.currentScript.src);
  const editorURL = new URL("../assets/editor/editor.js", document.currentScript.src);
  const mappingURL = new URL("snippet-files.json", document.currentScript.src);
  const filesystemURL = new URL("snippet-filesystem.js", document.currentScript.src);
  const loadRuntimeFiles = () => import(filesystemURL.href).then(module => module.preloadRuntime());
  // Keep HTTP misses for inventoried assets across fresh workers. Unknown import
  // candidates are rejected locally by the filesystem's fixed inventory.
  const missingFileURLs = new Set();
  const cells = new Map();
  const sourceSnapshot = () => Object.fromEntries(
    Array.from(cells, ([name, cell]) => [`${name}.slang`, cell.getValue()]));
  function sourcesChanged() {
    for (const cell of cells.values()) cell.sourcesChanged();
  }
  let editorAssets;
  function loadEditor() {
    if (!editorAssets) editorAssets = (async () => {
      const css = document.createElement("link");
      css.rel = "stylesheet";
      css.href = new URL("editor.css", editorURL).href;
      const styled = new Promise((resolve, reject) => {
        css.onload = resolve;
        css.onerror = () => reject(new Error("Cannot load editor styles"));
      });
      document.head.append(css);
      const [bundle, mappings] = await Promise.all([
        import(editorURL.href),
        fetch(mappingURL).then(response => {
          if (!response.ok) throw new Error("Cannot load snippet import mappings");
          return response.json();
        }),
        styled,
      ]);
      return { bundle, mappings };
    })();
    return editorAssets;
  }
  const comments = document.createTreeWalker(document.body, NodeFilter.SHOW_COMMENT);
  const blocks = [];
  while (comments.nextNode()) {
    const match = comments.currentNode.textContent.trim().match(/^runnable: ([a-z0-9-]+)(?: backend=(none|webgpu))?(?: canvas=([1-9][0-9]*):([1-9][0-9]*))?$/);
    const block = comments.currentNode.nextElementSibling;
    if (match && block?.matches(".codehilite")) {
      blocks.push([match[1], block, match[2] || "webgpu", match[3] ? [Number(match[3]), Number(match[4])] : null]);
    }
  }

  for (const [name, block, backend, aspect] of blocks) {
    if (cells.has(name)) throw new Error(`Duplicate runnable snippet name: ${name}`);
    const filename = `${name}.slang`;
    const code = block.querySelector("code");
    const foldLines = [];
    // Use highlighted line-comment tokens so strings and block comments stay untouched.
    for (const comment of code.querySelectorAll(".c1")) {
      if (!/^\/\/!FOLD\s*$/.test(comment.textContent)) continue;
      const prefix = document.createRange();
      prefix.selectNodeContents(code);
      prefix.setEndBefore(comment);
      foldLines.push(prefix.toString().split("\n").length);
      comment.textContent = comment.textContent.replace(/\/\/!FOLD[ \t]*/, "");
    }
    const original = code.textContent.split("\n")
      .map((line, index) => foldLines.includes(index + 1) ? line.trimEnd() : line)
      .join("\n").trimEnd();
    const card = document.createElement("section");
    card.className = "slang-snippet";
    card.dataset.snippet = name;
    card.setAttribute("aria-label", "Runnable Slang example");
    card.innerHTML = `
      <div class="snippet-filename">${filename}</div>
      <div class="snippet-source">
        <div class="snippet-actions">
          <button type="button" class="snippet-reset" title="Reset example" aria-label="Reset example">
            <svg viewBox="0 0 24 24" aria-hidden="true" focusable="false"><path d="m9 4-5 5 5 5M4 9h9a7 7 0 0 1 7 7v4"/></svg>
          </button>
          <button type="button" class="snippet-stop" title="Stop execution" aria-label="Stop execution" hidden>
            <svg viewBox="0 0 24 24" aria-hidden="true" focusable="false"><rect x="6" y="6" width="12" height="12" rx="1" fill="currentColor" stroke="none"/></svg>
          </button>
          <button type="button" class="snippet-run" title="Run (Ctrl+Enter or ⌘+Enter)" aria-label="Run example">
            <svg viewBox="0 0 24 24" aria-hidden="true" focusable="false"><path d="m8 5 11 7-11 7Z" fill="currentColor" stroke="none"/></svg>
          </button>
        </div>
        <textarea class="snippet-editor" spellcheck="false" autocapitalize="off" autocomplete="off" wrap="off"></textarea>
      </div>
      <div class="snippet-language-status" role="status" hidden></div>
      <div class="snippet-source-status" role="status" hidden>Code changed. Run again to update this result.</div>
      <div class="snippet-result" hidden>
        <div class="snippet-status" role="status"></div>
        <div class="snippet-display" hidden></div>
        <pre class="snippet-output" tabindex="0" aria-label="Program output" hidden></pre>
      </div>`;
    const fallback = card.querySelector("textarea");
    fallback.value = original;
    fallback.rows = Math.min(28, Math.max(8, original.split("\n").length));
    fallback.setAttribute("aria-label", `Slang source: ${name}`);
    let editor = {
      getValue: () => fallback.value,
      setValue: value => { fallback.value = value; sourcesChanged(); },
      setReadOnly: value => { fallback.readOnly = value; },
      dispose() {},
    };
    const languageStatus = card.querySelector(".snippet-language-status");
    const sourceStatus = card.querySelector(".snippet-source-status");
    const run = card.querySelector(".snippet-run");
    const stop = card.querySelector(".snippet-stop");
    const reset = card.querySelector(".snippet-reset");
    const result = card.querySelector(".snippet-result");
    const status = card.querySelector(".snippet-status");
    const output = card.querySelector(".snippet-output");
    const display = card.querySelector(".snippet-display");
    if (aspect) display.style.aspectRatio = `${aspect[0]} / ${aspect[1]}`;
    let worker = null;
    let canvasRun = null;
    let displaySize = null;
    let timeout;
    let transcript = "";
    let disposed = false;
    let runSources;
    function updateSourceStatus() {
      sourceStatus.hidden = !runSources || result.hidden ||
        Object.entries(sourceSnapshot()).every(([path, text]) => runSources[path] === text);
    }
    cells.set(name, {
      getValue: () => editor.getValue(),
      sourcesChanged() {
        editor.sourcesChanged?.();
        updateSourceStatus();
      },
    });
    fallback.addEventListener("input", sourcesChanged);

    function measureCanvas() {
      const width = display.getBoundingClientRect().width;
      if (!width) return null;
      const scale = Math.min(window.devicePixelRatio || 1, 2);
      return [Math.max(1, Math.round(width * scale)),
        Math.max(1, Math.round(width * aspect[1] / aspect[0] * scale))];
    }

    function observeSize() {
      if (display.hidden) return;
      const next = measureCanvas();
      if (!next) return;
      if (displaySize && (next[0] !== displaySize[0] || next[1] !== displaySize[1]) && !canvasRun) {
        // Finished output has no live VM to redraw it. Discard it instead of scaling old pixels.
        display.replaceChildren();
      }
      displaySize = next;
    }

    function sizeCanvas(canvas) {
      observeSize();
      if (!displaySize) return;
      if (canvas.width !== displaySize[0]) canvas.width = displaySize[0];
      if (canvas.height !== displaySize[1]) canvas.height = displaySize[1];
    }

    const resizeObserver = aspect ? new ResizeObserver(observeSize) : null;
    if (aspect) window.addEventListener("resize", observeSize);

    function finish(message, error = false) {
      clearTimeout(timeout);
      worker?.terminate();
      worker = null;
      canvasRun = null;
      run.disabled = reset.disabled = false;
      stop.disabled = false;
      editor.setReadOnly(false);
      run.hidden = false;
      if (document.activeElement === stop) run.focus({ preventScroll: true });
      stop.hidden = true;
      card.classList.remove("snippet-running");
      card.classList.toggle("snippet-error", error);
      status.textContent = message;
      updateSourceStatus();
    }

    function failure(message) {
      output.hidden = false;
      output.textContent = transcript + (transcript ? "\n" : "") + message;
      finish("Could not run this example", true);
    }

    function execute() {
      if (worker || canvasRun) return;
      runSources = sourceSnapshot();
      sourceStatus.hidden = true;
      transcript = "";
      output.textContent = "";
      output.hidden = true;
      display.replaceChildren();
      display.hidden = true;
      card.classList.remove("snippet-error");
      card.classList.add("snippet-running");
      result.hidden = false;
      status.textContent = "Starting…";
      const focusStop = document.activeElement === run;
      run.disabled = reset.disabled = true;
      editor.setReadOnly(true);
      run.hidden = true;
      stop.hidden = false;
      if (focusStop) stop.focus({ preventScroll: true });
      try {
        const receive = (data) => {
          if (data.type === "missing-file") missingFileURLs.add(data.url);
          if (data.type === "status") {
            status.textContent = data.text;
            // Canvas programs run until they return or the reader clicks Stop.
            if (aspect && data.text === "Running…") clearTimeout(timeout);
          }
          if (data.type === "snapshot") {
            data.canvas.setAttribute("aria-label", `Rendered output: ${name}`);
            observeSize();
            // A one-shot program may finish after the container changed size during execution.
            // In that case show white, just like resizing already-completed output.
            if (displaySize && data.canvas.width === displaySize[0] && data.canvas.height === displaySize[1]) {
              display.replaceChildren(data.canvas);
            } else display.replaceChildren();
          }
          if (data.type === "output") {
            transcript = data.text;
            output.hidden = false;
            output.textContent = transcript;
          }
          if (data.type === "done") {
            output.hidden = !transcript && (!!aspect || data.stopped);
            output.textContent = transcript || "Program completed without printing output.";
            finish(data.stopped ? "Stopped" : "Finished");
          }
          if (data.type === "error") failure(data.text);
        };
        if (aspect) {
          const controller = new AbortController();
          canvasRun = controller;
          const canvas = document.createElement("canvas");
          canvas.setAttribute("aria-label", `Interactive output: ${name}`);
          display.append(canvas);
          display.hidden = false;
          sizeCanvas(canvas);
          timeout = setTimeout(() => {
            status.textContent = "Loading timed out; stopping…";
            controller.abort(new Error("Loading exceeded two minutes. Try running again."));
          }, 120000);
          Promise.all([import(runtimeURL.href), loadRuntimeFiles()]).then(([{ runSnippet }, runtimeFiles]) => runSnippet(
            { filename, sources: runSources, backend, canvas, sizeCanvas, ...runtimeFiles, missingFileURLs: [...missingFileURLs] },
            data => { if (canvasRun === controller) receive(data); },
            controller.signal,
          )).catch(error => {
            if (canvasRun === controller) failure(String(error.message || error));
          });
          return;
        }
        const current = new Worker(workerURL, { type: "module" });
        worker = current;
        current.onmessage = ({ data }) => {
          if (worker !== current) return;
          receive(data);
        };
        current.onerror = (event) => {
          event.preventDefault();
          if (worker === current) failure(event.message || "The browser worker could not start.");
        };
        current.onmessageerror = () => {
          if (worker === current) failure("Could not receive the worker's result.");
        };
        timeout = setTimeout(() => failure("Execution exceeded two minutes. Try a smaller program."), 120000);
        loadRuntimeFiles().then(runtimeFiles => {
          if (worker === current) current.postMessage({
            filename, sources: runSources, backend, ...runtimeFiles, missingFileURLs: [...missingFileURLs],
          });
        }).catch(error => {
          if (worker === current) failure(String(error.message || error));
        });
      } catch (error) {
        failure(String(error.message || error));
      }
    }

    run.addEventListener("click", execute);
    stop.addEventListener("click", () => {
      if (canvasRun) {
        clearTimeout(timeout);
        status.textContent = "Stopping…";
        stop.disabled = true;
        canvasRun.abort();
      } else finish("Stopped");
    });
    reset.addEventListener("click", () => {
      runSources = undefined;
      editor.setValue(original, foldLines);
      transcript = "";
      output.textContent = "";
      output.hidden = true;
      finish("");
      display.replaceChildren();
      display.hidden = true;
      result.hidden = true;
    });
    fallback.addEventListener("keydown", (event) => {
      if (event.key === "Enter" && (event.ctrlKey || event.metaKey)) {
        event.preventDefault();
        execute();
      }
    });
    window.addEventListener("pagehide", event => {
      canvasRun?.abort();
      finish("Stopped");
      // A persisted page remains usable when restored from the browser's back/forward cache.
      if (!event.persisted) {
        disposed = true;
        resizeObserver?.disconnect();
        window.removeEventListener("resize", observeSize);
        editor.dispose();
      }
    });
    block.replaceWith(card);
    resizeObserver?.observe(display);
    void loadEditor().then(({ bundle, mappings }) => {
      if (disposed) return;
      if (!mappings[name]) throw new Error(`No import mapping for ${name}`);
      const files = Object.fromEntries(Object.entries(mappings[name]).map(([path, url]) =>
        [path, new URL(url, mappingURL).href]));
      const container = document.createElement("div");
      container.className = "snippet-monaco";
      fallback.before(container);
      try {
        editor = bundle.createEditor(container, {
          source: fallback.value, name, filename, files, onRun: execute,
          getSources: sourceSnapshot, onChange: sourcesChanged,
          standardModulesURL: new URL("../assets/slangvm/slang/standard-modules.json", mappingURL).href,
          foldLines: fallback.value === original ? foldLines : [],
          onStatus(text, error = false) {
            languageStatus.hidden = false;
            languageStatus.textContent = text;
            languageStatus.classList.toggle("snippet-language-error", error);
          },
        });
      } catch (error) { container.remove(); throw error; }
      editor.setReadOnly(!!worker || !!canvasRun);
      fallback.remove();
    }).catch(error => {
      languageStatus.hidden = false;
      languageStatus.textContent = `Editor unavailable: ${error.message}. Plain text editing is available.`;
      languageStatus.classList.add("snippet-language-error");
    });
  }
})();
