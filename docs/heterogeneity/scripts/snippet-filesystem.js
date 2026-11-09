import { HTTPSlangFileSystem } from "../assets/slangvm/src/http-file-system.js";

// Keep downloaded runtime bytes on the page, even when an execution worker is terminated.
const libraries = new HTTPSlangFileSystem(new URL("../assets/slangvm/slang/", import.meta.url), {
  cacheNotFound: true,
});
let preloading;

export async function preloadRuntime() {
  if (!preloading) preloading = (async () => {
    const bytes = await libraries.loadFile("standard-modules.json");
    if (bytes === null) throw new Error("Cannot load standard-module inventory");
    const manifest = JSON.parse(new TextDecoder().decode(bytes));
    // The runtime entry and its implementation modules form a self-contained import group.
    // Keep unrelated tensor/numerics modules on demand instead of downloading the entire library.
    await libraries.preload(["slang/runtime.slang-module"]);
    await libraries.preloadDirectory("slang/runtime/", { manifest });
  })().catch(error => {
    preloading = undefined;
    throw error;
  });
  await preloading;
  return libraries.snapshot();
}
