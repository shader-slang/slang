import { HTTPSlangFileSystem } from "../assets/slangvm/src/http-file-system.js";
import { normalizePath } from "../assets/slangvm/src/file-system.js";

// Keep downloaded runtime bytes on the page, even when an execution worker is terminated.
const baseURL = new URL("../assets/slangvm/slang/", import.meta.url);
let libraries;
let allowedPaths;
let preloading;
const mappedFiles = new Map();

export async function preloadRuntime(files = {}) {
  if (!preloading) preloading = (async () => {
    const inventory = new HTTPSlangFileSystem(baseURL, { allowedPaths: ["standard-modules.json"] });
    const bytes = await inventory.loadFile("standard-modules.json");
    if (bytes === null) throw new Error("Cannot load standard-module inventory");
    const manifest = JSON.parse(new TextDecoder().decode(bytes));
    allowedPaths = manifest.flatMap(path => [path, path.replace(/\.slang-module$/, ".slang")]);
    libraries = new HTTPSlangFileSystem(baseURL, { allowedPaths, cacheNotFound: true });
    // The runtime entry and its implementation modules form a self-contained import group.
    // Keep unrelated tensor/numerics modules on demand instead of downloading the entire library.
    await libraries.preload(["slang/runtime.slang-module"]);
    await libraries.preloadDirectory("slang/runtime/", { manifest });
  })().catch(error => {
    preloading = undefined;
    throw error;
  });
  await preloading;
  const preloadedFiles = libraries.snapshot();
  await Promise.all(Object.entries(files).map(async ([path, url]) => {
    const name = normalizePath(path);
    // Legacy editor mappings name standard-library sources. Execution already has
    // their compiled modules; do not fetch sources or enable source fallback here.
    const libraryFile = url === new URL(name.slice(1), baseURL).href;
    if (libraryFile && (Object.hasOwn(preloadedFiles, name)
        || Object.hasOwn(preloadedFiles, name.replace(/\.slang$/, ".slang-module")))) return;
    if (!mappedFiles.has(url)) {
      mappedFiles.set(url, fetch(url).then(async response => {
        if (!response.ok) throw new Error(`Cannot load snippet file: ${path}`);
        return new Uint8Array(await response.arrayBuffer());
      }).catch(error => {
        mappedFiles.delete(url); // A later Run can retry a failed download.
        throw error;
      }));
    }
    preloadedFiles[name] = await mappedFiles.get(url);
  }));
  return { preloadedFiles, allowedPaths };
}
