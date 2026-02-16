import { fileURLToPath, URL } from 'node:url'

import { defineConfig, PluginOption, PreviewServerHook, ServerHook } from 'vite'
import vue from '@vitejs/plugin-vue'
import vueJsx from '@vitejs/plugin-vue-jsx'
import vueDevTools from 'vite-plugin-vue-devtools'
import { resolve } from 'path'

function gzipFixPlugin(): PluginOption {
  const fixHeader: PreviewServerHook & ServerHook = (server) => {
    server.middlewares.use((req, res, next) => {
      if (req.originalUrl?.includes(".gz")) {
        res.setHeader("Content-Type", "application/x-gzip");
        // `res.removeHeader("Content-Encoding")` does not work
        res.setHeader("Content-Encoding", "invalid-value");
      }
      next();
    });
  };

  return {
    name: "gzip-fix-plugin",
    configureServer: fixHeader,
    // vite dev and vite preview use different server, so we need to configure both.
    configurePreviewServer: fixHeader,
  };
};

// https://vite.dev/config/
export default defineConfig({
  plugins: [
    vue(),
    gzipFixPlugin(),
    vueJsx(),
    vueDevTools(),
  ],
  base: '',
  resolve: {
    dedupe: ['vue']
  },
})
