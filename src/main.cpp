/*
 * Copyright (c) 2020 Miku AuahDark
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *		claim that you wrote the original software. If you use this software
 *		in a product, an acknowledgment in the product documentation would be
 *		appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *		misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

#include "check64.cpp"

#include "../ScriptHookSDK/inc/main.h"

#include "../LuaJIT/src/lua.hpp"

static std::unordered_map<std::string, lua_State*> luaState;

inline uint32_t getMainScriptIndex(lua_State *L)
{
	union {lua_State *a; uint64_t b;} c = {L};
	return uint32_t((c.b >> 32ULL) ^ (c.b & UINT32_MAX));
}

static void logDebug(const std::string &str)
{
	OutputDebugStringA((str + '\n').c_str());
	printf("%s\n", str.c_str());
}

static lua_State *newLuaState(const std::string &path)
{
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);

	// Load debug.traceback at REGISTRY 0 and leave it at index 1
	lua_pushstring(L, "debug");
	lua_rawget(L, LUA_GLOBALSINDEX); // debug table = 1
	lua_pushstring(L, "traceback");
	lua_rawget(L, -2); // traceback function = index 2
	lua_remove(L, 1); // remove debug table; traceback function = index 1
	lua_pushvalue(L, -1); // copy traceback function = index 2
	lua_rawseti(L, LUA_REGISTRYINDEX, 0); // now traceback function = index 1

	// Load script file (index 2)
	if (luaL_loadfile(L, path.c_str()))
	{
		logDebug(lua_tostring(L, -1));
		lua_close(L);
		return nullptr;
	}

	// Execute main.lua
	luaL_dofile(L, "ljscripts\\main.lua");

	// Execute script file. The resulting script table is at index 2
	if (lua_pcall(L, 0, 1, 1))
	{
		logDebug(lua_tostring(L, -1));
		logDebug("Error when loading script " + path);
		lua_close(L);
		return nullptr;
	}

	// Store it at random script index
	lua_pushinteger(L, getMainScriptIndex(L));
	lua_pushvalue(L, -2);
	lua_rawset(L, LUA_REGISTRYINDEX);

	// Check update function existence
	lua_pushstring(L, "update");
	lua_rawget(L, 2);
	if (lua_isnil(L, -1))
	{
		logDebug("Error when loading script " + path);
		logDebug("Missing update function");
		lua_close(L);
		return nullptr;
	}
	lua_pop(L, 3); // remove update function, script table, and debug.traceback

	// Setup package.path
	lua_pushstring(L, "package");
	lua_rawget(L, LUA_GLOBALSINDEX); // package = index 1
	lua_pushstring(L, "path"); // path = index 2
	lua_pushvalue(L, -1); // path = index 3
	lua_rawget(L, -3); // package.path = index 3
	lua_pushstring(L, "ljscripts/libs/?.lua;");
	lua_concat(L, 2); // package.path concat = 3
	lua_rawset(L, -3);

	// Perfectly balanced as all things should be
	assert(lua_gettop(L) == 0);
	return L;
}

static void scriptMain()
{
	std::vector<std::string> forRemoval;

	// Iterate all loaded scripts
	for (std::pair<std::string, lua_State *> scripts: luaState)
	{
		lua_State *L = scripts.second;

		// Get current script table (index 1)
		lua_pushinteger(L, getMainScriptIndex(L));
		lua_rawget(L, LUA_REGISTRYINDEX);

		// Get traceback function (index 2)
		lua_rawgeti(L, LUA_REGISTRYINDEX, 0);

		// Get update function (index 3)
		lua_pushstring(L, "update");
		lua_rawget(L, 1);

		// TODO: Pass delta Time?
		if (lua_pcall(L, 0, 0, 2))
		{
			logDebug(lua_tostring(L, -1));
			logDebug("Error when executing script " + scripts.first);
			forRemoval.push_back(scripts.first);
		}

		// remove update function, traceback function, and script table
		lua_pop(L, 3);

		// Perfectly balanced as all things should be
		assert(lua_gettop(L) == 0);
	}

	// Remove scripts which is scheduled for removal
	for (const std::string &removal: forRemoval)
	{
		lua_close(luaState[removal]);
		luaState.erase(removal);
	}

	// Script wait
	scriptWait(0);
}

BOOL APIENTRY DllMain(HMODULE hInstance, DWORD reason, LPVOID lpReserved)
{
	static bool init = false;

	switch (reason)
	{
		case DLL_PROCESS_ATTACH:
		{
			// Verify "ljscripts" folder
			DWORD ljscriptsDir = GetFileAttributesA("ljscripts");
			if (ljscriptsDir == INVALID_FILE_ATTRIBUTES || !(ljscriptsDir & FILE_ATTRIBUTE_DIRECTORY))
			{
				MessageBoxA(NULL, "Missing \"ljscripts\" directory!", "Error", MB_ICONERROR);
				return 0;
			}

			// Check main.lua existence
			DWORD mainFile = GetFileAttributesA("ljscripts\\native.lua");
			if (mainFile == INVALID_FILE_ATTRIBUTES || (mainFile & FILE_ATTRIBUTE_DIRECTORY))
			{
				MessageBoxA(NULL, "Missing \"main.lua\"!", "Error", MB_ICONERROR);
				return 0;
			}

			// List files in ljscripts/addins directory
			WIN32_FIND_DATAA findData;
			HANDLE dir = FindFirstFileA("ljscripts\\addins\\*", &findData);
			if (dir == INVALID_HANDLE_VALUE)
			{
				MessageBoxA(NULL, "Cannot iterate \"ljscripts\\addins\" folder!", "Error", MB_ICONERROR);
				return 0;
			}

			do
			{
				if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					continue;
				
				lua_State *L = newLuaState(std::string("ljscripts\\addins\\") + findData.cFileName);
				if (L)
					luaState[findData.cFileName] = L;
				else
					logDebug(std::string("Unable to load ") + findData.cFileName);
			} while (FindNextFileA(dir, &findData) != 0);

			// Only register ScriptHook if there's scripts to be loaded.
			if (luaState.size() > 0)
			{
				scriptRegister(hInstance, scriptMain);
				init = true;
			}
			else
				logDebug("No scripts loaded!");

			return (BOOL) init;
		}
		case DLL_PROCESS_DETACH:
		{
			if (init)
			{
				for (std::pair<std::string, lua_State *> scripts: luaState)
					lua_close(scripts.second);

				luaState.clear();
				scriptUnregister(hInstance);
				init = false;
			}
			break;
		}
	}

	return TRUE;
}
