PICO_SDK_PATH ?= ../../pico-sdk
BUILD_DIR ?= build
RELEASE_DIR ?= release
VERSION ?= dev
PICOTOOL_FETCH_FROM_GIT_PATH ?=
LANGUAGE ?= english
FLASH_TARGET ?= $(if $(filter english,$(LANGUAGE)),bip39_waveshare,bip39_waveshare_$(LANGUAGE))
LANGUAGES = english czech french italian portuguese spanish

.PHONY: all configure clean flash release test audit

all: configure
	$(MAKE) -C $(BUILD_DIR)

configure:
	cmake -S . -B $(BUILD_DIR) -G "Unix Makefiles" \
		-DPICO_SDK_PATH=$(abspath $(PICO_SDK_PATH)) \
		-DPICOTOOL_FETCH_FROM_GIT_PATH=$(abspath $(PICOTOOL_FETCH_FROM_GIT_PATH))

test:
	$(CC) -std=c11 -Wall -Wextra -Werror -I../common \
		../common/bip39_lookup.c ../common/mnemonic_state.c ../common/sha256.c ../common/utf8.c \
		../tests/test_mnemonic_state.c -o /tmp/bip39_mnemonic_tests
	/tmp/bip39_mnemonic_tests
	$(CC) -std=c11 -Wall -Wextra -Werror -I../common \
		../common/mnemonic_state.c ../common/sha256.c ../common/seedqr.c \
		../common/vendor/qrcodegen/qrcodegen.c ../tests/test_seedqr.c -o /tmp/bip39_seedqr_tests
	/tmp/bip39_seedqr_tests
	$(CC) -std=c11 -Wall -Wextra -Werror -DDICEROLL_ENABLE_SEEDQR=1 \
		-I../common -Iinclude -Ivendor/waveshare/fonts \
		../tests/test_waveshare_seedqr_ui.c src/app/app.c src/ui/graphics.c src/ui/ui.c \
		../common/bip39_lookup.c ../common/mnemonic_state.c ../common/sha256.c \
		../common/seedqr.c ../common/vendor/qrcodegen/qrcodegen.c \
		../common/ui_model.c ../common/utf8.c \
		vendor/waveshare/fonts/font12.c vendor/waveshare/fonts/font16_unifont.c \
		vendor/waveshare/fonts/font16_compare.c vendor/waveshare/fonts/font20.c \
		vendor/waveshare/fonts/font24.c -o /tmp/bip39_seedqr_ui_tests
	/tmp/bip39_seedqr_ui_tests 12
	/tmp/bip39_seedqr_ui_tests 24

audit: configure
	$(MAKE) -C $(BUILD_DIR) $(addprefix bip39_waveshare_,$(filter-out english,$(LANGUAGES))) bip39_waveshare
	@set -e; \
	for language in $(LANGUAGES); do \
		target=bip39_waveshare; \
		if [ "$$language" != "english" ]; then \
			target=bip39_waveshare_$$language; \
		fi; \
		if nm -C "$(BUILD_DIR)/$$target.elf" | \
			rg -q 'flash_range_(erase|program)|flash_do_cmd'; then \
			echo "ERROR: $(BUILD_DIR)/$$target.elf contains flash-write symbols"; exit 1; \
		fi; \
		echo "Flash-write audit passed: $(BUILD_DIR)/$$target.elf"; \
	done

release: all
	@mkdir -p $(RELEASE_DIR)
	@set -e; \
	for language in $(LANGUAGES); do \
		target=bip39_waveshare; \
		if [ "$$language" != "english" ]; then \
			target=bip39_waveshare_$$language; \
		fi; \
		cp $(BUILD_DIR)/$$target.uf2 \
			$(RELEASE_DIR)/waveshare-$$language-$(VERSION).uf2; \
		echo "Release file: $(RELEASE_DIR)/waveshare-$$language-$(VERSION).uf2"; \
	done

flash: configure
	$(MAKE) -C $(BUILD_DIR) $(FLASH_TARGET)
	@set -e; \
	uf2="$(abspath $(BUILD_DIR))/$(FLASH_TARGET).uf2"; \
	echo "Waiting for RP2350 BOOTSEL volume..."; \
	seconds=0; volume=""; \
	while [ -z "$$volume" ]; do \
		if [ -d /Volumes/RP2350 ]; then volume=/Volumes/RP2350; fi; \
		if [ -d /Volumes/RPI-RP2 ]; then volume=/Volumes/RPI-RP2; fi; \
		if [ $$seconds -ge 120 ]; then \
			echo "Timed out waiting for BOOTSEL mode"; exit 1; \
		fi; \
		if [ -z "$$volume" ]; then sleep 1; seconds=$$((seconds + 1)); fi; \
	done; \
	echo "Copying $$uf2 to $$volume"; \
	cp "$$uf2" "$$volume/"; \
	sync; \
	echo "Flash complete; the board should now be rebooting."

clean:
	$(MAKE) -C $(BUILD_DIR) clean
