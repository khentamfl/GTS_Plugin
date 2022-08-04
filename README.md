[![CI](https://github.com/QuantumEntangledAndy/GTS_Plugin/actions/workflows/build.yml/badge.svg)](https://github.com/QuantumEntangledAndy/GTS_Plugin/actions/workflows/build.yml)

# Size Matters SKSE64 Plugin

This is the source for the Gianttess mod Size matters SKSE plugin.

WIP

---

## Feature Wish List

- [ ] Auto scale height to room
- [ ] Correct bumper for height
- [ ] Talk to actor when crouching
- [ ] Accurate Body Collision that inflicts size effects on contact instead of using cloak magiceffects
- [ ] Proper headtracking
- [ ] Ability to edit HitBoxes so it'll be possible to have Giant Enemies/Have more accurate hitbox
  - AABB was shared between all actors of the same skeleton
  - Found a way to clone the AABB and make them unique to each actor
- [X] Ability to spawn dust effect explosions literally under the feet, not under center of character. Ideally even scale them.
- [X] Maybe proper High-Heel module that dynamically adjusts high heel size based on Root scale?
- [x] In the perfect scenario, repair Animation Speed based on size
- [ ] If possible, transfer from SkyrimPlatform to Pure DLL, so SP won't be needed
- [X] Make DLL track Giantess PC/NPC footsteps to shake the screen/place dust without lags. Currently it has a delay because of script latency.
- [ ] Ray-Tracked Vore instead of dialogue. Vore on button press if conditions are met.
- [ ] Blood on the feet after crushing (Not sure how to do it at all)
- [ ] Quest progression ui/current size ui
- [ ] Vore anim
- [ ] Vore belly

## Easier Things
- [ ] Scale
  - [x] Change scale in papyrus
  - [ ] Get height in meters of any actor
  - [ ] Get volume of any actor
- [ ] Mechanics  
  - [ ] Apply size effects to near by actor
  - [x] Move away from favor active
  - [x] Animation speed adjustment
- [ ] Walk/Jump events
  - [x] Camera distance to event
  - [ ] Camera shake
  - [ ] Feet sounds
