// Each worker owns one synchronous slangd instance and its standard Emscripten FS.
let module;
let server;
let uri;
const documents = new Map();

// Embind vectors cannot cross postMessage and must be released after copying.
function vector(values, convert = value => value) {
    if (!values) return [];
    try {
        return Array.from({ length: values.size() }, (_, i) => convert(values.get(i)));
    } finally {
        values.delete();
    }
}

function completion(item) {
    return { ...item, commitCharacters: vector(item.commitCharacters) };
}

async function initialize(data) {
    // The package inventory is authoritative for its standard-module dependencies.
    // Site mappings add custom files; editable cells override both below.
    let fileURLs = {};
    if (data.standardModulesURL) {
        const response = await fetch(data.standardModulesURL);
        if (!response.ok) throw new Error("Cannot load standard-module inventory");
        fileURLs = Object.fromEntries((await response.json()).map(name => {
            const path = name.replace(/\.slang-module$/, ".slang");
            return [`/${path}`, new URL(path, data.standardModulesURL).href];
        }));
    }
    Object.assign(fileURLs, data.files);
    const [factory, files] = await Promise.all([
        import(/* @vite-ignore */ new URL("../slangd/slang-wasm.js", import.meta.url).href),
        Promise.all(Object.entries(fileURLs).map(async ([name, url]) => {
            // Mappings are site-authored, absolute virtual paths; never interpret URL paths as FS paths.
            if (!name.startsWith("/") || name.split("/").some(part => part === ".." || part === ".")) {
                throw new Error(`Invalid virtual file path: ${name}`);
            }
            const response = await fetch(url);
            if (!response.ok) throw new Error(`Cannot load ${name}: HTTP ${response.status}`);
            return [name, new Uint8Array(await response.arrayBuffer())];
        })),
    ]);
    module = await factory.default();
    for (const [name, bytes] of files) {
        module.FS.mkdirTree(name.slice(0, name.lastIndexOf("/")) || "/");
        module.FS.writeFile(name, bytes);
    }
    uri = data.uri;
    for (const [name, text] of Object.entries(data.sources ?? {})) {
        documents.set(new URL(name, uri).href, text);
    }
    documents.set(uri, data.source);
    for (const [documentURI, text] of documents) {
        const path = new URL(documentURI).pathname;
        module.FS.mkdirTree(path.slice(0, path.lastIndexOf("/")) || "/");
        module.FS.writeFile(path, text);
    }
    // Workspace initialization enumerates the filesystem, so all imports must exist first.
    server = module.createLanguageServer();
    server.setEnableExperimentalFeatures(true);
    for (const [documentURI, text] of documents) server.didOpenTextDocument(documentURI, text);
}

function update(documentURI, text) {
    const previous = documents.get(documentURI);
    if (text === previous) return;
    const path = new URL(documentURI).pathname;
    module.FS.mkdirTree(path.slice(0, path.lastIndexOf("/")) || "/");
    module.FS.writeFile(path, text);
    if (previous === undefined) {
        server.didOpenTextDocument(documentURI, text);
        documents.set(documentURI, text);
        return;
    }
    const lines = previous.split("\n");
    const edits = new module.TextEditList();
    try {
        edits.push_back({
            range: { start: { line: 0, character: 0 }, end: {
                line: lines.length - 1, character: lines.at(-1).length,
            } },
            text,
        });
        server.didChangeTextDocument(documentURI, edits);
        documents.set(documentURI, text);
    } finally {
        edits.delete();
    }
}

async function request(data) {
    if (data.method === "initialize") return initialize(data);
    if (!server) throw new Error("Language service is not initialized");
    for (const [name, text] of Object.entries(data.sources ?? {})) update(new URL(name, uri).href, text);
    update(uri, data.source);
    switch (data.method) {
        case "diagnostics": return vector(server.getDiagnostics(uri));
        case "hover": return server.hover(uri, data.position);
        case "completion": return vector(server.completion(uri, data.position, data.context), completion);
        case "signatureHelp": {
            const help = server.signatureHelp(uri, data.position);
            if (!help) return null;
            return { ...help, signatures: vector(help.signatures, signature => ({
                ...signature, parameters: vector(signature.parameters),
            })) };
        }
        default: throw new Error(`Unknown language request: ${data.method}`);
    }
}

// Initialization fetches asynchronously; queue later requests until the FS and server are ready.
let queue = Promise.resolve();
self.onmessage = ({ data }) => {
    queue = queue.then(async () => {
        try {
            self.postMessage({ id: data.id, result: await request(data) });
        } catch (error) {
            self.postMessage({ id: data.id, error: String(error.message || error) });
        }
    });
};
