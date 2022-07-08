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