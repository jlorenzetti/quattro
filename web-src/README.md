# Quattro — browser demo (source)

Source of the GitHub Pages browser demo. Uses [VICE.js](https://github.com/rjanicek/vice.js). Not the primary way to play.

**Templates (committed):** `index.html`, `NOTICE`, `README.md`.

**Generated in CI:** `quattro.prg` (from `make c64`), `vendor/x64.js` (from VICE.js). The workflow assembles these into a `site/` artifact and deploys to GitHub Pages. They are not committed.

**Publication:** `.github/workflows/pages.yml` builds the C64 artifact, assembles the site from `web-src/` + generated PRG + downloaded VICE.js, and deploys. Set GitHub Pages source to **GitHub Actions**.

**Licensing:** Quattro is MIT. VICE.js is GPLv2 — see [NOTICE](NOTICE).
