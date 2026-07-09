/*
 * ps1-ram-tester - UI sound effects
 *
 * Uploads a single one-shot .VAG sample (the standard PS1 SPU-ADPCM format)
 * to SPU RAM once, then plays it on either of two dedicated channels
 * depending on whether it's triggered by menu scrolling or confirming a
 * selection - both share the same underlying sample data, just on
 * different channels, so triggering one never cuts the other off early.
 *
 * Built entirely on top of this project's own common/spu.c driver and
 * ps1/registers.h definitions, no external SDK involved.
 */

#include <stdint.h>
#include "common/spu.h"
#include "main/sound.h"
#include "ps1/registers.h"

// The pickup.vag data embedded via addBinaryFile() in CMakeLists.txt.
extern const uint8_t uiSound[];

#define SCROLL_CHANNEL  0
#define CONFIRM_CHANNEL 1

// Standard 48-byte .VAG header. All multi-byte fields are big-endian.
typedef struct {
	char     id[4];        // "VAGp"
	uint32_t version;
	uint32_t reserved;
	uint32_t dataSize;
	uint32_t sampleRate;
	uint8_t  reserved2[12];
	char     name[16];
} VAGHeader;

static uint32_t swapEndian(uint32_t value) {
	return ((value & 0x000000ff) << 24)
	     | ((value & 0x0000ff00) <<  8)
	     | ((value & 0x00ff0000) >>  8)
	     | ((value & 0xff000000) >> 24);
}

static uint32_t uiSoundOffset = 0;

void initSound(void) {
	const VAGHeader *header = (const VAGHeader *) uiSound;

	uint32_t dataSize    = swapEndian(header->dataSize);
	// SPU DMA transfers are done in 64-byte blocks.
	uint32_t alignedSize = (dataSize + 63) & ~((uint32_t) 63);

	uiSoundOffset = SPU_RAM_ALLOC_OFFSET;
	sendSPURAMData(uiSound + sizeof(VAGHeader), uiSoundOffset, alignedSize);
	waitForSPUDMADone();
}

static void playUISound(int channel) {
	const VAGHeader *header = (const VAGHeader *) uiSound;

	uint32_t sampleRate = swapEndian(header->sampleRate);
	uint32_t pitch       = (sampleRate << 12) / 44100;

	// Stop the channel first in case it's still playing from a previous
	// trigger, then reconfigure and key it back on.
	if (channel < 16)
		SPU_KOFF0 = 1 << channel;
	else
		SPU_KOFF1 = 1 << (channel - 16);

	SPU_CH_VOLL (channel) = SPU_MAX_VOLUME;
	SPU_CH_VOLR (channel) = SPU_MAX_VOLUME;
	SPU_CH_PITCH(channel) = pitch;
	SPU_CH_SSA  (channel) = uiSoundOffset / SPU_RAM_ADDR_UNIT;
	// Instant attack, slowest decay, max sustain level: holds at full
	// volume until the sample's own end-of-data flag stops it.
	SPU_CH_ADSR1(channel) = 0x00ff;
	SPU_CH_ADSR2(channel) = 0x0000;

	if (channel < 16)
		SPU_KON0 = 1 << channel;
	else
		SPU_KON1 = 1 << (channel - 16);
}

void playScrollSound(void) {
	playUISound(SCROLL_CHANNEL);
}

void playConfirmSound(void) {
	playUISound(CONFIRM_CHANNEL);
}
