cmake_minimum_required(VERSION 3.1)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "Prevented in-tree build.")
endif()

# For LuaJIT compilation
include(ExternalProject)

project(ljscripthook LANGUAGES CXX)

# We only support MSVC
if (MSVC)
	# This script is directly copied from LOVE Megasource
	if(NOT(MSVC_TOOLSET_VERSION STREQUAL ""))
		set(VSVERSION "${MSVC_TOOLSET_VERSION}")
	elseif((MSVC_VERSION GREATER 1909) AND (MSVC_VERSION LESS 1920))
		set(VSVERSION "141")
	elseif(MSVC_VERSION EQUAL 1900)
		set(VSVERSION "140")
	elseif(MSVC_VERSION EQUAL 1800)
		set(VSVERSION "120")
	elseif(MSVC_VERSION LESS 1800)
		message(WARNING "Visual Studio 2013 (VS12) or newer is required!")
	else()
		message(WARNING "Cannot determine Visual Studio version!")
	endif()

	set(PROGRAMFILES_X86_STR "ProgramFiles(x86)")
	set(PROGRAMFILES_X86 "$ENV{${PROGRAMFILES_X86_STR}}")

	set(VSCOMNTOOLSSTRING "VS${VSVERSION}COMNTOOLS")
	set(VSCOMNTOOLS "$ENV{${VSCOMNTOOLSSTRING}}")

	if(VSCOMNTOOLS STREQUAL "")
		set(VSWHERE_PATH "${PROGRAMFILES_X86}\\Microsoft Visual Studio\\Installer\\vswhere.exe")

		execute_process(COMMAND ${VSWHERE_PATH} "-property" "installationPath" OUTPUT_VARIABLE VS_INSTALL_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)

		if(NOT(VS_INSTALL_DIR STREQUAL ""))
			set(VSCOMNTOOLS "${VS_INSTALL_DIR}\\Common7\\Tools")
		endif()
	endif()

	if(VSCOMNTOOLS STREQUAL "")
		message(WARNING "Environment variable VSCOMNTOOLS not defined and vswhere.exe could not be found. Is Visual Studio properly installed?")
	else()
		# Find vcvarsall.bat.
		get_filename_component(VS_VCVARSALL_BAT ${VSCOMNTOOLS}/../../VC/Auxiliary/Build/vcvarsall.bat ABSOLUTE)

		if(NOT(EXISTS ${VS_VCVARSALL_BAT}))
			# older VS versions have it here
			get_filename_component(VS_VCVARSALL_BAT ${VSCOMNTOOLS}/../../VC/vcvarsall.bat ABSOLUTE)
		endif()

		if(EXISTS ${VS_VCVARSALL_BAT})
			message(STATUS "Found vcvarsall.bat")
			set(MEGA_MSVC_VCVARSALL_BAT ${VS_VCVARSALL_BAT})
			set(MEGA_MSVC_VCVARSALL_BAT_ARG "x86_amd64")
		else()
			message(FATAL_ERROR "Could not find vcvarsall.bat")
		endif()
	endif()
else()
	message(FATAL_ERROR "Please use MSVC compiler targetting x64")
endif()

# Test x64 target
try_compile(IS_64BIT ${CMAKE_CURRENT_BINARY_DIR}
	SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/check64.cpp
	COMPILE_DEFINITIONS -DCMAKE_TRY_COMPILE
)
if (NOT IS_64BIT)
	message(FATAL_ERROR "Only 64-bit target is supported")
endif ()

# Find Lua executable
if (NOT LUA_EXE)
	find_program(LUA_EXE
		NAMES luajit lua5.1 lua5.2 lua5.3 lua
		DOC "Lua executable to use to generate definitions."
	)
endif()

if (NOT LUA_EXE)
	message(STATUS "No Lua interpreter found. Provided LuaJIT will be used.")
else()
	message(STATUS "Lua interpreter: ${LUA_EXE}")
endif()

# Custom LuaJIT build
# This script is directly copied from LOVE Megasource
# Copy LuaJIT source to binary dir. LuaJIT builds in-source,
# and we don't want to modify the original source tree, so
# we copy it.
set(MEGA_LUAJIT_SOURCE_DIR ${CMAKE_BINARY_DIR}/LuaJIT)
file(COPY LuaJIT DESTINATION ${CMAKE_BINARY_DIR})
set(MEGA_LUAJIT_BUILD_BAT "
	@echo off
	call \"${MEGA_MSVC_VCVARSALL_BAT}\" ${MEGA_MSVC_VCVARSALL_BAT_ARG}
	cd \"${MEGA_LUAJIT_SOURCE_DIR}/src\"
	if %1 == \"\" shift /1
	msvcbuild.bat %1 %2
")
file(WRITE ${MEGA_LUAJIT_SOURCE_DIR}/megabuild.bat ${MEGA_LUAJIT_BUILD_BAT})
set(MEGA_LUAJIT_LIB ${MEGA_LUAJIT_SOURCE_DIR}/src/lua51.lib)
set(MEGA_LUAJIT_DLL ${MEGA_LUAJIT_SOURCE_DIR}/src/lua51.dll)
ExternalProject_Add(luajit
	SOURCE_DIR ${MEGA_LUAJIT_SOURCE_DIR}
	BUILD_IN_SOURCE 1
	CONFIGURE_COMMAND ""
	BUILD_COMMAND "${MEGA_LUAJIT_SOURCE_DIR}/megabuild.bat" $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:debug> $<$<NOT:$<CONFIG:Debug>>:amalg>
	INSTALL_COMMAND ""
	BUILD_BYPRODUCTS ${MEGA_LUAJIT_LIB} ${MEGA_LUAJIT_DLL} ${MEGA_LUAJIT_SOURCE_DIR}/src/luajit.exe
)
set(MEGA_LUAJIT_INCLUDE ${MEGA_LUAJIT_SOURCE_DIR}/src)

# native.lua
set(NATIVE_DEPENDS "")
if (NOT LUA_EXE)
	set(NATIVE_DEPENDS luajit)
endif()

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/native.lua
	MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/ScriptHookSDK/inc/natives.h ${CMAKE_CURRENT_SOURCE_DIR}/scripts/build_native.lua
	COMMAND "${LUA_EXE}" ARGS "${CMAKE_CURRENT_SOURCE_DIR}/scripts/build_native.lua" "${CMAKE_CURRENT_SOURCE_DIR}/ScriptHookSDK/inc/natives.h" "${CMAKE_CURRENT_BINARY_DIR}/native.lua"
	DEPENDS ${NATIVE_DEPENDS}
	VERBATIM
)

add_custom_target(native SOURCES ${CMAKE_CURRENT_BINARY_DIR}/native.lua)

# LJ.asi
add_library(ljscripthook MODULE src/main.cpp)
add_dependencies(ljscripthook luajit native)
target_link_libraries(ljscripthook ${MEGA_LUAJIT_LIB} ${CMAKE_CURRENT_SOURCE_DIR}/ScriptHookSDK/lib/ScriptHookV.lib)
set_target_properties(ljscripthook PROPERTIES PREFIX "" OUTPUT_NAME "LJ" SUFFIX ".asi")

# install
install(TARGETS ljscripthook DESTINATION .)
install(PROGRAMS ${MEGA_LUAJIT_DLL} DESTINATION .)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/native.lua scripts/main.lua DESTINATION ljscripts)
