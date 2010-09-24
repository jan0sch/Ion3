/*
 * config.h
 *
 * Copyright (c) Tuomo Valkonen 1999-2009.
 *
 * See the included file LICENSE for details.
 */

#ifndef ION_CONFIG_H
#define ION_CONFIG_H

/* #define CF_NO_LOCK_HACK */

#define CF_FALLBACK_FONT_NAME "fixed"
/*#define CF_FALLBACK_FONT_NAME "-*-helvetica-medium-r-normal-*-12-*-*-*-*-*-*-*"*/
#define CF_DRAG_TRESHOLD 2
#define CF_DBLCLICK_DELAY 250

#define CF_MAX_MOVERES_STR_SIZE 32

#define CF_RESIZE_DELAY 1500

#define CF_XMESSAGE "xmessage -file "

#define CF_EDGE_RESISTANCE 16

#define CF_RAISE_DELAY 500

#define CF_STATUSBAR_SYSTRAY_HEIGHT 24

#define CF_VISIBILITY_CONSTRAINT 16 /* = CF_EDGE_RESISTANCE */

/* Cursors
 */

#define CF_CURSOR_DEFAULT XC_left_ptr
#define CF_CURSOR_RESIZE XC_sizing
#define CF_CURSOR_MOVE XC_fleur
#define CF_CURSOR_DRAG XC_cross
#define CF_CURSOR_WAITKEY XC_icon

#define CF_STDISP_MIN_SZ 8

#endif /* ION_CONFIG_H */
