scriptName GtsPlugin hidden

; Get Distance To Camera
;
; This will get the distance to the camera
Float function GetDistanceToCamera(Actor target) global native

; This is the time it takes to reach half of the target height
; By default this is 0.05s (instantaneous)
;
; Value is saved into the cosave
Bool function SetGrowthHalfLife(Actor target, Float halflife) global native
Float function GetGrowthHalfLife(Actor target) global native

; This sets the speed at which anims are played
; 1.0 is normal speed while 2.0 is twice as fast
;
; Value is saved into the cosave
Bool function SetAnimSpeed(Actor target, Float animspeed) global native

; Format a number to a string with specified significant figures
;
; Uses sprintf
;
; e.g.
; String formatted_number = SigFig(10.2323, 3)
; ; formatted_number should now be 10.2
String function SigFig(Float number, Int sf) global native

; Controls if the HH correction method is enabled or not
;
; Value is saved into the cosave
Function SetIsHighHeelEnabled(Bool enabled) global native
Bool Function GetIsHighHeelEnabled() global native

; Controls if the Anim and Walk speeds adjustments are enabled or not
;
; Value is saved into the cosave
Function SetIsSpeedAdjusted(Bool enabled) global native
Bool Function GetIsSpeedAdjusted() global native
; These control the variables in the speed adjustment formula
;
; The formula is
; 1/(1+(k*(x-1.0))^(n*s))^(1/s)
; https://www.desmos.com/calculator/klvqenjooi
; Values are saved into the cosave
Float Function GetSpeedParameterK() global native
Float Function SetSpeedParameterK(Float k) global native
Float Function GetSpeedParameterN() global native
Float Function SetSpeedParameterN(Float n) global native
Float Function GetSpeedParameterS() global native
Float Function SetSpeedParameterS(Float s) global native
