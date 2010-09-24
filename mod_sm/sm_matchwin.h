/*
 * ion/mod_sm/sm_mathcwin.c
 *
 * Copyright (c) Tuomo Valkonen 2004-2009. 
 * 
 * Based on the code of the 'sm' module for Ion1 by an unknown contributor.
 *
 * This is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 */

#ifndef ION_MOD_SM_MATCHWIN_H
#define ION_MOD_SM_MATCHWIN_H

#include <libtu/obj.h>
#include <ioncore/common.h>
#include <ioncore/pholder.h>

INTRSTRUCT(WWinMatch);

DECLSTRUCT(WWinMatch){
    WPHolder *pholder;
    
    char *client_id;
    char *window_role;
    char *wclass;
    char *winstance;
    char *wm_name;
    char *wm_cmd;
    
    WWinMatch *next, *prev;
};

extern WPHolder *mod_sm_match_cwin_to_saved(WClientWin *cwin);
extern void mod_sm_register_win_match(WWinMatch *match);
extern char *mod_sm_get_window_cmd(Window window);
extern char *mod_sm_get_client_id(Window window);
extern char *mod_sm_get_window_role(Window window);
extern bool mod_sm_have_match_list();
extern void mod_sm_start_purge_timer();

extern bool mod_sm_add_match(WPHolder *ph, ExtlTab tab);
extern void mod_sm_get_configuration(WClientWin *cwin, ExtlTab tab);

#endif /* ION_MOD_SM_MATCHWIN_H */
