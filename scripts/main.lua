-- Copyright (c) 2022 Miku AuahDark
--
-- This software is provided 'as-is', without any express or implied
-- warranty. In no event will the authors be held liable for any damages
-- arising from the use of this software.
--
-- Permission is granted to anyone to use this software for any purpose,
-- including commercial applications, and to alter it and redistribute it
-- freely, subject to the following restrictions:
--
-- 1. The origin of this software must not be misrepresented; you must not
--    claim that you wrote the original software. If you use this software
--    in a product, an acknowledgment in the product documentation would be
--    appreciated but is not required.
-- 2. Altered source versions must be plainly marked as such, and must not be
--    misrepresented as being the original software.
-- 3. This notice may not be removed or altered from any source distribution.

local ffi = require("ffi")

ffi.cdef[[
int createTexture(const char *texFileName) asm("?createTexture@@YAHPEBD@Z");
void drawTexture(
	int id, int index, int level, int time,
	float sizeX, float sizeY, float centerX, float centerY,
	float posX, float posY,
	float rotation, float screenHeightScaleFactor,
	float r, float g, float b, float a
) asm("?drawTexture@@YAXHHHHMMMMMMMMMMMM@Z");
int getGameVersion() asm("?getGameVersion@@YA?AW4eGameVersion@@XZ");
void nativeInit(uint64_t hash) asm("?nativeInit@@YAX_K@Z");
void nativePush64(uint64_t val) asm("?nativePush64@@YAX_K@Z");
uint64_t *nativeCall() asm("?nativeCall@@YAPEA_KXZ");
]]

dofile("ljscripts/native.lua")
