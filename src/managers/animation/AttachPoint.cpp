#include "managers/animation/AttachPoint.hpp"

namespace Gts {
  void Test() {
    auto testActor = PlayerCharacter::GetSingleton();
    auto testHandle = testActor->CreateRefHandle();
    AttachToObjectA(testActor);
    AttachToObjectA(*testActor);
    AttachToObjectA(testHandle);
    AttachToObjectA(0x14);
  }
}
