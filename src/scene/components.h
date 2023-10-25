#pragma once

#include "utils/color.h"
#include "utils/math_utils.h"
#include "renderer/model.h"
#include "renderer/camera.h"

#include <string>
#include <memory>

namespace vre
{
	/// @brief Gives a name to the entity
	struct NameComponent
	{
		std::string name;

		NameComponent() = default;
		NameComponent(const NameComponent&) = default;
		NameComponent(const std::string& entityName)
			: name(entityName) {}
	};

	/// @brief Stores the transformation of the entity
	struct TransformComponent
	{
		Vector3 location = { 0.0f, 0.0f, 0.0f };
		Vector3 rotation = { 0.0f, 0.0f, 0.0f };
		Vector3 scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const Vector3& entityLocation)
			: location(entityLocation) {}
	};

	/// @brief Holds a pointer to a model
	struct MeshComponent
	{
		std::shared_ptr<Model> model;
		Color color;

		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
		MeshComponent(std::shared_ptr<Model> entityModel, const Color& baseColor = Color())
			: model(entityModel), color(baseColor) {}
	};

	/// @brief Creates a light 
	struct PointLightComponent
	{
		Vector3 color = { 1.0f, 1.0f, 1.0f };
		float intensity = 0.2f;
	};

	/// @brief Holds a camera to view the scene
	struct CameraComponent
	{
		Camera camera{};
	};

	class ScriptComponentBase;
	/// @brief Stores a custom script
	struct ScriptComponent
	{
		std::unique_ptr<ScriptComponentBase> script;
	};

} // namespace vre