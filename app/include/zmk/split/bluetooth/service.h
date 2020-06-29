#pragma once

#include <bluetooth/uuid.h>

#ifndef BT_UUID_NUM_OF_DIGITALS
#define BT_UUID_NUM_OF_DIGITALS           BT_UUID_DECLARE_16(0x2909)
#endif

#define ZMK_SPLIT_BT_BASE_UUID 0x2a, 0x48, 0xc2, 0xb1, 0xcf, 0xc5, 0x67, 0xc9, 0x07, 0x71, 0x96, 0x00
#define ZMK_SPLIT_BT_SERVICE_UUID ZMK_SPLIT_BT_BASE_UUID, 0x00, 0x00, 0x00, 0x00
#define ZMK_SPLIT_BT_CHAR_POSITION_STATE_UUID ZMK_SPLIT_BT_BASE_UUID, 0x01, 0x00, 0x00, 0x00

#define ZMK_BT_UUID_SPLIT           BT_UUID_DECLARE_128(ZMK_SPLIT_BT_SERVICE_UUID)
#define ZMK_BT_UUID_SPLIT_POS_STATE BT_UUID_DECLARE_128(ZMK_SPLIT_BT_CHAR_POSITION_STATE_UUID)

int zmk_split_bt_position_pressed(u8_t position);
int zmk_split_bt_position_released(u8_t position);