LJScriptHook
=====
Another Lua-based GTA V ScriptHook but uses LuaJIT.

Requirements
-----

You need:

* CMake. Latest version is recommended. Tested in 3.16.3 and 3.22.2.

* Visual Studio 2019 x64 toolchain or later version (2017 is untested).

* ScriptHookSDK with latest [natives.h](http://www.dev-c.com/nativedb/) definition.

Compiling
-----

First, copy the contents of ScriptHookVSDK to `ScriptHookSDK` folder. These files are checked:

* ScriptHookSDK/inc/natives.h

* ScriptHookSDK/lib/ScriptHookV.lib

Then, from your command prompt, compile the project:

```cmd
cmake -Bbuild -H. -A x64 -DCMAKE_INSTALL_DIR=%CD%\install
cmake --build build --config Release --target install
```

Installation
-----

Copy the contents of `install` to your GTA V. Also, create `libs` and `addins`
folder inside the `ljscripts` folder before or after copying.

We Don't Need Another [Lua ScriptHookV](https://github.com/gallexme/LuaPlugin-GTAV)
-----

That one doesn't fit my needs because it uses vanilla Lua.

The philosophy of this LJScriptHookV is to let LuaJIT handle almost everything.
This includes calling ScriptHookV natives, yes LuaJIT calls ScriptHookV natives directly
without additional bindings (see `ljscripts/natives.lua`). This is possible thanks to LuaJIT
built-in FFI library which opens completely new possibilities in game modding.

The `src/main.cpp` is quite small, it only handles setting up minimal environment for the
script core functionality including script loading, update loop, and keyboard handling.
The rest of it is handled by LuaJIT.

I never say this function is compatible with another Lua ScriptHookV. You may need to write lots
of boilerplate code but afterwards porting should be straightforward.

Known Issues
-----

* Whole script crashes when there's Lua error in the script. Probably has to do with LuaJIT exception handling or something.

* Many things are untested.
