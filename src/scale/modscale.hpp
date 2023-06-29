#pragma once
// Handles the various methods of scaling an actor


using namespace std;
using namespace RE;
using namespace SKSE;

namespace Gts {
	enum SizeMethod {
		ModelScale = 0,
		RootScale = 1,
    Hybrid = 2,
    RefScale = 3,
	};

  	float Get_Other_Scale(Actor* actor);

	void set_ref_scale(Actor* actor, float target_scale);
	bool set_model_scale(Actor* actor, float target_scale);
	bool set_npcnode_scale(Actor* actor, float target_scale);

	float get_npcnode_scale(Actor* actor);
	float get_npcparentnode_scale(Actor* actor);
	float get_model_scale(Actor* actor);
	float get_ref_scale(Actor* actor);
	float get_scale(Actor* actor);
	bool set_scale(Actor* actor, float scale);
}
