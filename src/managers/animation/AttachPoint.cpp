#include "managers/animation/AttachPoint.hpp"

namespace {
  void Test() {
    Actor* testActorPtr = PlayerCharacter::GetSingleton();
    Actor& testActorRef = *testActor;
    ActorHandle testHandle = testActor->CreateRefHandle();
    FormID testFormID = 0x14;

    log::info("Testing Attaches");

    AttachToObjectA(testActorPtr, testActorPtr);
    AttachToObjectA(testActorRef, testActorPtr);
    AttachToObjectA(testActorRef, testActorPtr);
    AttachToObjectA(testFormID, testActorPtr);

    log::info("Complete");
  }
}
