/*
 * ion/ioncore/rootwin.c
 *
 * Copyright (c) Tuomo Valkonen 1999-2009. 
 *
 * See the included file LICENSE for details.
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>

#include <libtu/objp.h>
#include "common.h"
#include "rootwin.h"
#include "cursor.h"
#include "global.h"
#include "event.h"
#include "gr.h"
#include "clientwin.h"
#include "property.h"
#include "focus.h"
#include "regbind.h"
#include "screen.h"
#include "screen.h"
#include "bindmaps.h"
#include <libextl/readconfig.h>
#include "resize.h"
#include "saveload.h"
#include "netwm.h"
#include "xwindow.h"


/*{{{ Error handling */


static bool redirect_error=FALSE;
static bool ignore_badwindow=TRUE;


static int my_redirect_error_handler(Display *dpy, XErrorEvent *ev)
{
    redirect_error=TRUE;
    return 0;
}


static int my_error_handler(Display *dpy, XErrorEvent *ev)
{
    static char msg[128], request[64], num[32];
    
    /* Just ignore bad window and similar errors; makes the rest of
     * the code simpler.
     * 
     * Apparently XGetWindowProperty can return BadMatch on a race
     * condition where the server is already reusing the XID for a 
     * non-window drawable, so let's just ignore BadMatch entirely...
     */
    if((ev->error_code==BadWindow ||
        (ev->error_code==BadMatch /*&& ev->request_code==X_SetInputFocus*/) ||
        (ev->error_code==BadDrawable && ev->request_code==X_GetGeometry)) &&
       ignore_badwindow)
        return 0;

#if 0
    XmuPrintDefaultErrorMessage(dpy, ev, stderr);
#else
    XGetErrorText(dpy, ev->error_code, msg, 128);
    snprintf(num, 32, "%d", ev->request_code);
    XGetErrorDatabaseText(dpy, "XRequest", num, "", request, 64);

    if(request[0]=='\0')
        snprintf(request, 64, "<unknown request>");

    if(ev->minor_code!=0){
        warn("[%d] %s (%d.%d) %#lx: %s", ev->serial, request,
             ev->request_code, ev->minor_code, ev->resourceid,msg);
    }else{
        warn("[%d] %s (%d) %#lx: %s", ev->serial, request,
             ev->request_code, ev->resourceid,msg);
    }
#endif

    kill(getpid(), SIGTRAP);
    
    return 0;
}


/*}}}*/


/*{{{ Init/deinit */


static void scan_initial_windows(WRootWin *rootwin)
{
    Window dummy_root, dummy_parent, *wins=NULL;
    uint nwins=0, i, j;
    XWMHints *hints;
    
    XQueryTree(ioncore_g.dpy, WROOTWIN_ROOT(rootwin), &dummy_root, &dummy_parent,
               &wins, &nwins);
    
    for(i=0; i<nwins; i++){
        if(wins[i]==None)
            continue;
        hints=XGetWMHints(ioncore_g.dpy, wins[i]);
        if(hints!=NULL && hints->flags&IconWindowHint){
            for(j=0; j<nwins; j++){
                if(wins[j]==hints->icon_window){
                    wins[j]=None;
                    break;
                }
            }
        }
        if(hints!=NULL)
            XFree((void*)hints);
    }
    
    rootwin->tmpwins=wins;
    rootwin->tmpnwins=nwins;
}


void rootwin_manage_initial_windows(WRootWin *rootwin)
{
    Window *wins=rootwin->tmpwins;
    Window tfor=None;
    int i, nwins=rootwin->tmpnwins;

    rootwin->tmpwins=NULL;
    rootwin->tmpnwins=0;
    
    for(i=0; i<nwins; i++){
        if(XWINDOW_REGION_OF(wins[i])!=NULL)
            wins[i]=None;
        if(wins[i]==None)
            continue;
        if(XGetTransientForHint(ioncore_g.dpy, wins[i], &tfor))
            continue;
        ioncore_manage_clientwin(wins[i], FALSE);
        wins[i]=None;
    }

    for(i=0; i<nwins; i++){
        if(wins[i]==None)
            continue;
        ioncore_manage_clientwin(wins[i], FALSE);
    }
    
    XFree((void*)wins);
}


static void create_wm_windows(WRootWin *rootwin)
{
    rootwin->dummy_win=XCreateWindow(ioncore_g.dpy, WROOTWIN_ROOT(rootwin),
                                     0, 0, 1, 1, 0,
                                     CopyFromParent, InputOnly,
                                     CopyFromParent, 0, NULL);

    XSelectInput(ioncore_g.dpy, rootwin->dummy_win, PropertyChangeMask);
}


static void preinit_gr(WRootWin *rootwin)
{
    XGCValues gcv;
    ulong gcvmask;

    /* Create XOR gc (for resize) */
    gcv.line_style=LineSolid;
    gcv.join_style=JoinBevel;
    gcv.cap_style=CapButt;
    gcv.fill_style=FillSolid;
    gcv.line_width=2;
    gcv.subwindow_mode=IncludeInferiors;
    gcv.function=GXxor;
    gcv.foreground=~0L;
    
    gcvmask=(GCLineStyle|GCLineWidth|GCFillStyle|
             GCJoinStyle|GCCapStyle|GCFunction|
             GCSubwindowMode|GCForeground);

    rootwin->xor_gc=XCreateGC(ioncore_g.dpy, WROOTWIN_ROOT(rootwin), 
                              gcvmask, &gcv);
}


static Atom net_virtual_roots=None;


static bool rootwin_init(WRootWin *rootwin, int xscr)
{
    Display *dpy=ioncore_g.dpy;
    WFitParams fp;
    Window root;
    
    /* Try to select input on the root window */
    root=RootWindow(dpy, xscr);
    
    redirect_error=FALSE;

    XSetErrorHandler(my_redirect_error_handler);
    XSelectInput(dpy, root, IONCORE_EVENTMASK_ROOT|IONCORE_EVENTMASK_SCREEN);
    XSync(dpy, 0);
    XSetErrorHandler(my_error_handler);

    if(redirect_error){
        warn(TR("Unable to redirect root window events for screen %d."),
             xscr);
        return FALSE;
    }
    
    rootwin->xscr=xscr;
    rootwin->default_cmap=DefaultColormap(dpy, xscr);
    rootwin->tmpwins=NULL;
    rootwin->tmpnwins=0;
    rootwin->dummy_win=None;
    rootwin->xor_gc=None;

    fp.mode=REGION_FIT_EXACT;
    fp.g.x=0; fp.g.y=0;
    fp.g.w=DisplayWidth(dpy, xscr);
    fp.g.h=DisplayHeight(dpy, xscr);
    
    if(!screen_init((WScreen*)rootwin, NULL, &fp, xscr, root)){
        free(rootwin);
        return FALSE;
    }

    ((WWindow*)rootwin)->event_mask=IONCORE_EVENTMASK_ROOT|IONCORE_EVENTMASK_SCREEN;
    ((WRegion*)rootwin)->flags|=REGION_BINDINGS_ARE_GRABBED|REGION_PLEASE_WARP;
    ((WRegion*)rootwin)->rootwin=rootwin;
    
    REGION_MARK_MAPPED(rootwin);
    
    scan_initial_windows(rootwin);

    create_wm_windows(rootwin);
    preinit_gr(rootwin);
    netwm_init_rootwin(rootwin);
    
    net_virtual_roots=XInternAtom(ioncore_g.dpy, "_NET_VIRTUAL_ROOTS", False);
    XDeleteProperty(ioncore_g.dpy, root, net_virtual_roots);

    LINK_ITEM(*(WRegion**)&ioncore_g.rootwins, (WRegion*)rootwin, p_next, p_prev);

    xwindow_set_cursor(root, IONCORE_CURSOR_DEFAULT);
    
    return TRUE;
}


WRootWin *create_rootwin(int xscr)
{
    CREATEOBJ_IMPL(WRootWin, rootwin, (p, xscr));
}


void rootwin_deinit(WRootWin *rw)
{
    WScreen *scr, *next;

    FOR_ALL_SCREENS_W_NEXT(scr, next){
        if(REGION_MANAGER(scr)==(WRegion*)rw)
            destroy_obj((Obj*)scr);
    }
    
    UNLINK_ITEM(*(WRegion**)&ioncore_g.rootwins, (WRegion*)rw, p_next, p_prev);
    
    XSelectInput(ioncore_g.dpy, WROOTWIN_ROOT(rw), 0);
    
    XFreeGC(ioncore_g.dpy, rw->xor_gc);
    
    rw->scr.mplex.win.win=None;

    screen_deinit(&rw->scr);
}


/*}}}*/


/*{{{ region dynfun implementations */


static bool rootwin_fitrep(WRootWin *rootwin, WWindow *par, 
                           const WFitParams *fp)
{
    D(warn("Don't know how to reparent or fit root windows."));
    return FALSE;
}


static void rootwin_map(WRootWin *rootwin)
{
    D(warn("Attempt to map a root window."));
}


static void rootwin_unmap(WRootWin *rootwin)
{
    D(warn("Attempt to unmap a root window -- impossible."));
}


/*}}}*/


/*{{{ Misc */


/*EXTL_DOC
 * Returns previously active screen on root window \var{rootwin}.
 */
EXTL_SAFE
EXTL_EXPORT_MEMBER
WScreen *rootwin_current_scr(WRootWin *rootwin)
{
    WScreen *scr, *fb=NULL;
    
    FOR_ALL_SCREENS(scr){
        if(REGION_MANAGER(scr)==(WRegion*)rootwin && REGION_IS_MAPPED(scr)){
            fb=scr;
            if(REGION_IS_ACTIVE(scr))
                return scr;
        }
    }
    
    return (fb ? fb : &rootwin->scr);
}


/*}}}*/


/*{{{ Dynamic function table and class implementation */


static DynFunTab rootwin_dynfuntab[]={
    {region_map, rootwin_map},
    {region_unmap, rootwin_unmap},
    {(DynFun*)region_fitrep, (DynFun*)rootwin_fitrep},
    END_DYNFUNTAB
};


EXTL_EXPORT
IMPLCLASS(WRootWin, WScreen, rootwin_deinit, rootwin_dynfuntab);

    
/*}}}*/
