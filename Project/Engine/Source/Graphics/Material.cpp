#include <Engine/Log.hpp>
#include <Engine/Graphics/Shader.hpp>
#include <Engine/Graphics/Texture.hpp>
#include <Engine/Graphics/Material.hpp>

using namespace std;
using namespace Engine;
using namespace Engine::Graphics;

void BindTexture(unsigned int index, string shaderName, Texture* texture, Shader* shader)
{
	shader->Set(shaderName, (int)index);
	glActiveTexture(GL_TEXTURE0 + index);
	
	if (texture)
		texture->Bind(index);
	else
		glBindTexture(GL_TEXTURE_2D, 0);
}

void Material::FillShader(Shader* shader) const
{
	if (!shader)
		return;

	shader->Set("material.AlbedoColour", Albedo);

	shader->Set("material.Roughness", Roughness);
	shader->Set("material.Metalness", Metalness);
	shader->Set("material.AlphaClipping", AlphaClipping);
	shader->Set("material.AlphaClipThreshold", AlphaClipThreshold);
	shader->Set("material.TextureCoordScale", TextureCoordinateScale);
	shader->Set("material.TextureCoordOffset", TextureCoordinateOffset);
	
	shader->Set("material.HasAlbedoMap", AlbedoMap != nullptr);
	shader->Set("material.HasNormalMap", NormalMap != nullptr);
	shader->Set("material.HasRoughnessMap", RoughnessMap != nullptr);
	shader->Set("material.HasMetalnessMap", MetalnessMap != nullptr);
	shader->Set("material.HasAmbientOcclusionMap", AmbientOcclusionMap != nullptr);

	BindTexture(0, "material.AlbedoMap", AlbedoMap, shader);
	BindTexture(1, "material.NormalMap", NormalMap, shader);
	BindTexture(2, "material.RoughnessMap", RoughnessMap, shader);
	BindTexture(3, "material.MetalnessMap", MetalnessMap, shader);
	BindTexture(4, "material.AmbientOcclusionMap", AmbientOcclusionMap, shader);
}
