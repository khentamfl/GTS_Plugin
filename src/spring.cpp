#include "spring.hpp"
#include "data/time.hpp"

namespace {
	// Spring code from https://theorangeduck.com/page/spring-roll-call
	float halflife_to_damping(float halflife, float eps = 1e-5f)
	{
		return (4.0f * 0.69314718056f) / (halflife + eps);
	}

	float damping_to_halflife(float damping, float eps = 1e-5f)
	{
		return (4.0f * 0.69314718056f) / (damping + eps);
	}
	float fast_negexp(float x)
	{
		return 1.0f / (1.0f + x + 0.48f*x*x + 0.235f*x*x*x);
	}
}

namespace Gts {

	void SpringBase::UpdateValues(float& value, const float& target, float & velocity, const float& halflife, const float& dt) {
		float y = halflife_to_damping(halflife) / 2.0F;
		float j0 = value - target;
		float j1 = velocity + j0*y;
		float eydt = fast_negexp(y*dt);

		value = eydt*(j0 + j1*dt) + target;
		velocity = eydt*(velocity - j1*y*dt);
	}

	void Spring::Update(float dt) {
		UpdateValues(this->value, this->target, this->velocity, this->halflife, dt);
	}

	Spring::Spring() {
		SpringManager::AddSpring(this);
	}

	Spring::Spring(float initial, float halflife) : value(initial), target(initial), halflife(halflife) {
		SpringManager::AddSpring(this);
	}

	Spring::~Spring() {
		SpringManager::RemoveSpring(this);
	}
	
	void Spring::Ser(SerializationInterface* serde) {
		uint32_t version = 0;
		serde->WriteRecordData(&version, sizeof(version));
		serde->WriteRecordData(&this->value, sizeof(this->value));
		serde->WriteRecordData(&this->target, sizeof(this->target));
		serde->WriteRecordData(&this->velocity, sizeof(this->velocity));
		serde->WriteRecordData(&this->halflife, sizeof(this->halflife));
	}
	void Spring::Des(SerializationInterface* serde) {
		uint32_t version = 0;
		serde->ReadRecordData(&version, sizeof(version));
		switch (version) {
			case 0: {
				serde->ReadRecordData(&this->value, sizeof(this->value));
				serde->ReadRecordData(&this->target, sizeof(this->target));
				serde->ReadRecordData(&this->velocity, sizeof(this->velocity));
				serde->ReadRecordData(&this->halflife, sizeof(this->halflife));
				return;
			}
		}
		throw std::runtime_error("Cannot deserilize spring of this version");
	}



	void Spring3::Update(float dt) {
		UpdateValues(this->value.x, this->target.x, this->velocity.x, this->halflife, dt);
		UpdateValues(this->value.y, this->target.y, this->velocity.y, this->halflife, dt);
		UpdateValues(this->value.z, this->target.z, this->velocity.z, this->halflife, dt);
	}

	Spring3::Spring3() {
		SpringManager::AddSpring(this);
	}

	Spring3::Spring3(NiPoint3 initial, float halflife) : value(initial), target(initial), halflife(halflife) {
		SpringManager::AddSpring(this);
	}

	Spring3::~Spring3() {
		SpringManager::RemoveSpring(this);
	}


	SpringManager& SpringManager::GetSingleton() {
		static SpringManager instance;
		return instance;
	}

	void SpringManager::AddSpring(SpringBase* spring)  {
		SpringManager::GetSingleton().springs.insert(spring);
	}
	void SpringManager::RemoveSpring(SpringBase* spring) {
		SpringManager::GetSingleton().springs.erase(spring);
	}

	std::string SpringManager::DebugName()  {
		return "SpringManager";
	}

	void SpringManager::Update() {
		float dt = Time::WorldTimeDelta();
		for (auto spring: this->springs) {
			spring->Update(dt);
		}
	}
}
