local ffi = require("ffi")

ffi.cdef[[
int createTexture(const char *texFileName);
void drawTexture(
	int id, int index, int level, int time,
	float sizeX, float sizeY, float centerX, float centerY,
	float posX, float posY,
	float rotation, float screenHeightScaleFactor,
	float r, float g, float b, float a
);
void nativeInit(uint64_t hash);
void nativePush64(uint64_t val);
uint64_t *nativeCall();
]]

dofile("ljscripts/native.lua")
