/*
 * ps1-ram-tester - (C) 2026 spicyjpeg
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include "common/gpu.h"
#include "common/sio0.h"
#include "common/spu.h"
#include "main/defs.h"
#include "main/mainmenu.h"
#include "main/ramconfig.h"
#include "main/renderer.h"
#include "main/sound.h"
#include "main/ui.h"
#include "ps1/gpucmd.h"
#include "ps1/registers.h"

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

int main(int argc, const char **argv) {
	(void) argc;
	(void) argv;

#ifdef ENABLE_LOGGING
	initSerialIO(115200);
#endif
	LOG("ps1-ram-tester " VERSION_STRING " (" __DATE__ " " __TIME__ ")");
	LOG("(C) 2026 spicyjpeg");

	if ((GPU_GP1 & GP1_STAT_FB_MODE_BITMASK) == GP1_STAT_FB_MODE_PAL) {
		LOG("using PAL mode");
		setupGPU(GP1_MODE_PAL, SCREEN_WIDTH, SCREEN_HEIGHT);
	} else {
		LOG("using NTSC mode");
		setupGPU(GP1_MODE_NTSC, SCREEN_WIDTH, SCREEN_HEIGHT);
	}

	initSPU();
	initSound();
	playBGM();
	initControllerBus();
	fixRetailRAMConfig();

	static RenderContext ctx;
	static UIState       state;

	setupRenderer(&ctx, SCREEN_WIDTH, SCREEN_HEIGHT);
	setupUIState(&state);
	enterMainMenu(&ctx, &state, 0);

	for (;;) {
		beginFrame(&ctx);
		renderMenu(&ctx, &state);
		updateMenu(&ctx, &state, pollController(0) | pollController(1));
		endFrame(&ctx);
	}

	return 0;
}
