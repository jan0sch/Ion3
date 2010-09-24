/*
 * ion/mod_query/query.c
 *
 * Copyright (c) Tuomo Valkonen 1999-2009. 
 *
 * See the included file LICENSE for details.
 */


#include <ioncore/common.h>
#include <ioncore/global.h>
#include <ioncore/focus.h>
#include <ioncore/frame.h>
#include <ioncore/stacking.h>
#include <libtu/objp.h>
#include "wmessage.h"
#include "fwarn.h"


/*(internal) EXTL_DOC
 * Display an error message box in the multiplexer \var{mplex}.
 */
EXTL_EXPORT
WMessage *mod_query_do_warn(WMPlex *mplex, const char *p)
{
    char *p2;
    WMessage *wmsg;
    
    if(p==NULL)
        return NULL;
    
    p2=scat(TR("Error:\n"), p);
    
    if(p2==NULL)
        return NULL;
    
    wmsg=mod_query_do_message(mplex, p2);
    
    free(p2);
    
    return wmsg;
}


/*(internal) EXTL_DOC
 * Display a message in the \var{mplex}.
 */
EXTL_EXPORT
WMessage *mod_query_do_message(WMPlex *mplex, const char *p)
{
    WMPlexAttachParams par;

    if(p==NULL)
        return NULL;
    
    par.flags=(MPLEX_ATTACH_SWITCHTO|
               MPLEX_ATTACH_LEVEL|
               MPLEX_ATTACH_UNNUMBERED|
               MPLEX_ATTACH_SIZEPOLICY);
    par.szplcy=SIZEPOLICY_FULL_BOUNDS;
    par.level=STACKING_LEVEL_MODAL1+2;

    return (WMessage*)mplex_do_attach_new(mplex, &par,
                                          (WRegionCreateFn*)create_wmsg,
                                          (void*)p);
}

