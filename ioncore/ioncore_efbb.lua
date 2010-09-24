--
-- ion/share/ioncore_efbb.lua -- Minimal emergency fallback bindings.
-- 
-- Copyright (c) Tuomo Valkonen 2004-2009.
--
-- See the included file LICENSE for details.
--

warn(TR([[
Making the following minimal emergency mappings:
  F2 -> xterm
  F11 -> restart
  F12 -> exit
  Mod1+C -> close
  Mod1+K P/N -> WFrame.switch_next/switch_prev
]]))


defbindings("WScreen", {
    kpress("F2", function() ioncore.exec('xterm')  end),
    kpress("F11", function() ioncore.restart() end),
    kpress("F12", function() ioncore.exit() end),
})

defbindings("WMPlex", {
    kpress_wait("Mod1+C", WRegion.rqclose_propagate),
})

defbindings("WFrame", {
    submap("Mod1+K", {
        kpress("AnyModifier+N", function(f) f:switch_next() end),
        kpress("AnyModifier+P", function(f) f:switch_prev() end),
    })
})
