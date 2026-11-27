<script setup lang="ts">
import { useTemplateRef, onMounted, ref, shallowRef, onUnmounted } from 'vue'
import * as monaco from 'monaco-editor';
import { initLanguageServer, initMonaco, initMonacoLanguages, translateSeverity, userCodeURI } from '../language-server';
import { compiler, slangd } from '../try-slang';

const container = useTemplateRef('container')
const editor = shallowRef<monaco.editor.IStandaloneCodeEditor>();

let diagnosticTimeout: number | null = null;


function setEditorValue(value: string, revealEnd: boolean = false) {
	editor.value!.setValue(value);
	if (revealEnd)
		editor.value!.revealLine(editor.value!.getModel()?.getLineCount() || 0);
	else
		editor.value!.revealLine(0);
}

function appendEditorValue(value: string, revealEnd: boolean = false) {
	setEditorValue(editor.value!.getValue() + value, revealEnd);
}

function getValue() {
	return editor.value!.getValue();
}

function setLanguage(language: string) {
	let model = editor.value!.getModel();
	if (model == null) {
		throw new Error("Could not get editor model");
	}
	monaco.editor.setModelLanguage(model, language);
}

defineExpose({
	setEditorValue,
	appendEditorValue,
	getValue,
	setLanguage,
})


let { readOnlyMode } = defineProps<{
	readOnlyMode?: boolean
}>()

onMounted(() => {
	const preloadCode = "";
	const wordWrap = "off";
	if (container.value == null) {
		throw new Error("Could not find container for editor");
	}
	initMonaco();
	let model = readOnlyMode
		? monaco.editor.createModel(preloadCode)
		: monaco.editor.createModel("", "slang", monaco.Uri.parse(userCodeURI));
	editor.value = monaco.editor.create(container.value, {
		model: model,
		language: readOnlyMode ? 'csharp' : 'slang',
		theme: 'slang-dark',
		readOnly: readOnlyMode,
		lineNumbers: readOnlyMode ? "off" : "on",
		automaticLayout: true,
		wordWrap: wordWrap,
		"semanticHighlighting.enabled": true,
		renderValidationDecorations: "on",
		minimap: {
			enabled: false
		},
	});
	if (!readOnlyMode) {
		model.onDidChangeContent(codeEditorChangeContent);
		model.setValue(preloadCode);
	}
	async function setupMonaco() {
		initLanguageServer();
		initMonacoLanguages();
	}
	// Wait for WASM to be loaded before initializing Monaco and the language server
	if (slangd || compiler) {
		setupMonaco();
	} else {
		window.addEventListener('slangLoaded', setupMonaco, { once: true });
	}
})

onUnmounted(() => {
	if (editor.value != null) {
		editor.value.getModel()?.dispose();
		editor.value.dispose();
	}
})

function codeEditorChangeContent(e: monaco.editor.IModelContentChangedEvent) {
	if (slangd == null)
		return;
	if (compiler == null) {
		throw new Error("Compiler is undefined!");
	}
	let lspChanges = new compiler.slangWasmModule.TextEditList();

	{
		const FS = compiler.slangWasmModule.FS;
		FS.writeFile("/user.slang", editor.value!.getValue());
	}

	e.changes.forEach(change =>
		lspChanges.push_back(
			{
				range: {
					start: { line: change.range.startLineNumber - 1, character: change.range.startColumn - 1 },
					end: { line: change.range.endLineNumber - 1, character: change.range.endColumn - 1 }
				},
				text: change.text
			}
		));
	try {
		slangd.didChangeTextDocument(userCodeURI, lspChanges);
		if (diagnosticTimeout != null) {
			clearTimeout(diagnosticTimeout);
		}
		diagnosticTimeout = setTimeout(() => {
			if (slangd == null) {
				throw new Error("Slang is undefined!");
			}
			let diagnostics = slangd.getDiagnostics(userCodeURI);
			let model = editor.value?.getModel();
			if (model == null) {
				throw new Error("Could not get editor model");
			}
			if (diagnostics == null) {
				monaco.editor.setModelMarkers(model, "slang", []);
				return;
			}
			let markers: monaco.editor.IMarkerData[] = [];
			for (let i = 0; i < diagnostics.size(); i++) {
				let lspDiagnostic = diagnostics.get(i);
				if (lspDiagnostic == undefined) {
					throw new Error("Invalid state!");
				}
				markers.push({
					startLineNumber: lspDiagnostic.range.start.line + 1,
					startColumn: lspDiagnostic.range.start.character + 1,
					endLineNumber: lspDiagnostic.range.end.line + 1,
					endColumn: lspDiagnostic.range.end.character + 1,
					message: lspDiagnostic.message.toString(),
					severity: translateSeverity(lspDiagnostic.severity),
					code: lspDiagnostic.code.toString()
				});
			}
			monaco.editor.setModelMarkers(model, "slang", markers);
			diagnosticTimeout = null;
		}, 500);

	} catch (e) {
		console.error(e);
	}
	finally {
		lspChanges.delete();
	}

}
</script>

<template>
	<div class="editorContainer" ref="container"></div>
</template>

<style scoped>
.editorContainer {
	background-color: var(--panel-background);
	height: 100%;
}
</style>