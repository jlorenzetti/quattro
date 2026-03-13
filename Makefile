.PHONY: help host_debug test c64 c64_run compdb compdb-host compdb-c64 compdb-all clean

CORE_SRC := src/core/board.c src/core/game_state.c src/core/piece.c \
	src/core/rng.c src/core/rules.c src/core/scoring.c
CORE_INC := -Isrc/core

C64_PLATFORM_SRC := src/platform/c64/main.c src/platform/c64/video.c \
	src/platform/c64/input.c src/platform/c64/timing.c
C64_INC := $(CORE_INC) -Isrc/platform/c64
MOS_CC ?= mos-c64-clang
BUILD_DIR := build
C64_PRG := $(BUILD_DIR)/quattro.prg

help:
	@echo "Quattro — host-side core + C64"
	@echo ""
	@echo "  make host_debug   Build and run host debug harness (optional: SEED=42)"
	@echo "  make test        Build and run core tests"
	@echo "  make c64         Build C64 PRG (requires llvm-mos: mos-c64-clang)"
	@echo "  make c64_run     Build and run in emulator (requires VICE x64sc)"
	@echo "  make compdb       Generate compile_commands.json for clangd (host; requires Bear)"
	@echo "  make compdb-host  Generate compile_commands.host.json only"
	@echo "  make compdb-c64   Generate compile_commands.c64.json only (requires Bear + llvm-mos)"
	@echo "  make compdb-all   Generate merged compile_commands.json (host + C64)"
	@echo "  make clean        Remove build artifacts"

host_debug: tools/host_debug/host_debug
	$(if $(SEED),./tools/host_debug/host_debug $(SEED),./tools/host_debug/host_debug)

tools/host_debug/host_debug: tools/host_debug/main.c tools/host_debug/ascii_renderer.c $(CORE_SRC)
	$(CC) -o $@ $(CORE_INC) -Itools/host_debug $(CORE_SRC) tools/host_debug/ascii_renderer.c tools/host_debug/main.c

test: tests/test_runner
	./tests/test_runner

tests/test_runner: tests/test_runner.c tests/core/test_board.c tests/core/test_piece.c tests/core/test_rules.c tests/core/test_game_state.c $(CORE_SRC)
	$(CC) -o $@ $(CORE_INC) -Itests -Itests/core tests/test_runner.c tests/core/test_board.c tests/core/test_piece.c tests/core/test_rules.c tests/core/test_game_state.c $(CORE_SRC)

c64: $(C64_PRG)

$(C64_PRG): $(CORE_SRC) $(C64_PLATFORM_SRC)
	@mkdir -p $(BUILD_DIR)
	$(MOS_CC) -Os $(C64_INC) -o $@ $(CORE_SRC) $(C64_PLATFORM_SRC)

c64_run: $(C64_PRG)
	@command -v x64sc >/dev/null 2>&1 || { echo "VICE x64sc not found; install VICE or run the PRG in your C64 emulator."; exit 1; }
	x64sc $(abspath $(C64_PRG))

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
	rm -f $(C64_PRG)
	rm -f compile_commands.json compile_commands.host.json compile_commands.c64.json
