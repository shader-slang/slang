// esbuild.config.mjs
// Basic config for building both client and server
import esbuild from 'esbuild';
import vuePlugin from 'esbuild-plugin-vue3';

async function buildAll() {
	try {
		// Build browser client
		await esbuild.build({
			entryPoints: ['client/src/browserClientMain.ts'],
			bundle: true,
			outfile: 'client/dist/browserClientMain.js',
			platform: 'browser',
			sourcemap: true,
			target: ['es2020'],
			format: 'cjs',
			tsconfig: 'client/tsconfig.json',
			define: { 'process.env.NODE_ENV': '"production"' },
			external: ['vscode'],
		});

		// Build native client
		await esbuild.build({
			entryPoints: ['client/src/nativeClientMain.ts'],
			bundle: true,
			outfile: 'client/dist/nativeClientMain.js',
			platform: 'node',
			sourcemap: true,
			target: ['es2020'],
			format: 'cjs',
			tsconfig: 'client/tsconfig.json',
			define: { 'process.env.NODE_ENV': '"production"' },
			external: ['vscode'],
		});

		// Build browser server
		await esbuild.build({
			entryPoints: ['server/src/browserServerMain.ts'],
			bundle: true,
			outfile: 'server/dist/browserServerMain.js',
			platform: 'browser',
			sourcemap: true,
			target: ['es2020'],
			format: 'cjs',
			tsconfig: 'server/tsconfig.json',
			define: { 'process.env.NODE_ENV': '"production"' },
			external: [],
			loader: {
				'.slang': 'text',
			},
		});

		// Build native server
		await esbuild.build({
			entryPoints: ['server/src/nativeServerMain.ts'],
			bundle: true,
			outfile: 'server/dist/nativeServerMain.js',
			platform: 'node',
			sourcemap: true,
			target: ['es2020'],
			format: 'cjs',
			tsconfig: 'server/tsconfig.json',
			define: { 'process.env.NODE_ENV': '"production"' },
			external: [],
			loader: {
				'.slang': 'text',
			},
		});

		// Build webview bundle
		await esbuild.build({
			entryPoints: ['webview/src/app.ts'],
			bundle: true,
			outfile: 'client/dist/webviewBundle.js',
			platform: 'browser',
			sourcemap: true,
			target: ['es2022'],
			format: 'esm',
			tsconfig: 'webview/tsconfig.json',
			define: { 'process.env.NODE_ENV': '"production"' },
			plugins: [vuePlugin()],
			external: ['vue'],
		});

		// Build uniform webview bundle
		await esbuild.build({
			entryPoints: ['uniform_webview/src/app.ts'],
			bundle: true,
			outfile: 'client/dist/uniformWebviewBundle.js',
			platform: 'browser',
			sourcemap: true,
			target: ['es2022'],
			format: 'esm',
			tsconfig: 'uniform_webview/tsconfig.json',
			define: { 'process.env.NODE_ENV': '"production"' },
			plugins: [vuePlugin()],
			external: ['vue'],
		});
	} catch (err) {
		// Use globalThis.console for ESM compatibility
		if (typeof globalThis.console !== 'undefined') {
			globalThis.console.error(err);
		}
		// Exit with error code for CI/CD (works in Node.js only)
		if (typeof globalThis.process !== 'undefined' && globalThis.process.exit) {
			globalThis.process.exit(1);
		}
		throw err;
	}
}

buildAll();
