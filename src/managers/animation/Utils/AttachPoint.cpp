#include "managers/animation/Utils/AttachPoint.hpp"
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