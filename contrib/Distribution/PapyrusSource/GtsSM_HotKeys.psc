ScriptName GtsSM_HotKeys Extends Quest

Actor Property PlayerRef Auto

Function GrowPlayer()
  GtsPlugin.ModTargetScale(PlayerRef, 0.1)
EndFunction

Function ShrinkPlayer()
  GtsPlugin.ModTargetScale(PlayerRef, -0.1)
EndFunction


Function GrowFollowers()
  GtsPlugin.ModTeammateScale(0.1)
EndFunction

Function ShrinkFollowers()
  GtsPlugin.ModTeammateScale(-0.1)
EndFunction
