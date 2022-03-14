#include <string>
#include <Engine/Graphics/Shader.hpp>
#include <Engine/Components/Light.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <Engine/Components/Transform.hpp>
#include <Engine/Graphics/Passes/ShadowMap.hpp>

using namespace std;
using namespace glm;
using namespace Engine;
using namespace Engine::Graphics;
using namespace Engine::Components;

bool Light::GetCastShadows() { return m_CastShadows; }
void Light::SetCastShadows(bool canCast)
{
	if (canCast == m_CastShadows)
		return; // No change

	m_CastShadows = canCast;

	ShadowMapPass* shadowPass = Renderer::GetPipeline()->GetShadowMapPass();
	if (m_CastShadows)
		shadowPass->AddShadowCaster(this);
	else
		shadowPass->RemoveShadowCaster(this);
}

void Light::FillShader(unsigned int lightIndex, Shader* shader)
{
	Transform* transform = GetTransform();
	string prefix = "lights[" + to_string(lightIndex) + "].";

	shader->Set(prefix + "Position", Type != LightType::Directional ?
		transform->GetGlobalPosition() :
		-transform->Forward() * 10000.0f); // Directional light position is very far

	shader->Set(prefix + "Colour", Colour);
	shader->Set(prefix + "Type", (int)Type);
	shader->Set(prefix + "Distance", Distance);
	shader->Set(prefix + "Intensity", Intensity);
	shader->Set(prefix + "CastShadows", m_CastShadows);
	shader->Set(prefix + "Direction", transform->Forward());
	shader->Set(prefix + "FadeCutoffInner", FadeCutoffInner);
	shader->Set(prefix + "Radius", Type == LightType::Directional ? 999999.99f : Radius);

	if (m_CastShadows)
	{
		ShadowMapPass::LightShadowData& shadowData = Renderer::GetPipeline()->GetShadowMapPass()->m_ShadowCasters[this];
		shader->Set(prefix + "ShadowMapIndex", shadowData.ShadowMapArrayIndex);
		shader->Set(prefix + "LightSpaceMatrix", shadowData.LightSpaceMatrix);
	}
	else
	{
		shader->Set(prefix + "ShadowMapIndex", -1);
		shader->Set(prefix + "LightSpaceMatrix", mat4(1.0f));
	}
}