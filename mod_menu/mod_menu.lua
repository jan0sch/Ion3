--
-- ion/mod_menu/mod_menu.lua -- Menu opening helper routines.
-- 
-- Copyright (c) Tuomo Valkonen 2004-2009.
--
-- See the included file LICENSE for details.
--

-- This is a slight abuse of the package.loaded variable perhaps, but
-- library-like packages should handle checking if they're loaded instead of
-- confusing the user with require/include differences.
if package.loaded["mod_menu"] then return end

if not ioncore.load_module("mod_menu") then
    return
end

local mod_menu=_G["mod_menu"]
local menudb=_G["ioncore"]

assert(mod_menu and menudb)


-- Menu commands {{{

local function menu_(reg, sub, menu_or_name, fn, check)
    if check then
        -- Check that no other menus are open in reg.
        local ok=reg:managed_i(function(r) 
                                   return not obj_is(r, "WMenu") 
                               end)
        if not ok then
            return
        end
    end

    menu=menudb.evalmenu(menu_or_name, reg, sub)
    
    return fn(reg, function(e) e.func(reg, sub) end, menu)
end


--DOC
-- Display a menu in the lower-left corner of \var{mplex}.
-- The variable \var{menu_or_name} is either the name of a menu
-- defined with \fnref{mod_menu.defmenu} or directly a table similar
-- to ones passesd to this function. When this function is
-- called from a binding handler, \var{sub} should be set to
-- the second argument of to the binding handler (\var{_sub})
-- so that the menu handler will get the same parameters as the
-- binding handler. Extra options can be passed in the table
-- \var{param}. The initial entry can be specified as the field
-- \var{initial} as an integer starting from 1. Menus can be made
-- to use a bigger style by setting the field \var{big} to \code{true}.
function mod_menu.menu(mplex, sub, menu_or_name, param) 
   local function menu_stdmenu(m, s, menu)
        return ioncore.unsqueeze(mod_menu.do_menu(m, s, menu, param))
   end
   return menu_(mplex, sub, menu_or_name, menu_stdmenu, true)
end

-- Compatibility
function mod_menu.bigmenu(mplex, sub, menu_or_name, initial) 
    local param={big=true, initial=initial}
    return mod_menu.menu(mplex, sub, menu_or_name, param)
end

--DOC
-- This function is similar to \fnref{mod_menu.menu}, but input
-- is grabbed and the key used to active the menu can be used to
-- cycle through menu entries.
function mod_menu.grabmenu(mplex, sub, menu_or_name, param) 
    local function menu_grabmenu(m, s, menu)
        return mod_menu.do_grabmenu(m, s, menu, param)
    end
    return menu_(mplex, sub, menu_or_name, menu_grabmenu, true)
end

-- Compatibility
function mod_menu.biggrabmenu(mplex, sub, menu_or_name, key, initial) 
    local function menu_biggrabmenu(m, s, menu)
        return mod_menu.do_grabmenu(m, s, menu, true, key, initial or 0)
    end
    return menu_(mplex, sub, menu_or_name, menu_biggrabmenu, true, initial)
end

--DOC
-- This function displays a drop-down menu and should only
-- be called from a mouse press handler. The parameters are
-- similar to those of \fnref{mod_menu.menu}.
function mod_menu.pmenu(win, sub, menu_or_name) 
    return menu_(win, sub, menu_or_name, mod_menu.do_pmenu)
end

-- }}}


-- Mark ourselves loaded.
package.loaded["mod_menu"]=true


-- Load configuration file
dopath('cfg_menu', true)
