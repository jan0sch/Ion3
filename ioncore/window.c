/*
 * ion/ioncore/window.c
 *
 * Copyright (c) Tuomo Valkonen 1999-2009. 
 *
 * See the included file LICENSE for details.
 */

#include <libtu/objp.h>
#include <libtu/minmax.h>

#include "common.h"
#include "global.h"
#include "window.h"
#include "focus.h"
#include "rootwin.h"
#include "region.h"
#include "xwindow.h"
#include "region-iter.h"


/*{{{ Dynfuns */


void window_draw(WWindow *wwin, bool complete)
{
    CALL_DYN(window_draw, wwin, (wwin, complete));
}


void window_insstr(WWindow *wwin, const char *buf, size_t n)
{
    CALL_DYN(window_insstr, wwin, (wwin, buf, n));
}


int window_press(WWindow *wwin, XButtonEvent *ev, WRegion **reg_ret)
{
    int area=0;
    CALL_DYN_RET(area, int, window_press, wwin, (wwin, ev, reg_ret));
    return area;
}


void window_release(WWindow *wwin)
{
    CALL_DYN(window_release, wwin, (wwin));
}


/*}}}*/
    

/*{{{ Init, create */


bool window_do_init(WWindow *wwin, WWindow *par, 
                    const WFitParams *fp, Window win)
{
    if(win==None){
        assert(par!=NULL);
        win=create_xwindow(region_rootwin_of((WRegion*)par),
                           par->win, &(fp->g));
        if(win==None)
            return FALSE;
    }

    wwin->win=win;
    wwin->xic=NULL;
    wwin->event_mask=0;
    wwin->stacking=NULL;
    
    region_init(&(wwin->region), par, fp);
    
    XSaveContext(ioncore_g.dpy, win, ioncore_g.win_context, 
                 (XPointer)wwin);
    
    return TRUE;
}


bool window_init(WWindow *wwin, WWindow *par, const WFitParams *fp)
{
    return window_do_init(wwin, par, fp, None);
}


void window_deinit(WWindow *wwin)
{
    region_deinit((WRegion*)wwin);
    
    region_pointer_focus_hack(&wwin->region);
    
    if(wwin->xic!=NULL)
        XDestroyIC(wwin->xic);
        
    if(wwin->win!=None){
        XDeleteContext(ioncore_g.dpy, wwin->win, ioncore_g.win_context);
        /* Probably should not try destroy if root window... */
        XDestroyWindow(ioncore_g.dpy, wwin->win);
    }
    
    /* There are no backlinks from WStacking to us, so it is not
     * necessary to do any deinitialisation there.
     */
}


/*}}}*/


/*{{{ Region dynfuns */


static void window_notify_subs_rootpos(WWindow *wwin, int x, int y)
{
    WRegion *sub;
    
    FOR_ALL_CHILDREN(wwin, sub){
        region_notify_rootpos(sub,
                              x+REGION_GEOM(sub).x, 
                              y+REGION_GEOM(sub).y);
    }
}


void window_notify_subs_move(WWindow *wwin)
{
    int x=0, y=0;
    region_rootpos(&(wwin->region), &x, &y);
    window_notify_subs_rootpos(wwin, x, y);
}


void window_do_fitrep(WWindow *wwin, WWindow *par, const WRectangle *geom)
{
    bool move=(REGION_GEOM(wwin).x!=geom->x ||
               REGION_GEOM(wwin).y!=geom->y);
    int w=maxof(1, geom->w);
    int h=maxof(1, geom->h);

    if(par!=NULL){
        region_unset_parent((WRegion*)wwin);
        XReparentWindow(ioncore_g.dpy, wwin->win, par->win, geom->x, geom->y);
        XResizeWindow(ioncore_g.dpy, wwin->win, w, h);
        region_set_parent((WRegion*)wwin, par);
    }else{
        XMoveResizeWindow(ioncore_g.dpy, wwin->win, geom->x, geom->y, w, h);
    }
    
    REGION_GEOM(wwin)=*geom;

    if(move)
        window_notify_subs_move(wwin);
}


bool window_fitrep(WWindow *wwin, WWindow *par, const WFitParams *fp)
{
    if(par!=NULL && !region_same_rootwin((WRegion*)wwin, (WRegion*)par))
        return FALSE;
    window_do_fitrep(wwin, par, &(fp->g));
    return TRUE;
}


void window_map(WWindow *wwin)
{
    XMapWindow(ioncore_g.dpy, wwin->win);
    REGION_MARK_MAPPED(wwin);
}


void window_unmap(WWindow *wwin)
{
    region_pointer_focus_hack(&wwin->region);

    XUnmapWindow(ioncore_g.dpy, wwin->win);
    REGION_MARK_UNMAPPED(wwin);
}


void window_do_set_focus(WWindow *wwin, bool warp)
{
    region_finalise_focusing((WRegion*)wwin, wwin->win, warp);
}


void window_restack(WWindow *wwin, Window other, int mode)
{
    xwindow_restack(wwin->win, other, mode);
}


Window window_xwindow(const WWindow *wwin)
{
    return wwin->win;
}


/*}}}*/


/*{{{ Misc. */


/*EXTL_DOC
 * Return the X window id for \var{wwin}.
 */
EXTL_SAFE
EXTL_EXPORT_MEMBER
double window_xid(WWindow *wwin)
{
    return wwin->win;
}


void window_select_input(WWindow *wwin, long event_mask)
{
    XSelectInput(ioncore_g.dpy, wwin->win, event_mask);
    wwin->event_mask=event_mask;
}


/*}}}*/


/*{{{ Dynamic function table and class implementation */


static DynFunTab window_dynfuntab[]={
    {region_map, window_map},
    {region_unmap, window_unmap},
    {region_do_set_focus, window_do_set_focus},
    {(DynFun*)region_fitrep, (DynFun*)window_fitrep},
    {(DynFun*)region_xwindow, (DynFun*)window_xwindow},
    {region_notify_rootpos, window_notify_subs_rootpos},
    {region_restack, window_restack},
    END_DYNFUNTAB
};


EXTL_EXPORT
IMPLCLASS(WWindow, WRegion, window_deinit, window_dynfuntab);

    
/*}}}*/

