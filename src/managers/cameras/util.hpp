#pragma once

// #define ENABLED_SHADOW

namespace Gts {
	void SetINIFloat(std::string_view name, float value);

	float GetINIFloat(std::string_view name);

	float EnsureINIFloat(std::string_view name, float value);

	void UpdateThirdPerson();

	void ResetIniSettings();

	NiCamera* GetNiCamera();
	void UpdateWorld2ScreetMat(NiCamera* niCamera);

	#ifdef ENABLED_SHADOW
	ShadowSceneNode* GetShadowMap();
	#endif

	void UpdateSceneManager(NiPoint3 camLoc);

	void UpdateRenderManager(NiPoint3 camLoc);

	void UpdateNiCamera(NiPoint3 camLoc);

	void UpdatePlayerCamera(NiPoint3 camLoc);

	// Get's camera position relative to the player
	NiPoint3 GetCameraPosLocal();

	void ScaleCamera(float scale, NiPoint3 offset);
}
