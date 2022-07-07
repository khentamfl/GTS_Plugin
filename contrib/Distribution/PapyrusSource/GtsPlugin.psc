scriptName GtsPlugin hidden

; Model scale
;
; These directly and immediatly set the scale
Bool function SetModelScale(Actor target, Float scale) global native

Float function GetModelScale(Actor target) global native

Bool function ModModelScale(Actor target, Float amount) global native


; Target Scale
;
; These will be the target scale which is achieved on the main loop over the
; next few frames gradually
; Use this to alter the current scale
;
; Target scale is saved into the COSAVE and will persist
function SetTargetScale(Actor target, Float scale) global native

Float function GetTargetScale(Actor target) global native

function ModTargetScale(Actor target, Float amount) global native

; Max Scale
;
; These will set the max scales
;
; Max scale is saved into the COSAVE and will persist
function SetMaxScale(Actor target, Float scale) global native

Float function GetMaxScale(Actor target) global native

function ModMaxScale(Actor target, Float amount) global native

; Visual Scale
;
; This is the current actual scale of the actor. While the target scale
; is what this value aims for, before it gets to the target scale
; then this represents the actual scale
;
; Use this for any size effects
;
; Plan is to have growth be stop by obstacles to the target scale
; like ceilings, and this will be the actual scale that is achieved.
;
; Visual scale is saved into the COSAVE and will persist
Float function GetVisualScale(Actor target) global native


; TeamMate scale mod
;
; This will mod the target scale of all teammates nearby
function ModTeammateScale(Float amount) global native

; Get Distance To Camera
;
; This will get the distance to the camera
Float function GetDistanceToCamera(Actor target) global native
