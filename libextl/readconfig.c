/*
 * libextl/readconfig.c
 *
 * Copyright (c) Tuomo Valkonen 1999-2005.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 */

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "readconfig.h"
#include "extl.h"
#include "private.h"


typedef struct{
    ExtlFn fn;
    ExtlTab tab;
    int status;
} TryCallParam;


/*{{{ Path setup */


static char *userdir=NULL;
static char *sessiondir=NULL;
static char *scriptpath=NULL;


bool extl_add_searchdir(const char *dir)
{
    if(scriptpath==NULL){
        scriptpath=extl_scopy(dir);
        if(scriptpath==NULL)
            return FALSE;
    }else{
        char *p=extl_scat3(dir, ":", scriptpath);
        if(p==NULL)
            return FALSE;
        free(scriptpath);
        scriptpath=p;
    }
    
    return TRUE;
}


bool extl_set_searchpath(const char *path)
{
    char *s=NULL;
    
    if(path!=NULL){
        s=extl_scopy(path);
        if(s==NULL)
            return FALSE;
    }
    
    if(scriptpath!=NULL)
        free(scriptpath);
    
    scriptpath=s;
    return TRUE;
}


bool extl_set_userdirs(const char *appname)
{
    const char *home;
    char *tmp;
    int fails=2;
    
    if(userdir!=NULL)
        return FALSE;
    
    home=getenv("HOME");
    
    if(home==NULL){
        extl_warn(TR("$HOME not set"));
    }else{
        libtu_asprintf(&userdir, "%s/.%s", home, appname);
        if(userdir!=NULL)
            fails-=extl_add_searchdir(userdir);
        
        libtu_asprintf(&tmp, "%s/.%s/lib", home, appname);
        if(tmp!=NULL){
            fails-=extl_add_searchdir(tmp);
            free(tmp);
        }
    }
    
    return (fails==0);
}


bool extl_set_sessiondir(const char *session)
{
    char *tmp;
    bool ret=FALSE;
    
    if(strchr(session, '/')!=NULL){
        tmp=extl_scopy(session);
    }else if(userdir!=NULL){
        libtu_asprintf(&tmp, "%s/%s", userdir, session);
    }else{
        extl_warn(TR("User directory not set. "
                     "Unable to set session directory."));
        return FALSE;
    }
    
    if(tmp==NULL)
        return FALSE;
    
    if(sessiondir!=NULL)
        free(sessiondir);
    
    sessiondir=tmp;
    
    return TRUE;
}


const char *extl_userdir()
{
    return userdir;
}


const char *extl_sessiondir()
{
    return sessiondir;
}


const char *extl_searchpath()
{
    return scriptpath;
}


/*}}}*/


/*{{{ try_etcpath, do_include, etc. */


static int do_try(const char *dir, const char *file, ExtlTryConfigFn *tryfn,
                  void *tryfnparam)
{
    char *tmp=NULL;
    int ret;
    
    libtu_asprintf(&tmp, "%s/%s", dir, file);
    if(tmp==NULL)
        return EXTL_TRYCONFIG_MEMERROR;

    ret=tryfn(tmp, tryfnparam);
    free(tmp);
    return ret;
}


static int try_dir(const char *const *files, const char *cfdir,
                   ExtlTryConfigFn *tryfn, void *tryfnparam)
{
    const char *const *file;
    int ret, ret2=EXTL_TRYCONFIG_NOTFOUND;
    
    for(file=files; *file!=NULL; file++){
        if(cfdir==NULL)
            ret=tryfn(*file, tryfnparam);
        else
            ret=do_try(cfdir, *file, tryfn, tryfnparam);
        if(ret>=0)
            return ret;
        if(ret2==EXTL_TRYCONFIG_NOTFOUND)
            ret2=ret;
    }
    
    return ret2;
}


static int try_etcpath(const char *const *files, 
                       ExtlTryConfigFn *tryfn, void *tryfnparam)
{
    const char *const *file=NULL;
    int i, ret, ret2=EXTL_TRYCONFIG_NOTFOUND;
    char *path, *colon, *dir;

    if(sessiondir!=NULL){
        for(file=files; *file!=NULL; file++){
            ret=do_try(sessiondir, *file, tryfn, tryfnparam);
            if(ret>=0)
                return ret;
            if(ret2==EXTL_TRYCONFIG_NOTFOUND)
                ret2=ret;
        }
    }
    
    path=scriptpath;
    while(path!=NULL){
        colon=strchr(path, ':');
        if(colon!=NULL){
            dir=extl_scopyn(path, colon-path);
            path=colon+1;
        }else{
            dir=extl_scopy(path);
            path=NULL;
        }
        
        if(dir!=NULL){
            if(*dir!='\0'){
                for(file=files; *file!=NULL; file++){
                    ret=do_try(dir, *file, tryfn, tryfnparam);
                    if(ret>=0){
                        free(dir);
                        return ret;
                    }
                    if(ret2==EXTL_TRYCONFIG_NOTFOUND)
                        ret2=ret;
                }
            }
            free(dir);
        }
    }
    
    return ret2;
}


static int try_lookup(const char *file, char **ptr)
{
    if(access(file, F_OK)!=0)
        return EXTL_TRYCONFIG_NOTFOUND;
    *ptr=extl_scopy(file);
    return (*ptr!=NULL);
}


static int try_load(const char *file, TryCallParam *param)
{
    if(access(file, F_OK)!=0)
        return EXTL_TRYCONFIG_NOTFOUND;
    
    if(param->status==1)
        extl_warn(TR("Falling back to %s."), file);
    
    if(!extl_loadfile(file, &(param->fn))){
        param->status=1;
        return EXTL_TRYCONFIG_LOAD_FAILED;
    }
    
    return EXTL_TRYCONFIG_OK;
}


static int try_call(const char *file, TryCallParam *param)
{
    int ret=try_load(file, param);
    
    if(ret!=EXTL_TRYCONFIG_OK)
        return ret;
    
    ret=extl_call(param->fn, NULL, NULL);
    
    extl_unref_fn(param->fn);
    
    return (ret ? EXTL_TRYCONFIG_OK : EXTL_TRYCONFIG_CALL_FAILED);
}


static int try_read_savefile(const char *file, TryCallParam *param)
{
    int ret=try_load(file, param);
    
    if(ret!=EXTL_TRYCONFIG_OK)
        return ret;
    
    ret=extl_call(param->fn, NULL, "t", &(param->tab));
    
    extl_unref_fn(param->fn);
    
    return (ret ? EXTL_TRYCONFIG_OK : EXTL_TRYCONFIG_CALL_FAILED);
}


int extl_try_config(const char *fname, const char *cfdir,
                    ExtlTryConfigFn *tryfn, void *tryfnparam,
                    const char *ext1, const char *ext2)
{
    char *files[]={NULL, NULL, NULL, NULL};
    int n=0, ret=EXTL_TRYCONFIG_NOTFOUND, ret2;
    bool search=FALSE, has_ext;
    
    /* Search etcpath only if path is not absolute */
    search=(fname[0]!='/');

    /* Build list of files to look for */
    has_ext=strrchr(fname, '.')>strrchr(fname, '/');

    if(!has_ext){
        if(ext1!=NULL){
            files[n]=extl_scat3(fname, ".", ext1);
            if(files[n]!=NULL)
                n++;
        }

        if(ext2!=NULL){
            files[n]=extl_scat3(fname, ".", ext2);
            if(files[n]!=NULL)
                n++;
        }
    }
    
    if(has_ext || !search){
        files[n]=extl_scopy(fname);
        if(files[n]!=NULL)
            n++;
    }
    
    /* NOTE for future changes: cfdir must not be scanned first for
     * user configuration files to take precedence.
     */

    /* Scan through all possible files */
    if(search){
        ret2=try_etcpath((const char**)&files, tryfn, tryfnparam);
        if(ret==EXTL_TRYCONFIG_NOTFOUND)
            ret=ret2;
        if(ret<0)
            ret=try_dir((const char**)&files, cfdir, tryfn, tryfnparam);
    }else{
        ret=try_dir((const char**)&files, NULL, tryfn, tryfnparam);
    }
    
    while(n>0)
        free(files[--n]);
    
    return ret;
}


/*EXTL_DOC
 * Lookup script \var{file}. If \var{try_in_dir} is set, it is tried
 * before the standard search path.
 */
EXTL_EXPORT
char *extl_lookup_script(const char *file, const char *sp)
{
    const char *files[]={NULL, NULL};
    char* tmp=NULL;
    
    if(file!=NULL){
        files[0]=file;

        if(sp!=NULL)
            try_dir(files, sp, (ExtlTryConfigFn*)try_lookup, &tmp);
        if(tmp==NULL)
            try_etcpath(files, (ExtlTryConfigFn*)try_lookup, &tmp);
    }
    
    return tmp;
}


bool extl_read_config(const char *file, const char *sp, bool warn_nx)
{
    TryCallParam param;
    int retval;
    
    if(file==NULL)
        return FALSE;
    
    param.status=0;
    
    retval=extl_try_config(file, sp, (ExtlTryConfigFn*)try_call, &param,
                              EXTL_COMPILED_EXTENSION, EXTL_EXTENSION);
    
    if(retval==EXTL_TRYCONFIG_NOTFOUND && warn_nx)
        extl_warn(TR("Unable to find '%s' on search path."), file);

    return (retval==EXTL_TRYCONFIG_OK);
}


bool extl_read_savefile(const char *basename, ExtlTab *tabret)
{
    TryCallParam param;
    int retval;
    
    param.status=0;
    param.tab=extl_table_none();
    
    retval=extl_try_config(basename, NULL, (ExtlTryConfigFn*)try_read_savefile,
                              &param, EXTL_EXTENSION, NULL);

    *tabret=param.tab;
    
    return (retval==EXTL_TRYCONFIG_OK);
}


/*EXTL_DOC
 * Read a savefile.
 */
EXTL_EXPORT_AS(extl, read_savefile)
ExtlTab extl_extl_read_savefile(const char *basename)
{
    ExtlTab tab;
    if(!extl_read_savefile(basename, &tab))
        return extl_table_none();
    return tab;
}
    

/*}}}*/


/*{{{�extl_get_savefile */


static bool ensuredir(char *f)
{
    char *p;
    int tryno=0;
    bool ret=TRUE;
    
    if(access(f, F_OK)==0)
        return TRUE;
    
    if(mkdir(f, 0700)==0)
        return TRUE;
    
    p=strrchr(f, '/');
    if(p==NULL){
        extl_warn_err_obj(f);
        return FALSE;
    }
    
    *p='\0';
    if(!ensuredir(f))
        return FALSE;
    *p='/';
        
    if(mkdir(f, 0700)==0)
        return TRUE;
    
    extl_warn_err_obj(f);
    return FALSE;
}


/*EXTL_DOC
 * Get a file name to save (session) data in. The string \var{basename} 
 * should contain no path or extension components.
 */
EXTL_EXPORT
char *extl_get_savefile(const char *basename)
{
    char *res=NULL;
    
    if(sessiondir==NULL)
        return NULL;
    
    if(!ensuredir(sessiondir)){
        extl_warn(TR("Unable to create session directory \"%s\"."), 
                  sessiondir);
        return NULL;
    }
    
    libtu_asprintf(&res, "%s/%s." EXTL_EXTENSION, sessiondir, basename);
    
    return res;
}


/*EXTL_DOC
 * Write \var{tab} in file with basename \var{basename} in the
 * session directory.
 */
EXTL_EXPORT
bool extl_write_savefile(const char *basename, ExtlTab tab)
{
    bool ret=FALSE;
    char *fname=extl_get_savefile(basename);
    
    if(fname!=NULL){
        ret=extl_serialise(fname, tab);
        free(fname);
    }
    
    return ret;
}


/*}}}*/

