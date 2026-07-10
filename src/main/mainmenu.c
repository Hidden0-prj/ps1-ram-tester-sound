/*
 * PSX-iTests - (C) 2026
 *
 * Built on ps1-bare-metal and ps1-ram-tester's UI framework by spicyjpeg,
 * licensed under the MIT license.
 */

#include <stdint.h>
#include <stdio.h>
#include "common/reboot.h"
#include "main/cpu_bench.h"
#include "main/gpu_colorbars.h"
#include "main/gpu_cube.h"
#include "main/mainmenu.h"
#include "main/modals.h"
#include "main/ramtester.h"
#include "main/renderer.h"
#include "main/spu_channel_test.h"
#include "main/ui.h"

/* Main menu */

static const MenuItem mainMenu[] = {
	{
		.name = "- PSX-iTests -",
		.type = ITEM_TITLE
	}, {
		.type = ITEM_SEPARATOR
	}, {
		.name   = "GPU: Color bar test pattern",
		.type   = ITEM_ACTION,
		.action = { .callback = runColorBarTest }
	}, {
		.name   = "GPU: Spinning cube (3D/GTE)",
		.type   = ITEM_ACTION,
		.action = { .callback = runGPUCubeTest }
	}, {
		.name     = "CPU: Benchmark",
		.type     = ITEM_ACTION,
		.action   = {
			.tag      = cpuScoreResult,
			.callback = runCPUBenchmark
		}
	}, {
		.name   = "SPU: Channel test",
		.type   = ITEM_ACTION,
		.action = { .callback = runSPUChannelTest }
	}, {
		.type = ITEM_SEPARATOR
	}, {
		.name   = "RAM/VRAM/SPU RAM Tester...",
		.type   = ITEM_ACTION,
		.action = { .callback = enterRAMTesterMenu }
	}, {
		.type = ITEM_SEPARATOR
	}, {
		.name   = "Boot CD-ROM",
		.type   = ITEM_ACTION,
		.action = { .callback = enterFastRebootMenu }
	}, {
		.name   = "Reboot system",
		.type   = ITEM_ACTION,
		.action = { .callback = doFullReboot }
	}, {
		.type = ITEM_SEPARATOR
	}, {
		.name   = "About this tool...",
		.type   = ITEM_ACTION,
		.action = { .callback = enterAboutMenu }
	}, {
		.type = ITEM_END
	}
};

void enterMainMenu(RenderContext *ctx, UIState *state, const MenuItem *item) {
	(void) ctx;
	(void) item;

	state->currentMenu = mainMenu;
	state->menuCursor  = 0;
}
