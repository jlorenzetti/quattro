.PHONY: help host_debug test c64 c64_cart c64_crt c64_crt_run c64_perf c64_fixed_seed c64_run c64_vic_bank2 c64_vic_bank2_run c64_charset_probe c64_charset_probe_run c64_charset_probe_b21 c64_charset_probe_b21_run compdb compdb-host compdb-c64 compdb-all clean release_artifacts demo_gif

CORE_SRC := src/core/board.c src/core/game_state.c src/core/piece.c \
	src/core/rng.c src/core/rules.c src/core/scoring.c \
	src/core/input_model.c
CORE_INC := -Isrc/core

C64_PLATFORM_SRC := src/platform/c64/main.c src/platform/c64/video.c \
	src/platform/c64/input.c src/platform/c64/seed.c src/platform/c64/timing.c src/platform/c64/gravity.c \
	src/platform/c64/audio.c
C64_INC := $(CORE_INC) -Isrc/platform/c64
C64_DEFS ?=
C64_PERF_SRC ?=
MOS_CC ?= mos-c64-clang
BUILD_DIR := build
C64_PRG := $(BUILD_DIR)/quattro.prg
# Optional custom linker script for C64 experiments (e.g. charset memory layout). Example:
#   make c64 C64_LD=tools/c64/your_experiment.ld
C64_LD ?=
C64_CART_LD := tools/c64/cart.ld
C64_CART_HEADER_S := src/platform/c64/cart_header.S
C64_CART_HEADER_OBJ := $(BUILD_DIR)/cart_header.o
C64_CART_BOOT_S := src/platform/c64/cart_boot.S
C64_CART_BOOT_OBJ := $(BUILD_DIR)/cart_boot.o
C64_CART_AFTER_MAIN_S := src/platform/c64/cart_after_main.S
C64_CART_AFTER_MAIN_OBJ := $(BUILD_DIR)/cart_after_main.o
C64_CART_ROM := $(BUILD_DIR)/quattro-cart-8000.rom
C64_CART_MAP := $(BUILD_DIR)/quattro-cart.map
C64_CRT := $(BUILD_DIR)/quattro-cart.crt
C64_PACK_CRT := tools/c64/pack_crt.py

C64_CHARSET_PROBE_LD := tools/c64/charset_probe/charset_probe.ld
C64_CHARSET_PROBE_B21_LD := tools/c64/charset_probe/charset_probe_b21.ld
C64_CHARSET_PROBE_SRC := tools/c64/charset_probe/main.c
C64_CHARSET_PROBE_PRG := $(BUILD_DIR)/charset_probe.prg
C64_CHARSET_PROBE_B21_PRG := $(BUILD_DIR)/charset_probe_b21.prg

DIST_DIR := dist
VERSION ?=
# Default release uses the standard C64 PRG. A future release_artifacts_silent target
# can set RELEASE_SRC_PRG to a separately built silent PRG without changing this recipe.
RELEASE_SRC_PRG := $(C64_PRG)
ifeq ($(strip $(VERSION)),)
RELEASE_PRG := $(DIST_DIR)/quattro-local.prg
else
RELEASE_PRG := $(DIST_DIR)/quattro-$(VERSION).prg
endif

help:
	@echo "Quattro — host-side core + C64"
	@echo ""
	@echo "  make host_debug   Build and run host debug harness (optional: SEED=42)"
	@echo "  make test        Build and run core tests"
	@echo "  make c64         Build C64 PRG (requires llvm-mos: mos-c64-clang; optional: C64_LD=path.ld)"
	@echo "  make c64_cart      16 KB raw ROM at 0x8000 (CBM80 + llvm-mos crt0); map: build/quattro-cart.map"
	@echo "  make c64_crt       Pack ROM into VICE/CCS64 .crt (normal 16K type 0, EXROM=GAME=0)"
	@echo "  make c64_crt_run   Attach .crt in x64sc (same as: x64sc -cartcrt build/quattro-cart.crt)"
	@echo "  make release_artifacts  Stage default C64 PRG into dist/ for release (optional: VERSION=v0.1.0)"
	@echo "  make c64_perf    Build C64 PRG with QUATTRO_PERF (CIA2 timers; halts after N frames)"
	@echo "  make c64_fixed_seed  Build C64 PRG with fixed seed 12345 (reproducible debug)"
	@echo "  make c64_run     Build and run in emulator (requires VICE x64sc)"
	@echo "  make c64_vic_bank2  C64 PRG: VIC bank 2 + screen @ 0x8000 + charset @ 0x8800 (experiment; see custom-charset-investigation.md)"
	@echo "  make c64_vic_bank2_run  Build and run bank-2 experiment in x64sc"
	@echo "  make c64_charset_probe   Charset probe PRG (B1 linker; ram capped before 0x3000)"
	@echo "  make c64_charset_probe_run  Build and run B1 probe in x64sc"
	@echo "  make c64_charset_probe_b21  Charset probe — B2.1 linker (default C64 ram + assert; see custom-charset-investigation.md)"
	@echo "  make c64_charset_probe_b21_run  Build and run B2.1 probe in x64sc"
	@echo "  make compdb       Generate compile_commands.json for clangd (host; requires Bear)"
	@echo "  make compdb-host  Generate compile_commands.host.json only"
	@echo "  make compdb-c64   Generate compile_commands.c64.json only (requires Bear + llvm-mos)"
	@echo "  make compdb-all   Generate merged compile_commands.json (host + C64)"
	@echo "  make clean        Remove build artifacts"
	@echo "  make demo_gif     Build README GIF from VICE capture (requires ffmpeg; INPUT=path/to/video.avi [OUTPUT=docs/quattro-demo.gif])"

host_debug: tools/host_debug/host_debug
	$(if $(SEED),./tools/host_debug/host_debug $(SEED),./tools/host_debug/host_debug)

tools/host_debug/host_debug: tools/host_debug/main.c tools/host_debug/ascii_renderer.c $(CORE_SRC)
	$(CC) -o $@ $(CORE_INC) -Itools/host_debug $(CORE_SRC) tools/host_debug/ascii_renderer.c tools/host_debug/main.c

test: tests/test_runner
	./tests/test_runner

tests/test_runner: tests/test_runner.c tests/core/test_board.c tests/core/test_piece.c tests/core/test_rules.c tests/core/test_game_state.c tests/core/test_input_model.c tests/test_gravity.c src/platform/c64/gravity.c $(CORE_SRC)
	$(CC) -o $@ $(CORE_INC) -Itests -Itests/core -Isrc/platform/c64 tests/test_runner.c tests/core/test_board.c tests/core/test_piece.c tests/core/test_rules.c tests/core/test_game_state.c tests/core/test_input_model.c tests/test_gravity.c src/platform/c64/gravity.c $(CORE_SRC)

c64: $(C64_PRG)

$(C64_PRG): $(CORE_SRC) $(C64_PLATFORM_SRC) $(C64_PERF_SRC)
	@mkdir -p $(BUILD_DIR)
	$(MOS_CC) -Os $(C64_DEFS) $(C64_INC) -Wl,-Map=$(BUILD_DIR)/quattro.map $(if $(strip $(C64_LD)),-T $(C64_LD),) -o $@ $(CORE_SRC) $(C64_PLATFORM_SRC) $(C64_PERF_SRC)

c64_cart: $(C64_CART_ROM)

$(C64_CART_HEADER_OBJ): $(C64_CART_HEADER_S)
	@mkdir -p $(BUILD_DIR)
	$(MOS_CC) -c -o $@ $(C64_CART_HEADER_S)

$(C64_CART_BOOT_OBJ): $(C64_CART_BOOT_S)
	@mkdir -p $(BUILD_DIR)
	$(MOS_CC) -c -o $@ $(C64_CART_BOOT_S)

$(C64_CART_AFTER_MAIN_OBJ): $(C64_CART_AFTER_MAIN_S)
	@mkdir -p $(BUILD_DIR)
	$(MOS_CC) -c -o $@ $(C64_CART_AFTER_MAIN_S)

$(C64_CART_ROM): $(CORE_SRC) $(C64_PLATFORM_SRC) $(C64_PERF_SRC) $(C64_CART_LD) $(C64_CART_HEADER_OBJ) $(C64_CART_BOOT_OBJ) $(C64_CART_AFTER_MAIN_OBJ)
	@mkdir -p $(BUILD_DIR)
	$(MOS_CC) -Os $(C64_DEFS) $(C64_INC) -Wl,-Map=$(C64_CART_MAP) -T $(C64_CART_LD) -o $@ $(C64_CART_HEADER_OBJ) $(C64_CART_BOOT_OBJ) $(CORE_SRC) $(C64_PLATFORM_SRC) $(C64_PERF_SRC) $(C64_CART_AFTER_MAIN_OBJ)

c64_crt: $(C64_CRT)

$(C64_CRT): $(C64_CART_ROM) $(C64_PACK_CRT)
	python3 $(C64_PACK_CRT) $(C64_CART_ROM) $@ "QUATTRO 16K"

c64_crt_run: $(C64_CRT)
	@command -v x64sc >/dev/null 2>&1 || { echo "VICE x64sc not found; install VICE or attach $(C64_CRT) manually."; exit 1; }
	x64sc -cartcrt $(abspath $(C64_CRT))

c64_perf:
	rm -f $(C64_PRG)
	$(MAKE) $(C64_PRG) C64_DEFS=-DQUATTRO_PERF=1 C64_PERF_SRC=src/platform/c64/perf.c

c64_fixed_seed: C64_DEFS := -DQUATTRO_FIXED_SEED=12345
c64_fixed_seed: $(C64_PRG)

c64_run: $(C64_PRG)
	@command -v x64sc >/dev/null 2>&1 || { echo "VICE x64sc not found; install VICE or run the PRG in your C64 emulator."; exit 1; }
	x64sc $(abspath $(C64_PRG))

c64_vic_bank2:
	rm -f $(C64_PRG)
	$(MAKE) $(C64_PRG) C64_DEFS=-DQUATTRO_VIC_BANK2_CHARSET=1

c64_vic_bank2_run: c64_vic_bank2
	@command -v x64sc >/dev/null 2>&1 || { echo "VICE x64sc not found; run $(C64_PRG) in your C64 emulator."; exit 1; }
	x64sc $(abspath $(C64_PRG))

c64_charset_probe: $(C64_CHARSET_PROBE_PRG)

$(C64_CHARSET_PROBE_PRG): $(C64_CHARSET_PROBE_SRC) $(C64_CHARSET_PROBE_LD)
	@mkdir -p $(BUILD_DIR)
	$(MOS_CC) -Os -Wall -Wextra -Wl,-Map=$(BUILD_DIR)/charset_probe.map -T $(C64_CHARSET_PROBE_LD) -o $@ $(C64_CHARSET_PROBE_SRC)

c64_charset_probe_run: $(C64_CHARSET_PROBE_PRG)
	@command -v x64sc >/dev/null 2>&1 || { echo "VICE x64sc not found; run $(C64_CHARSET_PROBE_PRG) in your C64 emulator."; exit 1; }
	x64sc $(abspath $(C64_CHARSET_PROBE_PRG))

c64_charset_probe_b21: $(C64_CHARSET_PROBE_B21_PRG)

$(C64_CHARSET_PROBE_B21_PRG): $(C64_CHARSET_PROBE_SRC) $(C64_CHARSET_PROBE_B21_LD)
	@mkdir -p $(BUILD_DIR)
	$(MOS_CC) -Os -Wall -Wextra -Wl,-Map=$(BUILD_DIR)/charset_probe_b21.map -T $(C64_CHARSET_PROBE_B21_LD) -o $@ $(C64_CHARSET_PROBE_SRC)

c64_charset_probe_b21_run: $(C64_CHARSET_PROBE_B21_PRG)
	@command -v x64sc >/dev/null 2>&1 || { echo "VICE x64sc not found; run $(C64_CHARSET_PROBE_B21_PRG) in your C64 emulator."; exit 1; }
	x64sc $(abspath $(C64_CHARSET_PROBE_B21_PRG))

release_artifacts: c64
	@mkdir -p $(DIST_DIR)
	cp $(RELEASE_SRC_PRG) $(RELEASE_PRG)
	@echo "Staged: $(abspath $(RELEASE_PRG))"

compdb: compdb-host
	@cp compile_commands.host.json compile_commands.json
	@echo "compile_commands.json updated (host). Use compdb-c64 for C64; compdb-all for both."

compdb-host:
	@command -v bear >/dev/null 2>&1 || { echo "compdb requires Bear. Install: https://github.com/rizsotto/Bear — e.g. brew install bear (macOS), or your distro package."; exit 1; }
	bear -o compile_commands.host.json -- make tools/host_debug/host_debug tests/test_runner

compdb-c64:
	@command -v bear >/dev/null 2>&1 || { echo "compdb-c64 requires Bear. Install: https://github.com/rizsotto/Bear"; exit 1; }
	bear -o compile_commands.c64.json -- make $(C64_PRG)

compdb-all: compdb-host compdb-c64
	@python3 -c "import json; a=json.load(open('compile_commands.host.json')); b=json.load(open('compile_commands.c64.json')); json.dump(a+b, open('compile_commands.json','w'), indent=2)"
	@echo "compile_commands.json updated (host + C64 merged)."

clean:
	rm -f tools/host_debug/host_debug tests/test_runner
	rm -f $(C64_PRG) $(BUILD_DIR)/quattro.map $(C64_CART_ROM) $(C64_CART_MAP) $(C64_CART_HEADER_OBJ) $(C64_CART_BOOT_OBJ) $(C64_CART_AFTER_MAIN_OBJ) $(C64_CRT)
	rm -f $(C64_CHARSET_PROBE_PRG) $(BUILD_DIR)/charset_probe.map
	rm -f $(C64_CHARSET_PROBE_B21_PRG) $(BUILD_DIR)/charset_probe_b21.map
	rm -f compile_commands.json compile_commands.host.json compile_commands.c64.json

demo_gif:
	@test -n "$(INPUT)" || (echo 'Usage: make demo_gif INPUT=path/to/video.avi [OUTPUT=docs/quattro-demo.gif]' && exit 1)
	@command -v ffmpeg >/dev/null 2>&1 || { echo 'demo_gif requires ffmpeg (e.g. brew install ffmpeg).'; exit 1; }
	@tools/make_demo_gif.sh "$(INPUT)" "$(or $(OUTPUT),docs/quattro-demo.gif)"
