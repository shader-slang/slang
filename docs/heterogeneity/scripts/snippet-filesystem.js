import { HTTPSlangFileSystem } from "../assets/slangvm/src/http-file-system.js";

// Keep downloaded runtime bytes on the page, even when an execution worker is terminated.
const baseURL = new URL("../assets/slangvm/slang/", import.meta.url);
let libraries;
let allowedPaths;
let preloading;

export async function preloadRuntime() {
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
  return { preloadedFiles: libraries.snapshot(), allowedPaths };
}
