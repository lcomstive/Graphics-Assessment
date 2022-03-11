#include <string>
#include <Engine/Graphics/Shader.hpp>
#include <Engine/Components/Light.hpp>
#include <Engine/Components/Transform.hpp>

#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace Engine;
using namespace Engine::Graphics;
using namespace Engine::Components;

void Light::FillShadowData()
{
	Transform* transform = GetTransform();
	if (CastShadows)
	{
		switch (Type)
		{
		default:
		case LightType::Point:
		{
			// TODO: Point lights
			Transform pointTransform = *transform;
			for (unsigned int i = 0; i < LightMaxViews; i++)
			{
				ShadowData.Cameras[i].Orthographic = true;
				ShadowData.Cameras[i].OrthoSize = 10.0f;
				ShadowData.Cameras[i].ClipNear = 0.1f;
				ShadowData.Cameras[i].ClipFar = 100.0f;

				// pointTransform.Rotation = TODO THIS

				pointTransform.Update(0); // Update globals

				// Override camera's transform and set to this object's transform
				OverrideComponentInternals(&ShadowData.Cameras[i], nullptr, &pointTransform);

				ShadowData.Cameras[i].Update(0);

				ShadowData.LightSpaceMatrices[i] = ShadowData.Cameras[i].GetProjectionMatrix() * ShadowData.Cameras[i].GetViewMatrix();
			}
		}
		break;
		case LightType::Spot:
			// TODO: Spot Lights
			for (unsigned int i = 1; i < LightMaxViews; i++)
				ShadowData.ShadowMapArrayIndex[i] = -1;
			break;
		case LightType::Directional:
		{
			Transform* mainCamTransform = Camera::GetMainCamera()->GetTransform();
			for (unsigned int i = 1; i < LightMaxViews; i++)
				ShadowData.ShadowMapArrayIndex[i] = -1;

			const float DirNear = 1.0f;
			const float DirFar = 100.0f;
			ShadowData.Cameras[0].Orthographic = true;
			ShadowData.Cameras[0].OrthoSize = 10.0f;
			ShadowData.Cameras[0].ClipNear = DirNear;
			ShadowData.Cameras[0].ClipFar = DirFar;

			Transform dirTransform;
			dirTransform.Rotation = transform->Forward();
			dirTransform.Update(0); // Update globals

			// Override camera's transform and set to this object's transform
			OverrideComponentInternals(&ShadowData.Cameras[0], nullptr, &dirTransform);

			ShadowData.Cameras[0].Update(0);

			// Calculate view matrix
			glm::vec3 pos = mainCamTransform->GetGlobalPosition() - transform->Forward() * (DirFar / 2.0f);
			glm::mat4 viewMatrix = glm::lookAt(
				pos,
				pos + transform->Forward(),
				glm::vec3 { 0, 1, 0 }
			);

			// Combine matrices
			ShadowData.LightSpaceMatrices[0] = ShadowData.Cameras[0].GetProjectionMatrix() * viewMatrix;
			break;
		}
		}
	}
}

void Light::FillShader(unsigned int lightIndex, Shader* shader)
{
	Transform* transform = GetTransform();
	string prefix = "lights[" + to_string(lightIndex) + "].";
	for (unsigned int i = 0; i < LightMaxViews; i++)
	{
		if(!CastShadows)
			ShadowData.ShadowMapArrayIndex[i] = -1;
		shader->Set(prefix + "Position", Type != LightType::Directional ?
											 transform->GetGlobalPosition() :
											-transform->Forward() * 10000.0f); // Directional light position is very far
		shader->Set(prefix + "Colour", Colour);
		shader->Set(prefix + "Radius", Type == LightType::Directional ? 999999.99f : Radius);
		shader->Set(prefix + "Intensity", Intensity);
		shader->Set(prefix + "ShadowMapIndex["   + to_string(i) + "]", ShadowData.ShadowMapArrayIndex[i]);
		shader->Set(prefix + "LightSpaceMatrix[" + to_string(i) + "]", ShadowData.LightSpaceMatrices[i]);
		shader->Set(prefix + "Type", (int)Type);
		shader->Set(prefix + "CastShadows", CastShadows);
		shader->Set(prefix + "Direction", transform->GetGlobalRotation());
	}
}