import { TextDocumentContentChangeEvent } from 'vscode-languageserver';
import type { MainModule } from '../../media/slang-wasm.worker.js';

// Helper to resolve the correct URL for the WASM and JS files
export function removePrefix(data: string, prefix: string): string {
	if (data.startsWith(prefix))
		data = data.slice(prefix.length)
	return data;
}

export function getEmscriptenURI(uri: string, workspaceURIs: string[]): string {
	uri = uri.replace(/^([a-zA-Z\-]+:\/\/\/?)/, ""); // remove protocol
	for (const workspaceUri of workspaceURIs) {
		uri = removePrefix(uri, workspaceUri.replaceAll('\\', '/').replaceAll(':', '%3A'));
	}
	return uri;
}

export function getSlangdURI(uri: string, workspaceURIs: string[]): string {
	return `file:///${getEmscriptenURI(uri, workspaceURIs)}`;
}

function applyIncrementalChange(
	text: string,
	change: TextDocumentContentChangeEvent
): string {
	if (!TextDocumentContentChangeEvent.isIncremental(change)) {
		return change.text;
	}
	const lines = text.split('\n');

	const startLine = change.range.start.line;
	const startChar = change.range.start.character;
	const endLine = change.range.end.line;
	const endChar = change.range.end.character;

	const before = lines.slice(0, startLine);
	const after = lines.slice(endLine + 1);

	const startLineText = lines[startLine] ?? '';
	const endLineText = lines[endLine] ?? '';

	const prefix = startLineText.substring(0, startChar);
	const suffix = endLineText.substring(endChar);

	const newLines = (change.text || '').split('\n');
	const middle = [...newLines];
	if (middle.length > 0) {
		middle[0] = prefix + middle[0];
		middle[middle.length - 1] = middle[middle.length - 1] + suffix;
	}

	return [...before, ...middle, ...after].join('\n');
}

export function modifyEmscriptenFile(uri: string, changes: TextDocumentContentChangeEvent[], slangWasmModule: MainModule) {
	let content = slangWasmModule.FS.readFile(uri, {encoding: 'utf8'}).toString();
	for (const change of changes) {
		content = applyIncrementalChange(content, change)
	}
	slangWasmModule.FS.writeFile(uri, content);
}