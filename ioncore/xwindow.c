/*
 * ion/ioncore/xwindow.c
 *
 * Copyright (c) Tuomo Valkonen 1999-2009. 
 *
 * See the included file LICENSE for details.
 */

#include <string.h>

#include <libtu/minmax.h>
#include "common.h"
#include "global.h"
#include "xwindow.h"
#include "cursor.h"
#include "sizehint.h"


/*{{{ X window->region mapping */


WRegion *xwindow_region_of(Window win)
{
    WRegion *reg;
    
    if(XFindContext(ioncore_g.dpy, win, ioncore_g.win_context,
                    (XPointer*)&reg)!=0)
        return NULL;
    
    return reg;
}


WRegion *xwindow_region_of_t(Window win, const ClassDescr *descr)
{
    WRegion *reg=xwindow_region_of(win);
    
    if(reg==NULL)
        return NULL;
    
    if(!obj_is((Obj*)reg, descr))
        return NULL;
    
    return reg;
}


/*}}}*/


/*{{{ Create */


Window create_xwindow(WRootWin *rw, Window par, const WRectangle *geom)
{
    int w=maxof(1, geom->w);
    int h=maxof(1, geom->h);
    
    return XCreateSimpleWindow(ioncore_g.dpy, par, geom->x, geom->y, w, h,
                               0, 0, BlackPixel(ioncore_g.dpy, rw->xscr));
}


/*}}}*/


/*{{{ Restack */


void xwindow_restack(Window win, Window other, int stack_mode)
{
    XWindowChanges wc;
    int wcmask;
    
    wcmask=CWStackMode;
    wc.stack_mode=stack_mode;
    if(other!=None){
        wc.sibling=other;
        wcmask|=CWSibling;
    }

    XConfigureWindow(ioncore_g.dpy, win, wcmask, &wc);
}


/*}}}*/


/*{{{ Focus */


void xwindow_do_set_focus(Window win)
{
    XSetInputFocus(ioncore_g.dpy, win, RevertToParent, CurrentTime);
}


/*}}}*/


/*{{{ Pointer and cursors */

void xwindow_set_cursor(Window win, int cursor)
{
    XDefineCursor(ioncore_g.dpy, win, ioncore_xcursor(cursor));
}


bool xwindow_pointer_pos(Window rel, int *px, int *py)
{
    Window win=None, realroot=None;
    int rx=0, ry=0;
    uint mask=0;
    return XQueryPointer(ioncore_g.dpy, rel, &realroot, &win,
                         &rx, &ry, px, py, &mask);
}

/*}}}*/


/*{{{ Size hints */


void xwindow_get_sizehints(Window win, XSizeHints *hints)
{
    int minh, minw;
    long supplied=0;
    
    memset(hints, 0, sizeof(*hints));
    XGetWMNormalHints(ioncore_g.dpy, win, hints, &supplied);
    
    xsizehints_sanity_adjust(hints);
}


/*}}}*/

