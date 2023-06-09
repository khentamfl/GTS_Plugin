#include "managers/animation/AttachPoint.hpp"
#include "scale/scale.hpp"

using namespace RE;
using namespace Gts;

namespace {
  void TestActorAttaches() {
    Actor* testActorPtr = PlayerCharacter::GetSingleton();
    Actor& testActorRef = *testActorPtr;
    ActorHandle testHandle = testActorPtr->CreateRefHandle();
    FormID testFormID = 0x14;

    log::info("Testing Attaches");

    AttachToObjectA(testActorPtr, testActorPtr);
    AttachToObjectA(testActorRef, testActorPtr);
    AttachToObjectA(testHandle, testActorPtr);
    AttachToObjectA(testFormID, testActorPtr);

    log::info("Complete");
  }
}

namespace Gts {
   NiPoint3 GetBreastPos(Actor* giant, std::string_view bone_name) {
      NiPoint3 pos = NiPoint3();
		  if (giant) {
        auto scale = get_visual_scale(giant);
        auto rootModel = giant->Get3D(false);
        if (rootModel) {
          auto playerTrans = rootModel->world;
          playerTrans.scale = rootModel->parent ? rootModel->parent->world.scale : 1.0;  // Only do translation/rotation
          auto transform = playerTrans.Invert();
          auto node = find_node(giant, bone_name);
          NiPoint3 bonePos = NiPoint3(); 
          if (node) {
            auto localPos = transform * (node->world * NiPoint3());
            bonePos += localPos;
          }
          log::info("Bone {} Pos: {}", bone_name, Vector2Str(bonePos));
          return bonePos;
      }
    }
  }

  Actor* GetActorPtr(Actor* actor) {
    return actor;
  }

  Actor* GetActorPtr(Actor& actor) {
    return &actor;
  }

  Actor* GetActorPtr(ActorHandle& actor) {
    if (!actor) {
      return nullptr;
    }
    return actor.get().get();
  }
  Actor* GetActorPtr(const ActorHandle& actor) {
    if (!actor) {
      return nullptr;
    }
    return actor.get().get();
  }
  Actor* GetActorPtr(FormID formId) {
    Actor* actor = TESForm::LookupByID<Actor>(formId);
    if (!actor) {
      return nullptr;
    }
    return actor;
  }
}
