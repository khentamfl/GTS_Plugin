Scriptname GtsSMConfigMenu extends MCM_ConfigBase

Property Actor PlayerRef Auto

Event OnConfigInit()
    GtsScale.SetScaleMethod(GetModSettingInt("iScaleMethod:Main"))
    GtsPlugin.EnableHighHeelCorrection(GetModSettingBool("bEnableHighHeels:Main"))
    GtsPlugin.SetGrowthHalfLife(PlayerRef, GetModSettingFloat("fPlayerGrowthHalfLife:Main"))
EndEvent
    
; Event raised when a config menu is opened.
Event OnConfigOpen()
    SetModSettingInt("iScaleMethod:Main", GtsScale.GetScaleMethod())
    SetModSettingBool("bEnableHighHeels:Main", GtsPlugin.GetHighHeelCorrection())
    SetModSettingFloat("fPlayerGrowthHalfLife:Main", GtsPlugin.GetGrowthHalfLife(PlayerRef))
EndEvent