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
