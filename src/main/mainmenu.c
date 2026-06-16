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

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "common/reboot.h"
#include "common/spu.h"
#include "main/mainmenu.h"
#include "main/modals.h"
#include "main/ramconfig.h"
#include "main/renderer.h"
#include "main/test.h"
#include "main/ui.h"
#include "ps1/gpucmd.h"
#include "ps1/registers.h"

/* Menu callbacks */

uint8_t vramSize   =  0;
uint8_t testPasses = 10;

static char mainRAMConfig[32] = "";
static char finalResult[32]   = "";
static char mainRAMResult[32] = "";
static char vramResult[32]    = "";
static char spuRAMResult[32]  = "";

static const char *testMessage = 0;

extern char _bssEnd[];

static void testCallback(void *arg, int progress, int total) {
	RenderContext *ctx = (RenderContext *) arg;
	char          message[64];

	snprintf(message, sizeof(message), testMessage, progress / 2 + 1);

	beginFrame(ctx);
	renderProgressScreen(ctx, progress, total, message);
	endFrame(ctx);
}

static void runMainRAMTest(
	RenderContext  *ctx,
	UIState        *state,
	const MenuItem *item
) {
	testMessage = "Running main RAM test... (pass %d)";

	size_t    size = getMainRAMSize();
	TestError error;

	if (testMainRAM(
		&error,
		testCallback,
		ctx,
		(uintptr_t) _bssEnd,
		RAM_BASE | size,
		testPasses
	))
		snprintf(
			mainRAMResult,
			sizeof(mainRAMResult),
			"Passed (%d MB)",
			size / 0x100000
		);
	else
		snprintf(
			mainRAMResult,
			sizeof(mainRAMResult),
			"Error at 0x%08X (pass %d)",
			error.address,
			error.pass + 1
		);

	enterMainMenu(ctx, state, item);
}

static void runVRAMTest(
	RenderContext  *ctx,
	UIState        *state,
	const MenuItem *item
) {
	testMessage = "Running VRAM test... (pass %d)";

	GPU_GP1 = gp1_vramSize(vramSize);

	TestError error;

	if (testVRAM(
		&error,
		0,
		0,
		0,
		512 << vramSize,
		testPasses
	))
		snprintf(
			vramResult,
			sizeof(vramResult),
			"Passed (%d MB)",
			1 << vramSize
		);
	else
		snprintf(
			vramResult,
			sizeof(vramResult),
			"Error at 0x%06X (pass %d)",
			error.address,
			error.pass + 1
		);

	reloadTextures(ctx);
	enterMainMenu(ctx, state, item);
}

static void runSPURAMTest(
	RenderContext  *ctx,
	UIState        *state,
	const MenuItem *item
) {
	testMessage = "Running SPU RAM test... (pass %d)";

	TestError error;

	if (testSPURAM(
		&error,
		testCallback,
		ctx,
		SPU_RAM_ALLOC_OFFSET,
		SPU_RAM_SIZE,
		testPasses
	))
		snprintf(
			spuRAMResult,
			sizeof(spuRAMResult),
			"Passed (%d KB)",
			SPU_RAM_SIZE / 1024
		);
	else
		snprintf(
			spuRAMResult,
			sizeof(spuRAMResult),
			"Error at 0x%05X (pass %d)",
			error.address,
			error.pass + 1
		);

	enterMainMenu(ctx, state, item);
}

static void runAllTests(
	RenderContext  *ctx,
	UIState        *state,
	const MenuItem *item
) {
	runMainRAMTest(ctx, state, item);
	runVRAMTest(ctx, state, item);
	runSPURAMTest(ctx, state, item);
}

/* Main menu */

static const MenuItem mainMenu[] = {
	MENU_ACTION_TAG("Configure main RAM...", enterRAMConfigMenu, mainRAMConfig),
	MENU_ENUM("VRAM size", 0, 1, vramSize, "1 MB (retail/dev)", "2 MB (arcade)"),
	MENU_SEPARATOR(),
	MENU_INT("Number of test passes", 1, 250, testPasses),
	MENU_SEPARATOR(),
	MENU_ACTION_TAG("Test main RAM, VRAM and SPU RAM", runAllTests, finalResult),
	MENU_ACTION_TAG("Test main RAM", runMainRAMTest, mainRAMResult),
	MENU_ACTION_TAG("Test VRAM", runVRAMTest, vramResult),
	MENU_ACTION_TAG("Test SPU RAM", runSPURAMTest, spuRAMResult),
	MENU_SEPARATOR(),
	MENU_TEXT("Warning: the screen will flicker while testing VRAM."),
	MENU_SEPARATOR(),
	MENU_ACTION("Boot CD-ROM with current configuration", enterFastRebootMenu),
	MENU_ACTION("Reboot system (discards configuration)", doFullReboot),
	MENU_SEPARATOR(),
	MENU_ACTION("About this tool...", enterAboutMenu),
	MENU_END(),
};

void enterMainMenu(RenderContext *ctx, UIState *state, const MenuItem *item) {
	(void) ctx;
	(void) item;

	snprintf(
		mainRAMConfig,
		sizeof(mainRAMConfig),
		"%d MB [0x%04X]",
		getMainRAMSize() / 0x100000,
		DRAM_CTRL & 0xffff
	);

	// Somewhat ugly hack, but it works.
	if (
		(mainRAMResult[0] == 'E') ||
		(vramResult[0]    == 'E') ||
		(spuRAMResult[0]  == 'E')
	)
		__builtin_strncpy(finalResult, "Failed", sizeof(finalResult));
	else if (
		(mainRAMResult[0] == 'P') &&
		(vramResult[0]    == 'P') &&
		(spuRAMResult[0]  == 'P')
	)
		__builtin_strncpy(finalResult, "Passed", sizeof(finalResult));
	else
		finalResult[0] = 0;

	state->currentMenu = mainMenu;
	state->menuCursor  = 0;
}
