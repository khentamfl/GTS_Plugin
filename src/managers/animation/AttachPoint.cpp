#include "managers/animation/AttachPoint.hpp"

namespace Gts {
  void Test() {
    auto testActor = PlayerCharacter::GetSingleton();
    auto testHandle = testActor->CreateRefHandle();
    AttachToObjectA(testActor, testActor);
    AttachToObjectA(*testActor, testActor);
    AttachToObjectA(testHandle, testActor);
    AttachToObjectA(0x14, testActor);
  }
}
