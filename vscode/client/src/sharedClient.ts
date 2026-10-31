import { ExtensionContext, Uri, commands, window, workspace } from 'vscode';
import * as vscode from 'vscode';

let slangLogChannel: vscode.OutputChannel | undefined;
function getSlangLogChannel(): vscode.OutputChannel {
	if (!slangLogChannel) {
		slangLogChannel = vscode.window.createOutputChannel('Slang Extension Log');
	}
	return slangLogChannel;
}

import type { CompiledPlayground, CompileRequest, EntrypointsRequest, EntrypointsResult, PlaygroundMessage, Result, Shader, UniformController } from 'slang-playground-shared';
import { isControllerRendered, RUNNABLE_ENTRY_POINT_NAMES } from "slang-playground-shared";

// Maps to track open panels by file URI and command type
const playgroundPanels = new Map<string, vscode.WebviewPanel>();
const uniformPanels = new Map<string, vscode.WebviewPanel>();
const outputPanels = new Map<string, vscode.OutputChannel>();

const compileOptions = ['SPIRV', 'HLSL', 'GLSL', 'METAL', 'WGSL', 'CUDA'] as const;
type LanguageOptions = {
	languageId: string,
	requiresEntrypoint: boolean,
}
const compileOptionMap: { [k in (typeof compileOptions)[number]]: LanguageOptions } = {
	SPIRV: {
		languageId: 'spirv',
		requiresEntrypoint: false,
	},
	HLSL: {
		languageId: 'hlsl',
		requiresEntrypoint: true,
	},
	GLSL: {
		languageId: 'glsl',
		requiresEntrypoint: true,
	},
	METAL: {
		languageId: 'metal',
		requiresEntrypoint: false,
	},
	WGSL: {
		languageId: 'wgsl',
		requiresEntrypoint: false,
	},
	CUDA: {
		languageId: 'cuda-cpp',
		requiresEntrypoint: true
	}
}

export async function getSlangFilesWithContents(): Promise<{ uri: string, content: string }[]> {
	const pattern = '**/*.slang';
	const files = await vscode.workspace.findFiles(pattern);

	const results: { uri: string, content: string }[] = [];

	for (const uri of files) {
		try {
			const document = await vscode.workspace.openTextDocument(uri);
			results.push({ uri: uri.toString(false), content: document.getText() });
		} catch (err) {
			const logChannel = getSlangLogChannel();
			logChannel.appendLine(`Failed to read ${uri.fsPath}: ${err}`);
			logChannel.show(true);
		}
	}

	return results;
}

export type SlangHandler = {
	compileShader: (parameter: CompileRequest) => Promise<Result<Shader>>;
	compilePlayground: (parameter: CompileRequest & { uri: string }) => Promise<Result<CompiledPlayground>>;
	entrypoints: (parameter: EntrypointsRequest) => Promise<EntrypointsResult>;
};

// this method is called when vs code is activated
export async function sharedActivate(context: ExtensionContext, slangHandler: SlangHandler) {
	// Register Playground Run command to open a webview
	context.subscriptions.push(
		commands.registerCommand('slang.playgroundRun', async () => {
			const userSource = window.activeTextEditor.document.getText();
			const userURI = window.activeTextEditor.document.uri;
			const shaderType = checkShaderType(userSource);
			if (shaderType == null) {
				vscode.window.showErrorMessage("Error: In order to run the shader, please define either imageMain or printMain function in the shader code.");
				return;
			}
			const compileResult = await slangHandler.compilePlayground({
				target: "WGSL",
				entrypoint: shaderType,
				sourceCode: userSource,
				shaderPath: window.activeTextEditor.document.uri.toString(false),
				noWebGPU: false,
				uri: userURI.toString(),
			});
			if (compileResult.succ == false) {
				const logChannel = getSlangLogChannel();
				vscode.window.showErrorMessage(compileResult.message);
				if (compileResult.log) {
					logChannel.appendLine(compileResult.log);
					logChannel.show(true);
				}
				return;
			}
			const compilation = compileResult.result;

			// Key for this file/run
			const playgroundKey = userURI.toString() + ':playground';
			// Close previous panel if exists
			if (playgroundPanels.has(playgroundKey)) {
				try { playgroundPanels.get(playgroundKey)!.dispose(); } catch { }
				playgroundPanels.delete(playgroundKey);
			}
			const panel = window.createWebviewPanel(
				'slangPlayground',
				'Slang Playground',
				vscode.ViewColumn.Beside,
				{
					enableScripts: true,
					retainContextWhenHidden: true,
				}
			);
			playgroundPanels.set(playgroundKey, panel);
			panel.onDidDispose(() => playgroundPanels.delete(playgroundKey));
			panel.webview.html = getWebviewContent(context, panel, 'client/dist/webviewBundle.js', 'client/dist/webviewBundle.css');

			if (shaderType === 'printMain') {
				const outputKey = userURI.toString() + ':output';
				if (outputPanels.has(outputKey)) {
					try { outputPanels.get(outputKey)!.dispose(); } catch { }
					outputPanels.delete(outputKey);
				}
				const shaderOutputLog = vscode.window.createOutputChannel(`Slang Shader Output (${window.activeTextEditor.document.fileName})`);
				outputPanels.set(outputKey, shaderOutputLog);
				panel.webview.onDidReceiveMessage(message => {
					if (message.type === 'log') {
						shaderOutputLog.append(message.text);
						shaderOutputLog.show(true);
					}
				});
				panel.onDidDispose(() => {
					shaderOutputLog.dispose();
					outputPanels.delete(outputKey);
				});
			}

			let message: PlaygroundMessage = {
				type: "init",
				payload: compilation,
			};
			panel.webview.postMessage(message)

			if (compilation.uniformComponents.some(isControllerRendered)) {
				const uniformKey = userURI.toString() + ':uniform';
				if (uniformPanels.has(uniformKey)) {
					try { uniformPanels.get(uniformKey)!.dispose(); } catch { }
					uniformPanels.delete(uniformKey);
				}
				const uniform_panel = window.createWebviewPanel(
					'slangPlaygroundUniforms',
					'Slang Playground Uniforms',
					vscode.ViewColumn.Beside,
					{
						enableScripts: true,
						retainContextWhenHidden: true,
					}
				);
				uniformPanels.set(uniformKey, uniform_panel);
				uniform_panel.onDidDispose(() => uniformPanels.delete(uniformKey));
				uniform_panel.webview.html = getWebviewContent(context, uniform_panel, "client/dist/uniformWebviewBundle.js", "client/dist/uniformWebviewBundle.css");
				uniform_panel.webview.onDidReceiveMessage(uniform_message => {
					if (uniform_message.type === 'update') {
						let playground_message: PlaygroundMessage = {
							type: "uniformUpdate",
							payload: uniform_message.data,
						};
						panel.webview.postMessage(playground_message)
					}
				});
				uniform_panel.webview.postMessage({
					type: "init",
					uniformComponents: compilation.uniformComponents,
				})

				panel.onDidDispose(() => {
					try {
						uniform_panel.dispose()
					} catch {
						// ignore if panel was already disposed
					}
					uniformPanels.delete(uniformKey);
				})
			}
		})
	);

	vscode.window.onDidChangeActiveTextEditor(editor => {
		updateContext(editor);
	});

	vscode.workspace.onDidChangeTextDocument(event => {
		if (vscode.window.activeTextEditor?.document === event.document) {
			updateContext(vscode.window.activeTextEditor);
		}
	});

	updateContext(window.activeTextEditor);

	function updateContext(editor: vscode.TextEditor | undefined) {
		const text = editor?.document.getText() || "";
		const shouldShow = text.match("void\\s+printMain\\s*\\(") != null || text.match("float4\\s+imageMain\\s*\\(") != null;
		vscode.commands.executeCommand('setContext', 'isPlaygroundFile', shouldShow);
	}

	// Register a virtual document content provider for readonly docs
	const slangVirtualScheme = 'slang-virtual';
	const virtualDocumentContents = new Map();

	context.subscriptions.push(
		workspace.registerTextDocumentContentProvider(slangVirtualScheme, {
			provideTextDocumentContent: (uri) => {
				return virtualDocumentContents.get(uri.path.slice(1));
			}
		})
	);

	// Register the user command
	context.subscriptions.push(commands.registerCommand('slang.compile', async () => {
		const targetSelection = await window.showQuickPick(compileOptions, {
			placeHolder: 'Select a Target',
		}) as (typeof compileOptions)[number] | undefined;
		if (!targetSelection) {
			return;
		}
		const userSource = window.activeTextEditor.document.getText() ?? '';
		let selectedEntrypoint = ""
		if (compileOptionMap[targetSelection].requiresEntrypoint) {
			// Send the picked option to the server and get the result
			const parameter: EntrypointsRequest = {
				sourceCode: userSource,
				shaderPath: window.activeTextEditor.document.uri.toString(false),
			}
			let entrypoints: EntrypointsResult = await slangHandler.entrypoints(parameter);
			const entrypointSelection = await window.showQuickPick(entrypoints, {
				placeHolder: 'Select a Entrypoint',
			}) as (typeof compileOptions)[number] | undefined;
			if (!entrypointSelection) {
				return;
			}
			selectedEntrypoint = entrypointSelection;
		}
		// Send the picked option to the server and get the result
		let compilationResult = await slangHandler.compileShader({
			target: targetSelection,
			entrypoint: selectedEntrypoint,
			sourceCode: userSource,
			shaderPath: window.activeTextEditor.document.uri.toString(false),
			noWebGPU: true,
		});
		if (compilationResult.succ == false) {
			vscode.window.showErrorMessage(compilationResult.message);
			return;
		}
		const shader = compilationResult.result;
		const vDocName = `Slang Compile (${targetSelection})`
		// Show the result in a readonly virtual document
		const vdocUri = Uri.parse(`${slangVirtualScheme}:/${vDocName}`);
		virtualDocumentContents.set(vDocName, shader.code);
		const doc = await workspace.openTextDocument(vdocUri);
		await window.showTextDocument(doc, { preview: false, viewColumn: vscode.ViewColumn.Beside }).then(editor => {
			vscode.languages.setTextDocumentLanguage(doc, compileOptionMap[targetSelection].languageId);
		});
	}));

	context.subscriptions.push(commands.registerCommand('slang.reflection', async () => {
		const userSource = window.activeTextEditor.document.getText() ?? '';
		// Send the picked option to the server and get the result
		let compilationResult = await slangHandler.compileShader({
			target: "WGSL",
			entrypoint: "",
			sourceCode: userSource,
			shaderPath: window.activeTextEditor.document.uri.toString(false),
			noWebGPU: true,
		});
		if (compilationResult.succ == false) {
			vscode.window.showErrorMessage(compilationResult.message);
			return;
		}
		const shader = compilationResult.result;
		const vDocName = `Slang Reflection (${window.activeTextEditor.document.fileName.replace('\\', "")})`
		// Show the result in a readonly virtual document
		const vdocUri = Uri.parse(`${slangVirtualScheme}:/${vDocName}`);
		virtualDocumentContents.set(vDocName, JSON.stringify(shader.reflection, undefined, 4));
		const doc = await workspace.openTextDocument(vdocUri);
		await window.showTextDocument(doc, { preview: false, viewColumn: vscode.ViewColumn.Beside }).then(editor => {
			vscode.languages.setTextDocumentLanguage(doc, "json");
		});
	}));

	context.subscriptions.push(commands.registerCommand('slang.playgroundDocumentation', async () => {
		const mdFile = vscode.Uri.joinPath(context.extensionUri, 'media', 'playgroundDocumentation.md')
		await vscode.commands.executeCommand('markdown.showPreviewToSide', mdFile);
	}));
}

	export function getWebviewContent(context: ExtensionContext, panel: vscode.WebviewPanel, scriptPath: string, stylePath: string): string {
	// Webview HTML with script tag for the esbuild webview bundle
	const webviewMain = panel.webview.asWebviewUri(Uri.joinPath(context.extensionUri, scriptPath));
	const webviewStyle = panel.webview.asWebviewUri(Uri.joinPath(context.extensionUri, stylePath));
	const vueSrc = panel.webview.asWebviewUri(Uri.joinPath(context.extensionUri, 'node_modules/vue/dist/vue.esm-browser.prod.js'));

	return `
	<!DOCTYPE html>
	<html lang="en">
	<head>
		<meta charset="UTF-8">
		<meta name="viewport" content="width=device-width, initial-scale=1.0">
		<title>Slang Playground</title>
      	<link rel="stylesheet" href="${webviewStyle.toString(true)}">
		<script type="importmap">
			{
				"imports": {
					"vue": "${vueSrc.toString(true)}"
				}
			}
		</script>
		<script type="module" src="${webviewMain.toString(true)}"></script>
		<style>
			#app {
			    position: absolute;
				left: 0;
				right: 0;
				bottom: 0;
				top: 0;
			}
		</style>
	</head>
	<body>
	<div id="app">
		<p>Loading</p>
	</div>
	</body>
	</html>
`;
}

export function checkShaderType(userSource: string) {
    // we did a pre-filter on the user input source code.
    let shaderTypes = RUNNABLE_ENTRY_POINT_NAMES.filter((entryPoint) => userSource.includes(entryPoint));

    // Only one of the main function should be defined.
    // In this case, we will know that the shader is not runnable, so we can only compile it.
    if (shaderTypes.length !== 1)
        return null;

    return shaderTypes[0];
}
