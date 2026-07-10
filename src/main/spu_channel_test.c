/*
 * PSX-iTests - SPU channel test
 *
 * Automatically cycles through all 24 SPU channels in sequence, playing a
 * short test tone on each one in turn, to confirm every channel actually
 * outputs sound - no manual stepping required, just watch/listen and let
 * it run. Since channels 0-2 are normally used for the scroll/confirm/BGM
 * sounds, testing them interrupts that playback - normal audio is restored
 * on exit the same way the SPU RAM test restores it. Loops back to channel
 * 0 after the last channel and keeps going until any button is pressed.
 */

#include <stdint.h>
#include <stdio.h>
#include "common/sio0.h"
#include "common/spu.h"
#include "main/font.h"
#include "main/mainmenu.h"
#include "main/sound.h"
#include "main/spu_channel_test.h"

// How many frames to hold on each channel before advancing to the next one.
// At 60 fps this is 0.75 s, long enough to clearly hear the ~0.16 s test
// tone with a bit of a gap before the next channel starts.
#define FRAMES_PER_CHANNEL 45

void runSPUChannelTest(
	RenderContext  *ctx,
	UIState        *state,
	const MenuItem *item
) {
	int channel      = 0;
	int frameInStep  = 0;

	// Debounce: wait for the button that opened this screen to be released.
	while (pollController(0) | pollController(1))
		;

	playTestTone(channel);

	for (;;) {
		if (pollController(0) | pollController(1))
			break;

		frameInStep++;
		if (frameInStep >= FRAMES_PER_CHANNEL) {
			frameInStep = 0;
			channel     = (channel + 1) % SPU_NUM_CHANNELS;
			playTestTone(channel);
		}

		char line[64];

		beginFrame(ctx);
		drawBackground(ctx);

		printString(ctx, 16, 20, 0x808080, "SPU CHANNEL TEST");

		snprintf(
			line, sizeof(line), "Channel: %d / %d", channel + 1,
			SPU_NUM_CHANNELS
		);
		printString(ctx, 16, 60, 0xffffff, line);

		printString(ctx, 16,  90, 0x808080, "Auto-cycling through all channels...");
		printString(ctx, 16, 102, 0x808080, "Any button: return to menu");

		endFrame(ctx);
	}

	// Restore normal scroll/confirm/BGM playback on channels 0-2.
	initSound();
	playBGM();

	enterMainMenu(ctx, state, item);
}
