[![CI](https://github.com/QuantumEntangledAndy/GTS_Plugin/actions/workflows/build.yml/badge.svg)](https://github.com/QuantumEntangledAndy/GTS_Plugin/actions/workflows/build.yml)

# Size Matters SKSE64 Plugin

This is the source for the Gianttess mod Size matters SKSE plugin.

WIP

---

## Feature Wish List

- [ ] [1] Auto scale height to room
- [ ] [2] Correct bumper for height
- [ ] [3] Talk to actor when crouching
- [ ] [4] Accurate Body Collision that inflicts size effects on contact instead of using cloak magiceffects
- [ ] [5] Proper headtracking
- [ ] [6] Ability to edit HitBoxes so it'll be possible to have Giant Enemies/Have more accurate hitbox
  - AABB was shared between all actors of the same skeleton
  - Found a way to clone the AABB and make them unique to each actor
- [X] [7] Ability to spawn dust effect explosions literally under the feet, not under center of character. Ideally even scale them.
- [X] [8] Maybe proper High-Heel module that dynamically adjusts high heel size based on Root scale?
- [x] [9] In the perfect scenario, repair Animation Speed based on size
- [ ] [10] If possible, transfer from SkyrimPlatform to Pure DLL, so SP won't be needed
- [ ] [11] Disable swimming when huge enough: make player sink instead of swimming when huge enough or based on size
- [X] [12] Make DLL track Giantess PC/NPC footsteps to shake the screen/place dust without lags. Currently it has a delay because of script latency.
- [ ] [13] Ray-Cast Vore instead of dialogue. Vore on button press if conditions are met.
- [ ] [14] Blood on the feet after crushing (Not sure how to do it at all, perhaps emulating weapon hit on the feet may do?)
- [ ] [15] Quest progression ui/current size ui
- [ ] [16] Vore anim
- [ ] [17] Vore belly

## Easier Things
- [ ] Scale
  - [x] Change scale in papyrus
  - [ ] Get height in meters of any actor
  - [ ] Get volume of any actor
- [ ] Mechanics  
  - [ ] Apply size effects to near by actor
  - [x] Move away from favor active
  - [x] Animation speed adjustment
- [X] Walk/Jump events
  - [x] Camera distance to event
  - [X] Camera shake
  - [X] Feet sounds
