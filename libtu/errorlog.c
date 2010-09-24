/*
 * libtu/errorlog.c
 *
 * Copyright (c) Tuomo Valkonen 1999-2004. 
 *
 * You may distribute and modify this library under the terms of either
 * the Clarified Artistic License or the GNU LGPL, version 2.1 or later.
 */

#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "util.h"
#include "types.h"
#include "output.h"
#include "misc.h"
#include "errorlog.h"

static ErrorLog *current_log=NULL;

static void add_to_log(ErrorLog *el, const char *message, int l)
{
    if(message==NULL)
        return;
    
    /* Also write to stderr */
    fwrite(message, sizeof(char), l, stderr);

    if(el==NULL)
        return;
    
    if(el->file!=NULL){
        el->errors=TRUE;
        fwrite(message, sizeof(char), l, el->file);
        return;
    }

    if(el->msgs==NULL){
        el->msgs=ALLOC_N(char, ERRORLOG_MAX_SIZE);
        if(el->msgs==NULL){
            fprintf(stderr, "%s: %s\n", libtu_progname(), strerror(errno));
            return;
        }
        el->msgs[0]=0;
        el->msgs_len=0;
    }
            
    el->errors=TRUE;
    
    if(l+el->msgs_len>ERRORLOG_MAX_SIZE-1){
        int n=0;
        if(l<ERRORLOG_MAX_SIZE-1){
            n=ERRORLOG_MAX_SIZE-1-l;
            memmove(el->msgs, el->msgs+el->msgs_len-n, n);
        }
        memcpy(el->msgs+n, message+l-(ERRORLOG_MAX_SIZE-1-n),
               ERRORLOG_MAX_SIZE-1-n);
        el->msgs[ERRORLOG_MAX_SIZE]='\0';
        el->msgs_len=ERRORLOG_MAX_SIZE-1;
    }else{
        memcpy(el->msgs+el->msgs_len, message, l);
        el->msgs[el->msgs_len+l]='\0';
        el->msgs_len+=l;
    }
}


static void log_warn_handler(const char *message)
{
    const char *p=strchr(message, '\n');
    static int lineno=0;
    int alternat=0;
    
    add_to_log(current_log, lineno==0 ? ">> " : "   ", 3);
    
    if(p!=NULL){
        add_to_log(current_log, message, p-message+1);
        lineno++;
        log_warn_handler(p+1);
        lineno--;
        return;
    }
    
    add_to_log(current_log, message, strlen(message));
    add_to_log(current_log, "\n", 1);
}

           
void errorlog_begin_file(ErrorLog *el, FILE *file)
{
    el->msgs=NULL;
    el->msgs_len=0;
    el->file=file;
    el->prev=current_log;
    el->errors=FALSE;
    el->old_handler=set_warn_handler(log_warn_handler);
    current_log=el;
}


void errorlog_begin(ErrorLog *el)
{
    errorlog_begin_file(el, NULL);
}


bool errorlog_end(ErrorLog *el)
{
    current_log=el->prev;
    set_warn_handler(el->old_handler);
    el->prev=NULL;
    el->old_handler=NULL;
    return el->errors;
}


void errorlog_deinit(ErrorLog *el)
{
    if(el->msgs!=NULL)
        free(el->msgs);
    el->msgs=NULL;
    el->msgs_len=0;
    el->file=NULL;
    el->errors=FALSE;
}

