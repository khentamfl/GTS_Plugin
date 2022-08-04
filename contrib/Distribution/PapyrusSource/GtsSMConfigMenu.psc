Scriptname GtsSMConfigMenu extends MCM_ConfigBase

Actor Property PlayerRef Auto

Event OnConfigInit()
    PlayerRef = Game.GetPlayer()
    GtsScale.SetScaleMethod(GetModSettingInt("iScaleMethod:Main"))
    GtsPlugin.SetIsHighHeelEnabled(GetModSettingBool("bEnableHighHeels:Main"))
    GtsPlugin.SetTremorScale(GetModSettingFloat("fTremorScale:Main"))
    GtsPlugin.SetTremorScaleNPC(GetModSettingFloat("fTremorScaleNPC:Main"))

    GtsPlugin.SetGrowthHalfLife(PlayerRef, GetModSettingFloat("fPlayerGrowthHalfLife:Main"))
    GtsPlugin.SetIsHighHeelEnabled(GetModSettingBool("bEnableSpeedAdjustment:Main"))
    GtsPlugin.SetExperimentFloat(GetModSettingFloat("fExperiment:Main"))
EndEvent

; Event raised when a config menu is opened.
Event OnConfigOpen()
    SetModSettingInt("iScaleMethod:Main", GtsScale.GetScaleMethod())
    SetModSettingBool("bEnableHighHeels:Main", GtsPlugin.GetIsHighHeelEnabled())
    SetModSettingFloat("fTremorScale:Main", GtsPlugin.GetTremorScale())
    SetModSettingFloat("fTremorScaleNPC:Main", GtsPlugin.GetTremorScaleNPC())

    SetModSettingFloat("fPlayerGrowthHalfLife:Main", GtsPlugin.GetGrowthHalfLife(PlayerRef))
    SetModSettingBool("bEnableSpeedAdjustment:Main", GtsPlugin.GetIsSpeedAdjusted())
    SetModSettingFloat("fExperiment:Main", GtsPlugin.GetExperimentFloat())
EndEvent
