import { runSnippet } from "./snippet-runtime.js";

// One worker and one immutable VM source snapshot per Run. Termination also stops CPU loops.
self.onmessage = ({ data }) => runSnippet(data, message => self.postMessage(message));
