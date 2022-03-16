#include <Engine/Log.hpp>
#include <Engine/ResourceManager.hpp>
#include <Engine/Graphics/Shader.hpp>
#include <Engine/Graphics/Texture.hpp>
#include <Engine/Graphics/Material.hpp>

using namespace std;
using namespace Engine;
using namespace Engine::Graphics;

void BindTexture(unsigned int index, string shaderName, ResourceID& textureID, Shader* shader)
{
	shader->Set(shaderName, (int)index);
	glActiveTexture(GL_TEXTURE0 + index);
	
	if (textureID != InvalidResourceID)
	{
		Texture* texture = ResourceManager::Get<Texture>(textureID);
		if(texture)
			texture->Bind(index);
	}
	else
		glBindTexture(GL_TEXTURE_2D, 0);
}

void Material::FillShader(Shader* shader)
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
	
	shader->Set("material.HasAlbedoMap", AlbedoMap != InvalidResourceID);
	shader->Set("material.HasNormalMap", NormalMap != InvalidResourceID);
	shader->Set("material.HasRoughnessMap", RoughnessMap != InvalidResourceID);
	shader->Set("material.HasMetalnessMap", MetalnessMap != InvalidResourceID);
	shader->Set("material.HasAmbientOcclusionMap", AmbientOcclusionMap != InvalidResourceID);

	BindTexture(0, "material.AlbedoMap", AlbedoMap, shader);
	BindTexture(1, "material.NormalMap", NormalMap, shader);
	BindTexture(2, "material.RoughnessMap", RoughnessMap, shader);
	BindTexture(3, "material.MetalnessMap", MetalnessMap, shader);
	BindTexture(4, "material.AmbientOcclusionMap", AmbientOcclusionMap, shader);
	// Texture Slot 5 reserved for Shadow Map
	// Texture Slot 6 reserved for Environment Map
	// Texture Slot 7 reserved for Irradiance Map
}

void Material::Serialize(DataStream& stream)
{
	stream.Serialize(&Albedo);
	stream.Serialize(&Roughness);
	stream.Serialize(&Metalness);
	stream.Serialize(&AlphaClipping);
	stream.Serialize(&AlphaClipThreshold);
	stream.Serialize(&TextureCoordinateScale);
	stream.Serialize(&TextureCoordinateOffset);

	SerializeTexture(stream, AlbedoMap);
	SerializeTexture(stream, NormalMap);
	SerializeTexture(stream, RoughnessMap);
	SerializeTexture(stream, MetalnessMap);
	SerializeTexture(stream, AmbientOcclusionMap);
}

void Material::SerializeTexture(DataStream& stream, ResourceID& texture)
{
	string name = texture ? ResourceManager::GetName(texture) : "";
	string path = texture ? ResourceManager::Get<Texture>(texture)->GetPath() : "";
	stream.Serialize(&name);
	stream.Serialize(&path);
	if (stream.IsReading() && !path.empty())
		texture = ResourceManager::LoadNamed<Texture>(name, path);
}