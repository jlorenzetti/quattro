# C64 — 16 KB ROM-native cartridge

**Supported target:** normal **16 KB** cartridge, CPU map **`$8000`–`$BFFF`**, **`GAME`/`EXROM` inactive** (CRT type **0**). Code and **`.rodata`** execute from cart ROM; **`.bss`**, **`.data`**, stack, and ZP stay in RAM like the **`.prg`** build. The default ship artifact remains **`make c64`** → **`build/quattro.prg`**.

## Build

| Target | Output |
|--------|--------|
| **`make c64_cart`** | **`build/quattro-cart-8000.rom`** — 16 384 bytes (padded), map **`build/quattro-cart.map`** |
| **`make c64_crt`** | **`build/quattro-cart.crt`** — packs the ROM for VICE / compatible emulators |
| **`make c64_crt_run`** | Launches **`x64sc -cartcrt`** on the **`.crt`** (requires **`x64sc`** on `PATH`) |

**Layout:** [`tools/c64/cart.ld`](../../tools/c64/cart.ld) — **`.cart_header`** (CBM80 + vectors) → **`.zp.data` LMA** → **`.cart_boot`** → **`.text`** / **`.rodata`**. **CRT:** [`tools/c64/pack_crt.py`](../../tools/c64/pack_crt.py) — **one** CHIP **`$8000`**, size **`$4000`** (type **0**); do **not** split into two 8K CHIPs (VICE may reject).

**Platform sources (cart link only):** [`cart_header.S`](../../src/platform/c64/cart_header.S), [`cart_boot.S`](../../src/platform/c64/cart_boot.S), [`cart_after_main.S`](../../src/platform/c64/cart_after_main.S) (plus usual **`src/platform/c64/*.c`**).

## Boot (durable constraints)

1. **Cold/warm vectors** → **`quattro_cart_preflight`**, not **`_start`**: KERNAL must run **`IOINIT` (`$FF84`)** before llvm-mos **`crt0`** copies **`.zp.data`** into zero page.
2. **KERNAL `start1` parity after `IOINIT`:** CBM80 jumps to cart **before** the usual **`ramtas` / `restor` / `pcint` / `cli`**. Preflight calls **`RAMTAS` (`$FF87`)**, **`RESTOR` (`$FF8A`)**, **`PCINT` (`$FF81`)**, then **`cli`**, then **`JMP _start`** — so IRQ vectors, jiffy **`$A2`**, and **`SCNKEY`/`GETIN`** match a normal cold boot.
3. **`__zp_data_load_start`:** With **lld**, **`LOADADDR(.zp.data)`** can wrongly match **`.cart_boot`**; **`cart.ld`** sets the copy source to **`LOADADDR(.cart_header) + SIZEOF(.cart_header)`** and asserts ZP LMA placement.
4. **`__after_main`:** Stock **`crt0`** does **`JMP $B205`** (BASIC). On this cart, **`$B205`** is ROM — **`cart_after_main.S`** supplies an infinite loop; **`main`** is **`noreturn`**.
5. **Omit `unmap-basic.o`:** PRG helper assumes **`$A000`–`$BFFF`** can be RAM; here that range is cartridge ROM.
6. **VIC / IRQ:** **`video_init`** reapplies text mode @ **`$0400`** and banking; **`timing_wait_frame`** uses **`cli`** before waiting on the jiffy (KERNAL may have left **`sei`** on the CBM80 path).

## ROM budget snapshot (2026-04)

Illustrative **llvm-mos** sizes from a **`.prg`** map of the same sources — cart ROM adds **9-byte header**, **`.zp.data` image**, and **`.cart_boot`** on top of **`.text`+`.rodata`**, still **under 16 KB** with **~1.2–1.3 KiB** slack before padding. Re-check **`build/quattro-cart.map`** after non-trivial code growth.

| Region | ~Size | Notes |
|--------|------:|-------|
| **`.text`** | ~14.6 KiB | Dominant |
| **`.rodata`** | ~0.5 KiB | Tables, strings |
| **`.data`** | 0 | |
| **`.bss`** | ~400 B | RAM only on cart |
| **ZP sections** | ~80 B | Init image in ROM LMA |

**Classification:** partly KERNAL-dependent ( **`SCNKEY` / `GETIN`**, CIA, VIC) — acceptable for this target; not a bare-metal cart OS.

## Out of scope here

Hardware EPROM programming, commercial packaging, bank switching, **>16 KB** ROM. Real-hardware cart smoke tests are optional follow-up.
