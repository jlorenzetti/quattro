# Quattro — browser demo (source)

Source of the GitHub Pages browser demo. Uses [VICE.js](https://github.com/rjanicek/vice.js). Not the primary way to play.

**Touch controls:** A touch accessibility layer is shown below the emulator on touch-capable or narrow viewports. It emits the same canonical inputs as the keyboard (RET, LEFT, RIGHT, CCW, CW, DROP). On the start/help screen, touch LEFT/RIGHT change start level (keyboard 0–9 still work); RET continues/starts/replays. Keyboard remains canonical; the layer is browser-demo-only.

**Templates (committed):** `index.html`, `NOTICE`, `README.md`.

**Generated in CI:** `quattro.prg` (from `make c64`), `vendor/x64.js` (from VICE.js). The workflow assembles these into a `site/` artifact and deploys to GitHub Pages. They are not committed.

**Publication:** `.github/workflows/pages.yml` builds the C64 artifact, assembles the site from `web-src/` + generated PRG + downloaded VICE.js, and deploys. Set GitHub Pages source to **GitHub Actions**.

**Licensing:** Quattro is MIT. VICE.js is GPLv2 — see [NOTICE](NOTICE).
