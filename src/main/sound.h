/*
 * ps1-ram-tester - UI sound effects
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Uploads the UI sound effect to SPU RAM. Must be called once after
// initSPU().
void initSound(void);

// Plays the sound effect on the "scroll" channel (menu cursor movement).
void playScrollSound(void);

// Plays the same sound effect on a separate "confirm" channel (selecting
// a menu item), so a rapid scroll-then-confirm doesn't cut either sound
// off early.
void playConfirmSound(void);

#ifdef __cplusplus
}
#endif
