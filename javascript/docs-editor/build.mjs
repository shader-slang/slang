import { build } from "esbuild";
import { copyFile, mkdir } from "node:fs/promises";
import { fileURLToPath } from "node:url";

const outdir = fileURLToPath(new URL("../../build/docs-editor/", import.meta.url));
await mkdir(outdir, { recursive: true });
await build({
    absWorkingDir: fileURLToPath(new URL(".", import.meta.url)),
    entryPoints: {
        editor: "src/editor.js",
        "editor.worker": "node_modules/monaco-editor/esm/vs/editor/editor.worker.js",
        "language.worker": "src/language.worker.js",
    },
    outdir, bundle: true, format: "esm", target: "es2022", minify: true,
    loader: { ".ttf": "file" }, logLevel: "info",
});
await copyFile(new URL("node_modules/monaco-editor/LICENSE", import.meta.url), `${outdir}/MONACO-LICENSE.txt`);
await copyFile(new URL("node_modules/monaco-editor/ThirdPartyNotices.txt", import.meta.url), `${outdir}/MONACO-NOTICES.txt`);
await copyFile(new URL("../../LICENSE", import.meta.url), `${outdir}/SLANG-LICENSE.txt`);
