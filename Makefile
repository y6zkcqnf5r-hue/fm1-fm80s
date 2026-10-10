# SPDX-License-Identifier: GPL-3.0-only
# FM80S build. Works with GNU Make 3.81 (the macOS one).
#
#   make host    native simulator: build/host/fm80s SCRIPT.trs [OUTDIR] -> WAV + PNG
#   make web     wasm build: build/emu/{index.html, worklet.js, keymap.js, panel.js, fm80s.wasm}
#                (emcc from the local emsdk at EMSDK_VERSION; DOCKER=1 uses the pinned image)
#   make serve   make web, then serve build/emu on http://localhost:8080
#   make test    lint + lint self-test + generated tables + unit tests + size report + scenarios +
#                wasm/native parity
#   make size    tools/size_report.py: RAM, pool and code against the FM-1 budgets
#                (firmware/src compiled as 32-bit ELF objects by clang, ADR-020)
#   make parity  tests/host/emu_test.mjs: the wasm and the native build give the same bytes
#                (PARITY=0 skips it, loudly: CLAUDE.md rule 7 is then not checked)
#   make scenarios  every tests/scenarios/*.trs into build/scenarios/<name>/, outputs checked
#                against tests/golden/<name>.sha256
#   make golden  play the scenarios and the unit tests, accept their WAV and PNG as the new golden
#                hashes (tests/golden/, and tests/golden/renders/ for the unit tests' reference renders)
#   make gen     build/gen/tables.h (tools/gen_tables.py), build/gen/font.h (tools/gen_font.py)
#   make lint    chip rules on firmware/src, SPDX headers everywhere
#   make clean

CC ?= cc
CFLAGS_COMMON = -std=gnu99 -O2 -Wall -Wextra -Werror=double-promotion -ffp-contract=off -fwrapv \
                -Ifirmware/src -Ibuild/gen

# Emscripten, pinned (ADR-011): local emsdk at the same version, or the Docker image
EMSDK_VERSION := 6.0.11
EMSDK_DIR ?= $(HOME)/emsdk
EMCC ?= $(shell command -v emcc 2>/dev/null || echo $(EMSDK_DIR)/upstream/emscripten/emcc)
EMSDK_PYTHON ?= $(firstword $(wildcard /opt/homebrew/bin/python3.12 /usr/local/bin/python3.12))
ifneq ($(EMSDK_PYTHON),)
export EMSDK_PYTHON
endif
# as the user, not root (the files in build/ stay theirs); Emscripten's cache then goes in build/,
# the image's own is not writable (after Hortator's tools/build_sim.sh)
ifeq ($(DOCKER),1)
EMCC = docker run --rm -u $(shell id -u):$(shell id -g) -e HOME=/tmp -e EM_CACHE=/src/build/emcache \
       -v "$(CURDIR)":/src -w /src emscripten/emsdk:$(EMSDK_VERSION) emcc
endif

PYTHON ?= python3
GEN = build/gen/tables.h build/gen/font.h
FONT_BDF = assets/fonts/ter-u12n.bdf assets/fonts/ter-u16n.bdf
# the faces (ADR-027): Terminus 12 as is, cut to 6x10, and Terminus 16 cut to 8x12
FONTS = FONT_6X12=assets/fonts/ter-u12n.bdf FONT_6X10=assets/fonts/ter-u12n.bdf:1:10 \
        FONT_8X12=assets/fonts/ter-u16n.bdf:2:12
UNIT_SRC = $(wildcard tests/unit/test_*.c)
UNIT_BIN = $(patsubst tests/unit/%.c,build/unit/%,$(UNIT_SRC))
UNIT_LIB = $(wildcard firmware/src/dsp/*.c firmware/src/engine/*.c)   # pure code: no plat.h calls
FIRMWARE_H = $(wildcard firmware/src/*/*.h)
APP_SRC = $(filter-out %/plat_fm1.c,$(wildcard firmware/src/*/*.c))
HOST_SRC = host/fm80s_host.c host/scenario.c host/wav.c host/png.c
HOST_BIN = build/host/fm80s
SCENARIOS = $(wildcard tests/scenarios/*.trs)
WEB_SRC = web/emu/fm80s_web.c host/fm80s_host.c
WEB_PAGE = build/emu/index.html build/emu/worklet.js build/emu/keymap.js build/emu/panel.js
WASM = build/emu/fm80s.wasm
NODE ?= node

# size report (ADR-020): the shared sources as the device sees them, APP_POOL in .pool, no libc headers
SIZE_CC ?= clang
SIZE_CFLAGS = --target=thumbv7m-none-eabi -ffreestanding -nostdlibinc -fno-common -DAPP_DEVICE \
              $(CFLAGS_COMMON)
SIZE_OBJ = $(patsubst firmware/src/%.c,build/size/%.o,$(APP_SRC))

.PHONY: all host web serve gen test unit run-unit size scenarios run-scenarios golden parity lint lint-selftest clean

all: test

host: $(HOST_BIN)

$(HOST_BIN): $(HOST_SRC) $(APP_SRC) $(wildcard host/*.h) $(FIRMWARE_H) $(GEN)
	@mkdir -p build/host
	$(CC) $(CFLAGS_COMMON) -DAPP_HOST -Ihost -o $@ $(HOST_SRC) $(APP_SRC)

run-scenarios: $(HOST_BIN)
	@rm -rf build/scenarios
	@for s in $(SCENARIOS); do \
	    n=$$(basename $$s .trs); mkdir -p build/scenarios/$$n; \
	    echo "scenario $$n"; $(HOST_BIN) $$s build/scenarios/$$n || exit 1; \
	done
	@$(PYTHON) tests/host/check_outputs.py build/scenarios

scenarios: run-scenarios
	@$(PYTHON) tests/host/golden.py check build/scenarios tests/golden

golden: run-scenarios run-unit
	@$(PYTHON) tests/host/golden.py accept build/scenarios tests/golden
	@$(PYTHON) tests/host/golden.py accept build/renders tests/golden/renders

web: $(WASM) $(WEB_PAGE)

# same flags as the native build (rule 7), no Emscripten glue: the worklet brings a stub WASI
$(WASM): $(WEB_SRC) $(APP_SRC) $(wildcard host/*.h) $(FIRMWARE_H) $(GEN)
	@mkdir -p build/emu
	@if [ "$(DOCKER)" != 1 ] && ! command -v $(firstword $(EMCC)) >/dev/null 2>&1; then \
	    echo "make web: emcc introuvable ($(EMCC))."; \
	    echo "  Installer emsdk $(EMSDK_VERSION) dans $(EMSDK_DIR), ou lancer avec DOCKER=1 (image emscripten/emsdk:$(EMSDK_VERSION))."; \
	    echo "  Pour sauter la parité dans make test : make test PARITY=0"; exit 1; fi
	$(EMCC) $(CFLAGS_COMMON) -DAPP_HOST -DAPP_WEB -Ihost --no-entry -sSTANDALONE_WASM -sFILESYSTEM=0 \
	    -sSTACK_SIZE=1048576 -sINITIAL_MEMORY=33554432 -o $@ $(WEB_SRC) $(APP_SRC)
	@echo "web: $@ ($$(wc -c < $@ | tr -d ' ') octets)"

build/emu/%: web/emu/%
	@mkdir -p build/emu
	cp $< $@

serve: web
	@echo "FM80S : http://localhost:8080 (Ctrl+C pour arrêter)"
	@$(PYTHON) -m http.server 8080 --bind 127.0.0.1 --directory build/emu

ifeq ($(PARITY),0)
parity:
	@echo "SKIP parité (PARITY=0) : règle 7 NON vérifiée, le wasm et le natif ne sont pas comparés"
else
parity: $(HOST_BIN) $(WASM) web/emu/worklet.js web/emu/keymap.js web/emu/panel.js
	@$(NODE) tests/host/emu_test.mjs
endif

lint:
	@sh tools/lint.sh

lint-selftest:
	@sh tests/lint/selftest.sh

gen: $(GEN)

build/gen/tables.h: tools/gen_tables.py
	@$(PYTHON) tools/gen_tables.py $@

build/gen/font.h: tools/gen_font.py $(FONT_BDF)
	@$(PYTHON) tools/gen_font.py $@ $(FONTS)

build/unit/%: tests/unit/%.c tests/unit/test.h $(UNIT_LIB) $(FIRMWARE_H) $(GEN)
	@mkdir -p build/unit
	$(CC) $(CFLAGS_COMMON) -Itests/unit -o $@ $< $(UNIT_LIB) -lm

# the tests of ui/ bring their own lcd_fill / lcd_blit (a fake screen)
build/unit/test_gfx: firmware/src/ui/gfx.c
build/unit/test_gfx: UNIT_LIB += firmware/src/ui/gfx.c

# the unit tests also write reference renders in build/renders/<suite>/ (tests/unit/render.h)
run-unit: $(UNIT_BIN)
	@rm -rf build/renders
	@for t in $(UNIT_BIN); do $$t || exit 1; done

unit: run-unit
	@$(PYTHON) tests/host/check_outputs.py build/renders
	@$(PYTHON) tests/host/golden.py check build/renders tests/golden/renders

build/size/%.o: firmware/src/%.c $(FIRMWARE_H) $(GEN)
	@mkdir -p $(dir $@)
	@command -v $(SIZE_CC) >/dev/null 2>&1 || { echo "make size: $(SIZE_CC) introuvable (il faut clang)"; exit 1; }
	@$(SIZE_CC) $(SIZE_CFLAGS) -c $< -o $@

size: $(SIZE_OBJ) tools/size_report.py firmware/src/app/config.h
	@$(PYTHON) tools/size_report.py --config firmware/src/app/config.h $(SIZE_OBJ)

test: lint lint-selftest unit size scenarios parity

clean:
	rm -rf build
