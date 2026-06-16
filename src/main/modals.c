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
#include "main/renderer.h"
#include "main/ui.h"
#include "ps1/registers.h"

#define MENU_TITLE(text) {.name = text, .type = ITEM_TITLE}
#define MENU_TEXT(text) {.name = text, .type = ITEM_STATIC}
#define MENU_SEPARATOR() {.type = ITEM_SEPARATOR}
#define MENU_ACTION(t, a)                                          \
    {                                                              \
        .name = t, .type = ITEM_ACTION, .action = {.callback = a } \
    }
#define MENU_END() {.type = ITEM_END}

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
    MENU_TITLE("WARNINNG"),
    MENU_SEPARATOR(),
    MENU_TEXT("CD-ROM booting relies on injecting kernel patches to apply"),
    MENU_TEXT("the new RAM configuration. This process is by its nature"),
    MENU_TEXT("hacky and may introduce compatibility issues. Moreover,"),
    MENU_TEXT("some games and applications are known to break when the"),
    MENU_TEXT("default main RAM configuration is changed."),
    MENU_SEPARATOR(),
    MENU_TEXT("If you understand the risks, insert a valid disc and close"),
    MENU_TEXT("the lid before proceeding."),
    MENU_SEPARATOR(),
    MENU_ACTION("Boot CD-ROM", doFastReboot),
    MENU_ACTION("Cancel", enterMainMenu),
    MENU_END(),
};

static const MenuItem rebootErrorMenu[] = {
    MENU_TITLE("ERROR"),
    MENU_SEPARATOR(),
    MENU_TEXT("This console's BIOS ROM version is not supported or not"),
    MENU_TEXT("compatible with the kernel patch used for CD-ROM booting."),
    MENU_TEXT("Only Sony's own retail BIOS ROMs are currently supported."),
    MENU_SEPARATOR(),
    MENU_ACTION("Back", enterMainMenu),
    MENU_END(),
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
    MENU_TEXT("- " PROJECT_NAME " -"),
    MENU_SEPARATOR(),
    MENU_TEXT("Version " VERSION_STRING),
    MENU_TEXT("Copyright (C) 2026 spicyjpeg, NaokiS28"),
    MENU_SEPARATOR(),
    MENU_TEXT("Licensed under the MIT license."),
    MENU_TEXT("Source code available at:"),
    MENU_TEXT("    <https://github.com/spicyjpeg/ps1-ram-tester>"),
    MENU_TEXT("This tool is completely free and open source. Hopefully"),
    MENU_TEXT("you did not get tricked into paying for it."),
    MENU_SEPARATOR(),
    MENU_TEXT("Developed with no AI assistance whatsoever."),
    MENU_SEPARATOR(),
    MENU_ACTION("Back", enterMainMenu),
    MENU_END(),
};

void enterAboutMenu(RenderContext *ctx, UIState *state, const MenuItem *item) {
	(void) ctx;
	(void) item;

	state->currentMenu = aboutMenu;
	state->menuCursor  = (sizeof(aboutMenu) / sizeof(MenuItem)) - 2;
}
