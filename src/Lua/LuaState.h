#pragma once
/*
 * Copyright 2010-2016 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string>
#include <filesystem>

extern "C"
{
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

namespace OpenXcom
{

// Forward declarations
class Game;
class ModInfo;

namespace Lua
{

/**
 * LuaState is the base state used to run the LUA scripts. It instantiates a lua_State object and provides it with the in-game API.
 */
class LuaState
{
private:
	lua_State *_state; // The lua_State object

	bool _error; // Error flag
	std::string _errorString; // Error message

	const ModInfo* _modData; // The mod data

	std::filesystem::path _scriptPath; // The path to the script file

	/**
	 * Loads a script from a file.
	 * @param filename The name of the file to load.
	 * @return True if the script was loaded successfully, false otherwise.
	 */
	bool loadScript(const std::filesystem::path& filename);

  public:
	LuaState(const std::filesystem::path& scriptPath, const ModInfo* modData); // Constructor
	~LuaState(); // Destructor

	const std::filesystem::path &getScriptPath() const; // Returns the path to the script file
	const ModInfo* getModData() const;                  // Returns the mod data
};

} // namespace Lua
} // namespace OpenXcom
