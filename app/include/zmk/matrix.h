#pragma once

#include <devicetree.h>

#define ZMK_MATRIX_NODE_ID DT_CHOSEN(zmk_kscan)

#if DT_NODE_HAS_PROP(ZMK_MATRIX_NODE_ID,row_gpios)
#define ZMK_MATRIX_ROWS DT_PROP_LEN(ZMK_MATRIX_NODE_ID,row_gpios)
#define ZMK_MATRIX_COLS DT_PROP_LEN(ZMK_MATRIX_NODE_ID,col_gpios)
#else
#define ZMK_MATRIX_ROWS DT_PROP(ZMK_MATRIX_NODE_ID,rows)
#define ZMK_MATRIX_COLS DT_PROP(ZMK_MATRIX_NODE_ID,columns)
#endif

