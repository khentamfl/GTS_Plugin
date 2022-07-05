ScriptName GtsSM_HotKeys Extends Quest

Actor Property PlayerRef Auto

Function GrowPlayer()
  float current_scale = GtsPlugin.GetModelScale(PlayerRef)
  GtsPlugin.SetModelScale(PlayerRef, current_scale + 0.1)
EndFunction

Function ShrinkPlayer()
  float current_scale = GtsPlugin.GetModelScale(PlayerRef)
  GtsPlugin.SetModelScale(PlayerRef, current_scale - 0.1)
EndFunction


Function GrowFollowers()
  float current_scale = GtsPlugin.GetTestScale()
  GtsPlugin.SetTestScale(current_scale + 0.1)
EndFunction

Function ShrinkFollowers()
  float current_scale = GtsPlugin.GetTestScale()
  GtsPlugin.SetTestScale(current_scale - 0.1)
EndFunction
