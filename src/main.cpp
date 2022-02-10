/*
 * Copyright (c) 2022 Miku AuahDark
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
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include <cassert>
#include <cstdint>
#include <ctime>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>

#include "check64.cpp"

#include "../ScriptHookSDK/inc/main.h"

#include "../LuaJIT/src/lua.hpp"

static std::unordered_map<std::string, lua_State*> luaState;
static FILE *logFile = nullptr;

inline uint32_t getMainScriptIndex(lua_State *L)
{
	uint64_t value = (uint64_t) L;
	return uint32_t((value >> 32ULL) ^ (value & UINT32_MAX));
}

static void logDebug(const std::string &str)
{
	time_t timestamp = time(nullptr);
	tm *timeinfo = localtime(&timestamp);

	char timebuf[256];
	std::string bufstr = std::string(timebuf, strftime(timebuf, 256, "[%H:%M:%S] ", timeinfo));
	std::string message = bufstr + str + '\n';

	OutputDebugStringA(("[ljscripthookv] " + message).c_str());

	if (logFile)
		fwrite(message.data(), 1, message.length(), logFile);
}

static int traceback(lua_State *L)
{
	if (!lua_isstring(L, 1))  /* 'message' not a string? */
		return 1;  /* keep it intact */

	luaL_traceback(L, L, lua_tostring(L, 1), 1);
	return 1;
}

static int printToLog(lua_State* L)
{
	int nargs = lua_gettop(L);

	lua_getglobal(L, "tostring");

	std::stringstream outstring;

	for (int i = 1; i <= nargs; i++)
	{
		// Call tostring(arg) and leave the result on the top of the stack.
		lua_pushvalue(L, -1);
		lua_pushvalue(L, i);
		lua_call(L, 1, 1);

		const char *s = lua_tostring(L, -1);
		if (s == nullptr)
			return luaL_error(L, "'tostring' must return a string to 'print'");

		if (i > 1)
			outstring << "\t";

		outstring << s;

		lua_pop(L, 1); // Pop the result of tostring(arg).
	}

	logDebug(outstring.str());
	return 0;
}

static void keyboardHandler(DWORD key, WORD repeats, BYTE scanCode, BOOL isExtended, BOOL isWithAlt, BOOL wasDownBefore, BOOL isUpNow)
{
	// Iterate all loaded scripts
	for (std::pair<std::string, lua_State *> scripts: luaState)
	{
		lua_State *L = scripts.second;

		// Get current script table (index 1)
		lua_pushinteger(L, getMainScriptIndex(L));
		lua_rawget(L, LUA_REGISTRYINDEX);

		// Push traceback function
		lua_pushcfunction(L, traceback);

		// Get keyboardHandler function (index 3)
		lua_pushstring(L, "keyboardHandler");
		lua_rawget(L, 1);

		if (lua_isfunction(L, -1))
		{
			lua_pushinteger(L, key);
			lua_pushinteger(L, repeats);
			lua_pushinteger(L, scanCode);
			lua_pushboolean(L, isExtended);
			lua_pushboolean(L, isWithAlt);
			lua_pushboolean(L, wasDownBefore);
			lua_pushboolean(L, isUpNow);

			if (lua_pcall(L, 7, 0, 2))
			{
				logDebug("Error when executing script at keyboard handler " + scripts.first);
				logDebug(lua_tostring(L, -1));
			}
		}
		
		lua_pop(L, 3);
		// Perfectly balanced as all things should be
		assert(lua_gettop(L) == 0);
	}
}

static lua_State *newLuaState(const std::string &path)
{
	logDebug("Attempt to load " + path);

	lua_State *L = luaL_newstate();
	luaL_openlibs(L);

	// Setup package.path
	luaL_dostring(L, "package.path = package.path..\"ljscripts/libs/?.lua;\"");

	// Push traceback function (index 1)
	lua_pushcfunction(L, traceback);

	// Load script file (index 2)
	if (luaL_loadfile(L, path.c_str()))
	{
		logDebug(lua_tostring(L, -1));
		lua_close(L);
		return nullptr;
	}

	// Execute main.lua, stack is unchanged
	if (luaL_loadfile(L, "ljscripts\\main.lua") || lua_pcall(L, 0, 0, 1))
	{
		logDebug("Error when loading script " + path);
		logDebug(lua_tostring(L, -1));
		lua_close(L);
		return nullptr;
	}

	// Execute script file. Script table is at index 2
	if (lua_pcall(L, 0, 1, 1))
	{
		logDebug("Error when loading script " + path);
		logDebug(lua_tostring(L, -1));
		lua_close(L);
		return nullptr;
	}
	
	if (lua_isnil(L, -1))
	{
		logDebug("Error when loading script " + path);
		logDebug("Missing script table");
		lua_close(L);
		return nullptr;
	}

	// Store it at random script index
	lua_pushinteger(L, getMainScriptIndex(L));
	lua_pushvalue(L, 2);
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

	// Perfectly balanced as all things should be
	assert(lua_gettop(L) == 0);
	logDebug("success");
	return L;
}

static void scriptMain()
{
	while (true)
	{
		std::vector<std::string> forRemoval;

		// Iterate all loaded scripts
		for (std::pair<std::string, lua_State *> scripts: luaState)
		{
			lua_State *L = scripts.second;
			assert(lua_gettop(L) == 0);

			// Get current script table (index 1)
			lua_pushinteger(L, getMainScriptIndex(L));
			lua_rawget(L, LUA_REGISTRYINDEX);

			// Push traceback function (index 2)
			lua_pushcfunction(L, traceback);

			// Get update function (index 3)
			lua_pushstring(L, "update");
			lua_rawget(L, 1);

			// Call function. stack is now 2
			if (lua_pcall(L, 0, 0, 2))
			{
				logDebug("Error when executing script " + scripts.first);
				logDebug(lua_tostring(L, -1));
				forRemoval.push_back(scripts.first);
				lua_pop(L, 1);
			}

			lua_pop(L, 2);
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

			// Open log file
			//freopen("ljscripts\\log.txt", "ab", stdout);
			logFile = fopen("ljscripts\\log.txt", "ab");

			if (logFile)
				setvbuf(logFile, nullptr, _IONBF, 0);
			else
				logFile = stdout;

			logDebug("=== LJScript starts ===");

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
				//keyboardHandlerRegister(keyboardHandler);
				init = true;
			}
			else
				logDebug("No scripts loaded!");

			if (!init && logFile != stdout)
				fclose(logFile);

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
				//keyboardHandlerUnregister(keyboardHandler);

				if (logFile != stdout)
					fclose(logFile);

				init = false;
			}
			break;
		}
	}

	return TRUE;
}
