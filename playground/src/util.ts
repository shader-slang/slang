import pako from "pako";

export function isWebGPUSupported() {
    return 'gpu' in navigator && navigator.gpu !== null;
}

// Function to compress and decompress text, loading pako if necessary
export async function compressToBase64URL(text: string) {
    // Compress the text
    const compressed = pako.deflate(text, {});
    const base64 = btoa(String.fromCharCode(...new Uint8Array(compressed)));

    return base64.replace(/\+/g, '-').replace(/\//g, '_').replace(/=+$/, '');
}

export async function decompressFromBase64URL(base64: string) {
    // Decode the base64 URL
    base64 = base64.replace(/-/g, '+').replace(/_/g, '/');
    const compressed = Uint8Array.from(atob(base64), c => c.charCodeAt(0));
    // Decompress the data
    const decompressed = pako.inflate(compressed, { to: 'string' });

    return decompressed;
}

export async function fetchWithProgress(url: string, onProgress: { (loaded: number, total: number): void; }) {
    const response = await fetch(url);
    const contentLength = response.headers.get('Content-Length');

    if (!contentLength) {
        console.warn('Content-Length header is missing.');
        return new Uint8Array(await response.arrayBuffer());
    }

    let total = contentLength ? parseInt(contentLength, 10) : 8 * 1024 * 1024; // Default to 8 MB if unknown
    let buffer = new Uint8Array(total); // Initial buffer
    let position = 0; // Tracks the current position in the buffer

    if (!response.body) {
        // Probably needs to be handled properly
        throw new Error("No response body");
    }
    const reader = response.body.getReader();

    while (true) {
        const { done, value } = await reader.read();
        if (done) break;
        // Ensure buffer has enough space for the next chunk
        if (position + value.length > buffer.length) {
            // Double the buffer size
            let newBuffer = new Uint8Array(buffer.length * 2);
            newBuffer.set(buffer, 0); // Copy existing data to the new buffer
            buffer = newBuffer;
        }

        // Copy the chunk into the buffer
        buffer.set(value, position);
        position += value.length;

        if (contentLength) {
            onProgress(position, total); // Update progress if content length is known
        }
    }

    return buffer;
}