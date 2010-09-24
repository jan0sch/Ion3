/*
 * ion/ioncore/names.h
 *
 * Copyright (c) Tuomo Valkonen 1999-2009. 
 *
 * See the included file LICENSE for details.
 */

#ifndef ION_IONCORE_NAMES_H
#define ION_IONCORE_NAMES_H

#include "region.h"
#include "clientwin.h"
#include "gr.h"
#include <libextl/extl.h>


typedef struct{
    struct rb_node *rb;
    bool initialised;
} WNamespace;


extern WNamespace ioncore_internal_ns;
extern WNamespace ioncore_clientwin_ns;


extern bool region_register(WRegion *reg);
extern bool region_set_name(WRegion *reg, const char *name);
extern bool region_set_name_exact(WRegion *reg, const char *name);

extern bool clientwin_register(WClientWin *cwin);
extern bool clientwin_set_name(WClientWin *cwin, const char *name);

extern void region_unregister(WRegion *reg);
extern void region_do_unuse_name(WRegion *reg, bool insert_unnamed);

extern const char *region_name(WRegion *reg);
DYNFUN const char *region_displayname(WRegion *reg);

extern char *region_make_label(WRegion *reg, int maxw, GrBrush *brush);

extern bool ioncore_region_i(ExtlFn fn, const char *typenam);
extern bool ioncore_clientwin_i(ExtlFn fn);
extern WRegion *ioncore_lookup_region(const char *cname, const char *typenam);
extern WClientWin *ioncore_lookup_clientwin(const char *cname);

#endif /* ION_IONCORE_NAMES_H */
