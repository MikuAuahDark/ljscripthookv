LJScriptHook
=====
Another Lua-based GTA V ScriptHook but uses LuaJIT.

Requirements
-----

You need:

* CMake. Latest version is recommended. Tested in 3.16.3.

* Visual Studio 2019 x64 toolchain (2017 is untested).

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
