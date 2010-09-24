--
-- ion/share/ioncore_wd.lua
-- 
-- Copyright (c) Tuomo Valkonen 2004-2009.
--
-- See the included file LICENSE for details.
--

local savefile="saved_wd"
local dirs={}
local lfs

if pcall(function() return require('lfs') end) then
    lfs=_G["lfs"]
end

local function checkdir(d)
    if not lfs then
        return true
    else
        local t, err=lfs.attributes(d, "mode")
        if not t then
            return nil, err
        elseif t=="directory" then
            return true
        else
            return nil, TR("Not a directory.")
        end
    end
end

local function regtreepath_i(reg)
    local function f(s, v)
        if v then
            return v:manager()
        else
            return s
        end
    end
    return f, reg, nil
end

--DOC
-- Change default working directory for new programs started in \var{reg}.
function ioncore.chdir_for(reg, dir)
    assert(dir==nil or type(dir)=="string")
    if dir=="" or dir==nil then
        dirs[reg]=nil
        return true
    else 
        local ok, err=checkdir(dir)
        if ok then
            dirs[reg]=dir
        end
        return ok, err
    end
end

--DOC
-- Get default working directory for new programs started in \var{reg}.
function ioncore.get_dir_for(reg)
    for r in regtreepath_i(reg) do
        if dirs[r] then
            return dirs[r]
        end
    end
end


local function lookup_script_warn(script)
    local script=ioncore.lookup_script(script)
    if not script then
        warn(TR("Could not find %s", script))
    end
    return script
end


local function lookup_runinxterm_warn(prog, title, wait)
    local rx=lookup_script_warn("ion-runinxterm")
    if rx then
        rx="exec "..rx
        if wait then
            rx=rx.." -w"
        end
        if title then
            rx=rx.." -T "..string.shell_safe(title)
        end
        if prog then
            rx=rx.." -- "..prog
        end
    end
    return rx
end


--DOC
-- Run \var{cmd} with the environment variable DISPLAY set to point to the
-- root window of the X screen \var{reg} is on. If \var{cmd} is prefixed
-- by a colon (\code{:}), the following command is executed in an xterm
-- (or other terminal emulator) with the help of the \command{ion-runinxterm} 
-- script. If the command is prefixed by two colons, \command{ion-runinxterm}
-- will ask you to press enter after the command is finished, even if it
-- returns succesfully.
function ioncore.exec_on(reg, cmd, merr_internal)
    local _, _, col, c=string.find(cmd, "^[%s]*(:+)(.*)")
    if col then
        cmd=lookup_runinxterm_warn(c, nil, string.len(col)>1)
        if not cmd then
            return
        end
        if XTERM then
            cmd='XTERMCMD='..string.shell_safe(XTERM)..' '..cmd
        end
    end
    return ioncore.do_exec_on(reg, cmd, ioncore.get_dir_for(reg), 
                              merr_internal)
end


local function load_config()
    local d=ioncore.read_savefile(savefile)
    if d then
        dirs={}
        for nm, d in pairs(d) do
            local r=ioncore.lookup_region(nm)
            if r then
                local ok, err=checkdir(d)
                if ok then
                    dirs[r]=d
                else
                    warn(err)
                end
            end
        end
    end
end


local function save_config()
    local t={}
    for r, d in pairs(dirs) do
        local nm=obj_exists(r) and r:name()
        if nm then
            t[nm]=d
        end
    end
    ioncore.write_savefile(savefile, t)
end


local function init()
    load_config()
    ioncore.get_hook("ioncore_snapshot_hook"):add(save_config)
    ioncore.get_hook("ioncore_post_layout_setup_hook"):add(load_config)
end

init()

