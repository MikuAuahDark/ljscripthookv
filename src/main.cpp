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

#include <mutex>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>

#include "check64.cpp"

#include "../ScriptHookSDK/inc/main.h"

#include "../LuaJIT/src/lua.hpp"

constexpr const char *UPDATE_FUNCTION = "update";
constexpr const char *KEYHAND_FUNCTION = "keyboardHandler";
constexpr uint32_t MAX_CONSECUTIVE_ERROR = 10;

struct KeyboardInput
{
	DWORD key;
	WORD repeats;
	BYTE scanCode;
	BOOL isExtended;
	BOOL isWithAlt;
	BOOL wasDownBefore;
	BOOL isUpNow;
};

struct LuaStateInfo
{
	lua_State *L;
	std::queue<KeyboardInput> *key;
	std::mutex *keyMutex;
	uint32_t errorCount;
};

static std::unordered_map<std::string, LuaStateInfo*> luaState;
static FILE *logFile = nullptr;

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
	for (std::pair<const std::string, LuaStateInfo*> &scripts: luaState)
	{
		// Apparently keyboard handler thread is not same as the script thread
		LuaStateInfo *info = scripts.second;

		if (info->key)
		{
			std::lock_guard<std::mutex> lock(*info->keyMutex);
			info->key->push({key, repeats, scanCode, isExtended, isWithAlt, wasDownBefore, isUpNow});
		}
	}
}

static int quitScript(lua_State* L)
{
	for (std::pair<const std::string, LuaStateInfo*>& scripts : luaState)
	{
		if (scripts.second->L == L)
		{
			scripts.second->errorCount = MAX_CONSECUTIVE_ERROR + 1;
			break;
		}
	}

	return 0;
}

static LuaStateInfo *newLuaState(const std::string &path)
{
	logDebug("Attempt to load " + path);

	lua_State *L = luaL_newstate();
	luaL_openlibs(L);

	// Setup package.path
	luaL_dostring(L, "package.path = package.path..\"ljscripts/libs/?.lua;ljscripts/libs/?/init.lua\"");

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

	// Remove traceback function. Script table is now on top.
	lua_remove(L, 1);

	// Check update function existence
	lua_pushstring(L, UPDATE_FUNCTION);
	lua_rawget(L, 1);
	if (lua_isnil(L, -1))
	{
		logDebug("Error when loading script " + path);
		logDebug("Missing update function");
		lua_close(L);
		return nullptr;
	}

	// Remove update function
	lua_pop(L, 1);

	// Create new state info
	LuaStateInfo *info = new LuaStateInfo();
	info->L = L;
	info->key = nullptr;
	info->keyMutex = nullptr;
	info->errorCount = 0;

	// Check keyboard handler existence
	lua_pushstring(L, KEYHAND_FUNCTION);
	lua_rawget(L, 1);
	if (lua_isfunction(L, -1))
	{
		info->key = new std::queue<KeyboardInput>;
		info->keyMutex = new std::mutex();
	}

	// Register quitScript function
	lua_pushcfunction(L, quitScript);
	lua_setglobal(L, "quitScript");

	lua_pop(L, 1);
	// Perfectly balanced as all things should be
	assert(lua_gettop(L) == 1);
	logDebug("success");
	return info;
}

static void closeLuaState(LuaStateInfo* info)
{
	if (info->keyMutex)
	{
		info->keyMutex->lock();
		info->keyMutex->unlock();
	}

	lua_close(info->L);
	delete info->key;
	delete info->keyMutex;
	delete info;
}

static void scriptMain()
{
	while (true)
	{
		std::vector<std::string> forRemoval;

		// Iterate all loaded scripts
		for (std::pair<const std::string, LuaStateInfo*> &scripts: luaState)
		{
			LuaStateInfo *info = scripts.second;
			lua_State *L = info->L;
			bool error = false;

			// Script table is already at index 1
			assert(lua_gettop(L) == 1);

			// Push traceback function (index 2)
			lua_pushcfunction(L, traceback);

			if (info->key)
			{
				std::lock_guard<std::mutex> lock(*info->keyMutex);

				while (!info->key->empty())
				{
					KeyboardInput input = info->key->front();

					// Get keyboard handler function (index 3)
					lua_pushstring(L, KEYHAND_FUNCTION);
					lua_rawget(L, 1);
					lua_pushinteger(L, input.key);
					lua_pushinteger(L, input.repeats);
					lua_pushinteger(L, input.scanCode);
					lua_pushboolean(L, input.isExtended);
					lua_pushboolean(L, input.isWithAlt);
					lua_pushboolean(L, input.wasDownBefore);
					lua_pushboolean(L, input.isUpNow);

					if (lua_pcall(L, 7, 0, 2))
					{
						logDebug("Error when executing keyboard handler of script " + scripts.first);
						logDebug(lua_tostring(L, -1));
						lua_pop(L, 1);
						error = true;
					}

					info->key->pop();
				}
			}

			// Get update function (index 3)
			lua_pushstring(L, UPDATE_FUNCTION);
			lua_rawget(L, 1);

			// Call function. stack is now 2
			if (lua_pcall(L, 0, 0, 2))
			{
				logDebug("Error when executing update function of script " + scripts.first);
				logDebug(lua_tostring(L, -1));
				lua_pop(L, 1);
				error = true;
			}

			// Remove traceback function
			lua_pop(L, 1);
			assert(lua_gettop(L) == 1);

			if (info->errorCount >= MAX_CONSECUTIVE_ERROR)
				forRemoval.push_back(scripts.first);
			else if (error)
				info->errorCount++;
			else
				info->errorCount = 0;
		}

		// Remove scripts which is scheduled for removal
		for (const std::string &removal: forRemoval)
		{
			LuaStateInfo *info = luaState[removal];

			if (info)
			{
				luaState.erase(removal);
				closeLuaState(info);
			}
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
				
				LuaStateInfo *info = newLuaState(std::string("ljscripts\\addins\\") + findData.cFileName);
				if (info)
					luaState[findData.cFileName] = info;
				else
					logDebug(std::string("Unable to load ") + findData.cFileName);
			} while (FindNextFileA(dir, &findData) != 0);

			// Only register ScriptHook if there's scripts to be loaded.
			if (luaState.size() > 0)
			{
				scriptRegister(hInstance, scriptMain);
				keyboardHandlerRegister(keyboardHandler);
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
				for (std::pair<const std::string, LuaStateInfo*> scripts: luaState)
					closeLuaState(scripts.second);

				luaState.clear();
				scriptUnregister(hInstance);
				keyboardHandlerUnregister(keyboardHandler);

				if (logFile != stdout)
					fclose(logFile);

				init = false;
			}
			break;
		}
	}

	return TRUE;
}
