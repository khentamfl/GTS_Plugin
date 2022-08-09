scriptName GtsCamera hidden

; Disable/Enable various camera collisions
;
; Values are saved into the cosave
Function SetEnableCollisionActor(Bool enabled) global native
Bool Function GetEnableCollisionActor() global native
Function SetEnableCollisionTree(bool enabled) global native
Bool Function GetEnableCollisionTree() global native
Function SetEnableCollisionDebris(bool enabled) global native
Bool Function GetEnableCollisionDebris() global native
Function SetEnableCollisionTerrain(bool enabled) global native
Bool Function GetEnableCollisionTerrain() global native

; Get/Set the minimum scale below which camera collisions acts like default
; and above which the camera adheres to the settings above
;
; Value is stored in the cosave
Function SetCollisionScale(float scale) global native
Float Function GetCollisionScale() global native
