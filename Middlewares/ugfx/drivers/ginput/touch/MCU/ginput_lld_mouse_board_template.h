/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#ifndef _GINPUT_LLD_MOUSE_BOARD_H
#define _GINPUT_LLD_MOUSE_BOARD_H

static inline void init_board(void) {
}

static inline void aquire_bus(void) {
}

static inline void release_bus(void) {
}

static inline void setup_x(void) {
}

static inline void setup_y(void) {
}

static inline void setup_z(void) {
	palClearPad(GPIOB, GPIOB_DRIVEA);
	palClearPad(GPIOB, GPIOB_DRIVEB);
    chThdSleepMilliseconds(2);
}

static inline uint16_t read_x(void) {
}

static inline uint16_t read_y(void) {
}

static inline uint16_t read_z(void) {
}

#endif /* _GINPUT_LLD_MOUSE_BOARD_H */
