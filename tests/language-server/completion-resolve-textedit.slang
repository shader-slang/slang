//TEST:LANG_SERVER(filecheck=CHECK):

// completionItem/resolve must answer with the completion item, never null, even for an item that
// carries a textEdit but no `data` to enrich (file/import completions) — returning null there
// breaks the LSP resolve contract for strict clients.

int x;

//RESOLVE
// CHECK: label: resolveItem
// CHECK: textEdit: resolveItem
// CHECK: range: 5,0 - 5,3
