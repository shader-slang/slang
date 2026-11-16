// Markdown links and shared fragments use the same cell navigation path.
export function installCellLinks(cells) {
  let revision = 0;
  let panel;
  const cancel = () => {
    revision++;
    if (panel) panel.hidden = true;
  };
  async function navigate() {
    cancel();
    let id;
    try { id = decodeURIComponent(location.hash.slice(1)); }
    catch { return; }
    const target = document.getElementById(id);
    const filename = target?.dataset.cellTarget;
    const cell = filename && cells.get(filename.replace(/\.slang$/, ""));
    if (!cell) return;
    const current = revision;
    const isCurrent = () => current === revision;
    const symbol = target.dataset.cellSymbol;
    for (let parent = cell.element.parentElement; parent; parent = parent.parentElement) {
      if (parent.matches("details")) parent.open = true;
    }
    cell.element.scrollIntoView({ block: "center" });
    panel = cell.element.querySelector(".snippet-navigation");
    const activePanel = panel;
    const status = panel.querySelector(".snippet-navigation-status");
    const choices = panel.querySelector(".snippet-navigation-choices");
    choices.replaceChildren();
    panel.hidden = !symbol;
    status.textContent = symbol ? `Finding ${symbol}…` : "";
    try {
      await revealTarget(cell, filename, symbol, activePanel, status, choices, isCurrent);
    } catch (error) {
      // A failed lookup would otherwise leave "Finding …" on screen indefinitely.
      // We report it in the panel and still bring the cell into view.
      console.error(`Cannot navigate to ${symbol || filename}`, error);
      if (!isCurrent()) return;
      choices.replaceChildren();
      activePanel.hidden = false;
      status.textContent = `Definition navigation is unavailable. Showing ${filename}.`;
      cell.element.scrollIntoView({ block: "center" });
    }
  }
  async function revealTarget(cell, filename, symbol, activePanel, status, choices, isCurrent) {
    const editor = await cell.getEditor();
    if (!isCurrent()) return;
    if (!symbol) {
      await editor.reveal(undefined, isCurrent);
      return;
    }
    const matches = await editor.findSymbols?.(symbol);
    if (!isCurrent()) return;
    if (matches?.length === 1) {
      await editor.reveal(matches[0].range, isCurrent);
      if (isCurrent()) activePanel.hidden = true;
      return;
    }
    await editor.reveal(undefined, isCurrent);
    if (!isCurrent()) return;
    if (!matches) {
      status.textContent = `Definition navigation is unavailable. Showing ${filename}.`;
    } else if (!matches.length) {
      status.textContent = `${symbol} is not defined in the current ${filename}.`;
    } else {
      status.textContent = `Choose a definition of ${symbol}:`;
      for (const match of matches) {
        const button = document.createElement("button");
        button.type = "button";
        button.textContent = `${match.detail || match.name} (line ${match.range.startLineNumber})`;
        button.addEventListener("click", async () => {
          if (!isCurrent()) return;
          activePanel.hidden = true;
          await editor.reveal(match.range, isCurrent);
        });
        choices.append(button);
      }
      activePanel.scrollIntoView({ block: "center" });
      choices.firstElementChild.focus({ preventScroll: true });
    }
  }
  document.addEventListener("click", event => {
    if (event.defaultPrevented || event.button !== 0 || event.ctrlKey || event.metaKey ||
        event.shiftKey || event.altKey) return;
    const link = event.target.closest("a[data-cell-link]");
    if (!link || link.hasAttribute("download") || (link.target && link.target !== "_self")) return;
    const url = new URL(link.href);
    if (url.origin !== location.origin || url.pathname !== location.pathname || url.search !== location.search) return;
    event.preventDefault();
    if (location.hash === url.hash) void navigate();
    else location.hash = url.hash;
  });
  window.addEventListener("hashchange", navigate);
  window.addEventListener("pagehide", cancel);
  void navigate();
  return { cancel };
}
