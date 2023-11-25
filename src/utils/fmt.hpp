#pragma once
#include <fmt/core.h>
#include <spdlog/fmt/ostr.h>

using namespace RE;

template <> struct fmt::formatter<BSFixedString> : formatter<string_view> {
  auto format(BSFixedString v, format_context& ctx) const {
    string_view name = "<empty>";
    if (v.c_str()) {
      name = v.c_str();
    }
    return formatter<string_view>::format(name, ctx);
  }
};


template <> struct fmt::formatter<ACTOR_VALUE_MODIFIER>: formatter<string_view> {
  auto format(ACTOR_VALUE_MODIFIER v, format_context& ctx) const {
    string_view name = "unknown";
    switch (v) {
      case ACTOR_VALUE_MODIFIER::kPermanent: name = "kPermanent"; break;
      case ACTOR_VALUE_MODIFIER::kTemporary: name = "kTemporary"; break;
      case ACTOR_VALUE_MODIFIER::kDamage: name = "kDamage"; break;
    }
    return formatter<string_view>::format(name, ctx);
  }
};


template <> struct fmt::formatter<COL_LAYER>: formatter<string_view> {
  auto format(COL_LAYER v, format_context& ctx) const {
    string_view name = "unknown";
    switch (v) {
      case COL_LAYER::kUnidentified: name = "kUnidentified"; break;
      case COL_LAYER::kStatic: name = "kStatic"; break;
      case COL_LAYER::kAnimStatic: name = "kAnimStatic"; break;
      case COL_LAYER::kTransparent: name = "kTransparent"; break;
      case COL_LAYER::kClutter: name = "kClutter"; break;
      case COL_LAYER::kWeapon: name = "kWeapon"; break;
      case COL_LAYER::kProjectile: name = "kProjectile"; break;
      case COL_LAYER::kSpell: name = "kSpell"; break;
      case COL_LAYER::kBiped: name = "kBiped"; break;
      case COL_LAYER::kTrees: name = "kTrees"; break;
      case COL_LAYER::kProps: name = "kProps"; break;
      case COL_LAYER::kWater: name = "kWater"; break;
      case COL_LAYER::kTrigger: name = "kTrigger"; break;
      case COL_LAYER::kTerrain: name = "kTerrain"; break;
      case COL_LAYER::kTrap: name = "kTrap"; break;
      case COL_LAYER::kNonCollidable: name = "kNonCollidable"; break;
      case COL_LAYER::kCloudTrap: name = "kCloudTrap"; break;
      case COL_LAYER::kGround: name = "kGround"; break;
      case COL_LAYER::kPortal: name = "kPortal"; break;
      case COL_LAYER::kDebrisSmall: name = "kDebrisSmall"; break;
      case COL_LAYER::kDebrisLarge: name = "kDebrisLarge"; break;
      case COL_LAYER::kAcousticSpace: name = "kAcousticSpace"; break;
      case COL_LAYER::kActorZone: name = "kActorZone"; break;
      case COL_LAYER::kProjectileZone: name = "kProjectileZone"; break;
      case COL_LAYER::kGasTrap: name = "kGasTrap"; break;
      case COL_LAYER::kShellCasting: name = "kShellCasting"; break;
      case COL_LAYER::kTransparentWall: name = "kTransparentWall"; break;
      case COL_LAYER::kInvisibleWall: name = "kInvisibleWall"; break;
      case COL_LAYER::kTransparentSmallAnim: name = "kTransparentSmallAnim"; break;
      case COL_LAYER::kClutterLarge: name = "kClutterLarge"; break;
      case COL_LAYER::kCharController: name = "kCharController"; break;
      case COL_LAYER::kStairHelper: name = "kStairHelper"; break;
      case COL_LAYER::kDeadBip: name = "kDeadBip"; break;
      case COL_LAYER::kBipedNoCC: name = "kBipedNoCC"; break;
      case COL_LAYER::kAvoidBox: name = "kAvoidBox"; break;
      case COL_LAYER::kCollisionBox: name = "kCollisionBox"; break;
      case COL_LAYER::kCameraSphere: name = "kCameraSphere"; break;
      case COL_LAYER::kDoorDetection: name = "kDoorDetection"; break;
      case COL_LAYER::kConeProjectile: name = "kConeProjectile"; break;
      case COL_LAYER::kCamera: name = "kCamera"; break;
      case COL_LAYER::kItemPicker: name = "kItemPicker"; break;
      case COL_LAYER::kLOS: name = "kLOS"; break;
      case COL_LAYER::kPathingPick: name = "kPathingPick"; break;
      case COL_LAYER::kUnused0: name = "kUnused0"; break;
      case COL_LAYER::kUnused1: name = "kUnused1"; break;
      case COL_LAYER::kSpellExplosion: name = "kSpellExplosion"; break;
      case COL_LAYER::kDroppingPick: name = "kDroppingPick"; break;
    }
    return formatter<string_view>::format(name, ctx);
  }
};
