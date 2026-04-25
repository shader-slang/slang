(function () {
  // Measure the sticky chrome's height once after layout so any
  // sticky element below it (table <thead>, source heading, …)
  // can stack directly underneath via top: var(--chrome-h).
  function measureChrome() {
    var c = document.querySelector('.topChrome');
    if (c) {
      document.documentElement.style.setProperty(
        '--chrome-h', c.offsetHeight + 'px'
      );
    }
  }
  if (document.readyState === 'complete') measureChrome();
  else window.addEventListener('load', measureChrome);
  window.addEventListener('resize', measureChrome);
})();
(function () {
  // Per-file toggle: reveals the next sibling fileFunctions row,
  // lazy-cloning the function table from a <template> on first
  // open so the initial DOM stays small.
  function ensureFnLoaded(row) {
    if (row.dataset.loaded === '1') return;
    var tid = row.getAttribute('data-fn-tmpl');
    if (tid) {
      var tmpl = document.getElementById(tid);
      var td = row.querySelector('td');
      if (tmpl && td && td.children.length === 0) {
        td.appendChild(tmpl.content.cloneNode(true));
      }
    }
    row.dataset.loaded = '1';
  }
  function toggleFn(el) {
    var row = el.closest('tr');
    if (!row) return;
    var next = row.nextElementSibling;
    if (!next || !next.classList.contains('fileFunctions')) return;
    var hidden = next.hasAttribute('hidden');
    if (hidden) {
      ensureFnLoaded(next);
      next.removeAttribute('hidden');
      el.textContent = '\u25BC';
      el.setAttribute('aria-expanded', 'true');
    } else {
      next.setAttribute('hidden', '');
      el.textContent = '\u25B6';
      el.setAttribute('aria-expanded', 'false');
    }
  }

  // Per-directory toggle: hides every descendant row when collapsing
  // (anything whose data-dir or data-path lives under this dir's
  // path); when expanding, reveals only direct children — files
  // sitting in this dir, plus immediate sub-directory headers — all
  // in collapsed state. The user can drill further by clicking any
  // child chevron.
  function isDescendant(rowKey, parentPath) {
    if (rowKey === null) return false;
    if (parentPath === '') return rowKey !== '';
    return rowKey === parentPath || rowKey.indexOf(parentPath + '/') === 0;
  }
  function pathDepth(p) {
    return p === '' ? 0 : p.split('/').length;
  }
  function toggleDir(el) {
    var path = el.getAttribute('data-path');
    if (path === null) return;
    var collapsing = el.getAttribute('aria-expanded') !== 'false';
    var directChildDepth = pathDepth(path) + 1;

    var rows = document.querySelectorAll(
      'tr.dirHeader, tr.fileSummary, tr.fileFunctions'
    );
    rows.forEach(function (r) {
      // Skip the dir-header row that owns this toggle.
      if (r.classList.contains('dirHeader') &&
          r.getAttribute('data-path') === path) return;

      var rowKey = r.getAttribute('data-dir');
      if (rowKey === null) rowKey = r.getAttribute('data-path');
      if (!isDescendant(rowKey, path)) return;

      if (collapsing) {
        r.setAttribute('hidden', '');
        return;
      }
      // Expanding: reveal only direct children of `path`.
      if (r.classList.contains('fileSummary')) {
        if (rowKey === path) {
          r.removeAttribute('hidden');
          var t = r.querySelector('.fnToggle');
          if (t) {
            t.textContent = '\u25B6';
            t.setAttribute('aria-expanded', 'false');
          }
        }
      } else if (r.classList.contains('dirHeader')) {
        var rDepth = parseInt(r.getAttribute('data-depth') || '0', 10);
        if (rDepth === directChildDepth) {
          r.removeAttribute('hidden');
          var dt = r.querySelector('.dirToggle');
          if (dt) {
            dt.textContent = '\u25B6';
            dt.setAttribute('aria-expanded', 'false');
          }
        }
      }
      // fileFunctions rows stay hidden when expanding a directory.
    });

    el.textContent = collapsing ? '\u25B6' : '\u25BC';
    el.setAttribute('aria-expanded', collapsing ? 'false' : 'true');
  }

  document.addEventListener('click', function (e) {
    var t = e.target;
    if (!t || !t.classList) return;
    if (t.classList.contains('fnToggle'))   toggleFn(t);
    else if (t.classList.contains('dirToggle')) toggleDir(t);
  });
  document.addEventListener('keydown', function (e) {
    var t = e.target;
    if (!t || !t.classList) return;
    if (e.key !== 'Enter' && e.key !== ' ') return;
    if (t.classList.contains('fnToggle')) {
      e.preventDefault();
      toggleFn(t);
    } else if (t.classList.contains('dirToggle')) {
      e.preventDefault();
      toggleDir(t);
    }
  });
})();
