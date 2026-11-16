// Arithmatex marks equations during Markdown conversion. Render only those nodes,
// so dollar signs and backslashes in code examples and editable cells stay untouched.
(() => {
  // A display equation may carry one marker line, which KaTeX would ignore as a comment:
  // `% label: Smooth union` numbers the equation and names it in a caption below it;
  // `% number` numbers it without a caption, placing the number after the equation.
  const markerPattern = /^[ \t]*%[ \t]*(?:label:[ \t]*(.+?)|number)[ \t]*$/m;
  const sectionPattern = /^\s*(\d+(?:\.\d+)*)\s*:/;

  // A marked display equation is numbered after the nearest numbered section heading
  // above it, with a letter for its position in that section: the second marked
  // equation under "1.4: ..." is "Eq 1.4(b)". Sections without a number get no number.
  function equationNumbers() {
    const numbers = new Map();
    let section = null;
    let count = 0;
    const nodes = document.querySelectorAll(
      ".md-content h2, .md-content h3, div.arithmatex"
    );
    for (const node of nodes) {
      if (node.tagName !== "DIV") {
        section = node.textContent.match(sectionPattern)?.[1] ?? null;
        count = 0;
        continue;
      }
      const source = node.dataset.mathSource ?? node.textContent;
      if (!section || !markerPattern.test(source)) continue;
      numbers.set(node, `${section}(${String.fromCharCode(97 + count++)})`);
    }
    return numbers;
  }

  function createNumber(number, suffix) {
    const span = document.createElement("span");
    span.className = "math-label-number";
    span.textContent = `Eq ${number}${suffix}`;
    return span;
  }

  // Show the marker in the style of a paper: "Eq 1.4(a): Translated field" as a
  // caption below a labelled equation, or "Eq 1.4(a)" beside a numbered one.
  function addMarker(element, marker, number) {
    const name = marker[1];
    if (name) {
      const label = document.createElement("div");
      label.className = "math-label";
      if (number) label.append(createNumber(number, ":"), " ");
      label.append(name);
      element.append(label);
    } else if (number) {
      // The number hangs off the equation's right edge, so the equation stays centred.
      const equation = element.querySelector(".katex-display > .katex");
      equation.classList.add("math-numbered");
      const tag = createNumber(number, "");
      tag.classList.add("math-tag");
      equation.append(tag);
    }
  }

  function renderMath() {
    const numbers = equationNumbers();
    for (const element of document.querySelectorAll(".arithmatex")) {
      if (element.dataset.mathRendered) continue;
      // Generic mode wraps inline math in \(...\) and display math in \[...\].
      let source = element.textContent.slice(2, -2);
      element.dataset.mathSource = source;
      const marker = source.match(markerPattern);
      if (marker) source = source.replace(marker[0], "");
      katex.render(source, element, {
        displayMode: element.tagName === "DIV",
        output: "htmlAndMathml",
        throwOnError: false,
      });
      if (marker && element.tagName === "DIV") {
        addMarker(element, marker, numbers.get(element));
      }
      element.dataset.mathRendered = "true";
    }
  }

  // Material emits on initial load and after navigation if instant loading is enabled.
  if (typeof document$ !== "undefined") document$.subscribe(renderMath);
  else renderMath();
})();
