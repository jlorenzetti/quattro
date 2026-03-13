# Quattro — GitHub Pages demo

Minimal browser demo for Quattro (this repository) using [VICE.js](https://github.com/rjanicek/vice.js). Experimental; not the primary way to play.

## Static templates vs generated artifacts

**Templates (committed):** `index.html`, `NOTICE`, `README.md` — source of truth for the demo page and licensing.

**Generated (not committed when using Option B):** `quattro.prg` (from `make c64`), `vendor/x64.js` (from VICE.js). Option B produces these in CI and deploys them; Option A copies them into `web/` via `make web-demo` and commits.

## Publication paths

- **Option A (fallback):** Run `make web-demo`, commit `web/` (including `quattro.prg` and `vendor/x64.js`). Set GitHub Pages source to **Deploy from a branch** → folder **/web**.
- **Option B (CI):** Workflow `.github/workflows/pages.yml` builds C64, assembles the site, and deploys. Set GitHub Pages source to **GitHub Actions**. No need to commit PRG or vendor in `main`.

**Licensing:** Quattro is MIT. VICE.js is GPLv2 — see [NOTICE](NOTICE) in this folder.
