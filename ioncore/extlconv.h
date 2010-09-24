/*
 * ion/ioncore/extlconv.h
 *
 * Copyright (c) Tuomo Valkonen 1999-2009. 
 *
 * See the included file LICENSE for details.
 */

#ifndef ION_IONCORE_EXTLCONV_H
#define ION_IONCORE_EXTLCONV_H

#include <libextl/extl.h>
#include <libtu/iterable.h>
#include <libtu/setparam.h>
#include <libtu/objlist.h>
#include "common.h"
#include "region.h"

extern bool extl_iter_obj(ExtlFn fn, Obj *obj);
extern bool extl_iter_objlist_(ExtlFn fn, ObjIterator *iter, void *st);
extern bool extl_iter_objlist(ExtlFn fn, ObjList *list);

extern bool extl_table_is_bool_set(ExtlTab tab, const char *entry);

extern bool extl_table_to_rectangle(ExtlTab tab, WRectangle *rect);
extern ExtlTab extl_table_from_rectangle(const WRectangle *rect);

extern bool extl_table_gets_rectangle(ExtlTab tab, const char *nam,
                                      WRectangle *rect);
extern void extl_table_sets_rectangle(ExtlTab tab, const char *nam,
                                      const WRectangle *rect);

#endif /* ION_IONCORE_EXTLCONV_H */

