/*
 * ion/utils/ion-statusd/extlrx.c
 *
 * Copyright (c) Tuomo Valkonen 2004-2009.
 *
 * See the included file LICENSE for details.
 */

#include <libextl/extl.h>
#include <libtu/output.h>
#include <libtu/locale.h>


/*{{{ libtu */


/*EXTL_DOC
 * Issue a warning. How the message is displayed depends on the current
 * warning handler.
 */
EXTL_EXPORT
void statusd_warn(const char *str)
{
    warn("%s", str);
}


EXTL_EXPORT
const char *statusd_gettext(const char *s)
{
    if(s==NULL)
        return NULL;
    else
        return TR(s);
}


/*}}}*/

