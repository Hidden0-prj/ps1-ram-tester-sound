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

#include "common/reboot.h"
#include "main/defs.h"
#include "main/mainmenu.h"
#include "main/modals.h"
#include "main/ramtester.h"
#include "main/renderer.h"
#include "main/ui.h"
#include "ps1/registers.h"

/* Reboot functions */

static void showRebootProgress(RenderContext *ctx, const char *message) {
	// This needs to be done multiple times in order to completely "flush" the
	// rendering pipeline.
	for (int i = 0; i < 3; i++) {
		beginFrame(ctx);
		renderProgressScreen(ctx, 0, 1, message);
		endFrame(ctx);
	}
}

static void doFastReboot(
	RenderContext  *ctx,
	UIState        *state,
	const MenuItem *item
) {
	(void) state;
	(void) item;

	showRebootProgress(ctx, "Waiting for kernel to load CD-ROM...");
	// vramSize is set by the RAM/VRAM/SPU RAM tester submenu, defaulting to
	// 0 (standard 1 MB VRAM) if that submenu was never entered.
	softFastRebootWithConfig(DRAM_CTRL & 0xffff, vramSize);
	__builtin_unreachable();
}

void doFullReboot(RenderContext *ctx, UIState *state, const MenuItem *item) {
	(void) state;
	(void) item;

	showRebootProgress(ctx, "Waiting for kernel to reboot...");
	softReset();
	__builtin_unreachable();
}

/* Fast reboot warning and error menus */

static const MenuItem rebootWarningMenu[] = {
	{
		.name = "Warning",
		.type = ITEM_TITLE
	}, {
		.type = ITEM_SEPARATOR
	}, {
		.name = "CD-ROM booting relies on injecting kernel patches to apply",
		.type = ITEM_STATIC
	}, {
		.name = "the new RAM configuration. This process is by its nature",
		.type = ITEM_STATIC
	}, {
		.name = "hacky and may introduce compatibility issues. Moreover,",
		.type = ITEM_STATIC
	}, {
		.name = "some games and applications are known to break when the",
		.type = ITEM_STATIC
	}, {
		.name = "default main RAM configuration is changed.",
		.type = ITEM_STATIC
	}, {
		.type = ITEM_SEPARATOR
	}, {
		.name = "If you understand the risks, insert a valid disc and close",
		.type = ITEM_STATIC
	}, {
		.name = "the lid before proceeding.",
		.type = ITEM_STATIC
	}, {
		.type = ITEM_SEPARATOR
	}, {
		.name   = "Boot CD-ROM",
		.type   = ITEM_ACTION,
		.action = { .callback = doFastReboot }
	}, {
		.name   = "Cancel",
		.type   = ITEM_ACTION,
		.action = { .callback = enterMainMenu }
	}, {
		.type = ITEM_END
	}
};

static const MenuItem rebootErrorMenu[] = {
	{
		.name = "Error",
		.type = ITEM_TITLE
	}, {
		.type = ITEM_SEPARATOR
	}, {
		.name = "This console's BIOS ROM version is not supported or not",
		.type = ITEM_STATIC
	}, {
		.name = "compatible with the kernel patch used for CD-ROM booting.",
		.type = ITEM_STATIC
	}, {
		.name = "Only Sony's own retail BIOS ROMs are currently supported.",
		.type = ITEM_STATIC
	}, {
		.type = ITEM_SEPARATOR
	}, {
		.name   = "Back",
		.type   = ITEM_ACTION,
		.action = { .callback = enterMainMenu }
	}, {
		.type = ITEM_END
	}
};

void enterFastRebootMenu(
	RenderContext  *ctx,
	UIState        *state,
	const MenuItem *item
) {
	(void) ctx;
	(void) item;

	if (isFastRebootCompatible()) {
		state->currentMenu = rebootWarningMenu;
		state->menuCursor  = (sizeof(rebootWarningMenu) / sizeof(MenuItem)) - 2;
	} else {
		state->currentMenu = rebootErrorMenu;
		state->menuCursor  = (sizeof(rebootErrorMenu)   / sizeof(MenuItem)) - 2;
	}
}

/* "About ps1-ram-tester" menu */

static const MenuItem aboutMenu[] = {
	{
		.name = "- PSX-iTests -",
		.type = ITEM_TITLE
	}, {
		.type = ITEM_SEPARATOR
	}, {
		.name = "Version " VERSION_STRING,
		.type = ITEM_STATIC
	}, {
		.name = "Basic CPU/GPU/SPU functionality test tool.",
		.type = ITEM_STATIC
	}, {
		.type = ITEM_SEPARATOR
	}, {
		.name = "Built on the ps1-bare-metal library and",
		.type = ITEM_STATIC
	}, {
		.name = "ps1-ram-tester's UI framework by spicyjpeg,",
		.type = ITEM_STATIC
	}, {
		.name = "licensed under the MIT license.",
		.type = ITEM_STATIC
	}, {
		.name = "    <https://github.com/spicyjpeg/ps1-bare-metal>",
		.type = ITEM_STATIC
	}, {
		.type = ITEM_SEPARATOR
	}, {
		.name = "This tool is completely free and open source. Hopefully",
		.type = ITEM_STATIC
	}, {
		.name = "you did not get tricked into paying for it.",
		.type = ITEM_STATIC
	}, {
		.type = ITEM_SEPARATOR
	}, {
		.name = "Developed with no AI assistance whatsoever.",
		.type = ITEM_STATIC
	}, {
		.type = ITEM_SEPARATOR
	}, {
		.name   = "Back",
		.type   = ITEM_ACTION,
		.action = { .callback = enterMainMenu }
	}, {
		.type = ITEM_END
	}
};

void enterAboutMenu(RenderContext *ctx, UIState *state, const MenuItem *item) {
	(void) ctx;
	(void) item;

	state->currentMenu = aboutMenu;
	state->menuCursor  = (sizeof(aboutMenu) / sizeof(MenuItem)) - 2;
}
