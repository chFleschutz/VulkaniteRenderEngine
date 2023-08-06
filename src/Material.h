#pragma once

#include <string>

namespace vre
{
	class Material
	{
	public:
		static Material createFromFile(const std::string& texturePath);
		 
	private:
		// refrence Pipeline
		// Texture(s) or descriptor bindings
	};

} // namespace vre