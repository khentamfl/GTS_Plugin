#include "managers/animation/AttachPoint.hpp"

using namespace RE;
using namespace Gts;

namespace {
  void TestActorAttaches() {
    Actor* testActorPtr = PlayerCharacter::GetSingleton();
    Actor& testActorRef = *testActorPtr;
    ActorHandle testHandle = testActorPtr->CreateRefHandle();
    FormID testFormID = 0x14;

    log::info("Testing Attaches");

    // AttachToObjectA(testActorPtr, testActorPtr);
    // AttachToObjectA(testActorRef, testActorPtr);
    // AttachToObjectA(testHandle, testActorPtr);
    // AttachToObjectA(testFormID, testActorPtr);

    log::info("Complete");
  }
}
