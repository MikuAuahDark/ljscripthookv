-- A simple keyboard handler, copied from Native Trainer source code.
-- I (MikuAuahDark) has no qualify to license this file. Think of this
-- file as supplement file for LJScriptHookV in public domain.

local ffi = require("ffi")

ffi.cdef[[
enum WindowsVirtualKey {
	VK_LBUTTON = 1, LBUTTON = 1, lbutton = 1,
	VK_RBUTTON = 2, RBUTTON = 2, rbutton = 2,
	VK_CANCEL = 3, CANCEL = 3, cancel = 3,
	VK_MBUTTON = 4, MBUTTON = 4, mbutton = 4,
	VK_XBUTTON1 = 5, XBUTTON1 = 5, xbutton1 = 5,
	VK_XBUTTON2 = 6, XBUTTON2 = 6, xbutton2 = 6,
	VK_BACK = 8, BACK = 8, back = 8,
	VK_TAB = 9, TAB = 9, tab = 9,
	VK_CLEAR = 12, CLEAR = 12, clear = 12,
	VK_RETURN = 13, RETURN = 13, return = 13,
	VK_SHIFT = 16, SHIFT = 16, shift = 16,
	VK_CONTROL = 17, CONTROL = 17, control = 17,
	VK_MENU = 18, MENU = 18, menu = 18,
	VK_PAUSE = 19, PAUSE = 19, pause = 19,
	VK_CAPITAL = 20, CAPITAL = 20, capital = 20,
	VK_KANA = 21, KANA = 21, kana = 21,
	VK_HANGEUL = 21, HANGEUL = 21, hangeul = 21,
	VK_HANGUL = 21, HANGUL = 21, hangul = 21,
	VK_IME_ON = 22, IME_ON = 22, ime_on = 22,
	VK_JUNJA = 23, JUNJA = 23, junja = 23,
	VK_FINAL = 24, FINAL = 24, final = 24,
	VK_HANJA = 25, HANJA = 25, hanja = 25,
	VK_KANJI = 25, KANJI = 25, kanji = 25,
	VK_IME_OFF = 26, IME_OFF = 26, ime_off = 26,
	VK_ESCAPE = 27, ESCAPE = 27, escape = 27,
	VK_CONVERT = 28, CONVERT = 28, convert = 28,
	VK_NONCONVERT = 29, NONCONVERT = 29, nonconvert = 29,
	VK_ACCEPT = 30, ACCEPT = 30, accept = 30,
	VK_MODECHANGE = 31, MODECHANGE = 31, modechange = 31,
	VK_SPACE = 32, SPACE = 32, space = 32,
	VK_PRIOR = 33, PRIOR = 33, prior = 33,
	VK_NEXT = 34, NEXT = 34, next = 34,
	VK_END = 35, END = 35, end = 35,
	VK_HOME = 36, HOME = 36, home = 36,
	VK_LEFT = 37, LEFT = 37, left = 37,
	VK_UP = 38, UP = 38, up = 38,
	VK_RIGHT = 39, RIGHT = 39, right = 39,
	VK_DOWN = 40, DOWN = 40, down = 40,
	VK_SELECT = 41, SELECT = 41, select = 41,
	VK_PRINT = 42, PRINT = 42, print = 42,
	VK_EXECUTE = 43, EXECUTE = 43, execute = 43,
	VK_SNAPSHOT = 44, SNAPSHOT = 44, snapshot = 44,
	VK_INSERT = 45, INSERT = 45, insert = 45,
	VK_DELETE = 46, DELETE = 46, delete = 46,
	VK_HELP = 47, HELP = 47, help = 47,
	VK_LWIN = 91, LWIN = 91, lwin = 91,
	VK_RWIN = 92, RWIN = 92, rwin = 92,
	VK_APPS = 93, APPS = 93, apps = 93,
	VK_SLEEP = 95, SLEEP = 95, sleep = 95,
	VK_NUMPAD0 = 96, NUMPAD0 = 96, numpad0 = 96,
	VK_NUMPAD1 = 97, NUMPAD1 = 97, numpad1 = 97,
	VK_NUMPAD2 = 98, NUMPAD2 = 98, numpad2 = 98,
	VK_NUMPAD3 = 99, NUMPAD3 = 99, numpad3 = 99,
	VK_NUMPAD4 = 100, NUMPAD4 = 100, numpad4 = 100,
	VK_NUMPAD5 = 101, NUMPAD5 = 101, numpad5 = 101,
	VK_NUMPAD6 = 102, NUMPAD6 = 102, numpad6 = 102,
	VK_NUMPAD7 = 103, NUMPAD7 = 103, numpad7 = 103,
	VK_NUMPAD8 = 104, NUMPAD8 = 104, numpad8 = 104,
	VK_NUMPAD9 = 105, NUMPAD9 = 105, numpad9 = 105,
	VK_MULTIPLY = 106, MULTIPLY = 106, multiply = 106,
	VK_ADD = 107, ADD = 107, add = 107,
	VK_SEPARATOR = 108, SEPARATOR = 108, separator = 108,
	VK_SUBTRACT = 109, SUBTRACT = 109, subtract = 109,
	VK_DECIMAL = 110, DECIMAL = 110, decimal = 110,
	VK_DIVIDE = 111, DIVIDE = 111, divide = 111,
	VK_F1 = 112, F1 = 112, f1 = 112,
	VK_F2 = 113, F2 = 113, f2 = 113,
	VK_F3 = 114, F3 = 114, f3 = 114,
	VK_F4 = 115, F4 = 115, f4 = 115,
	VK_F5 = 116, F5 = 116, f5 = 116,
	VK_F6 = 117, F6 = 117, f6 = 117,
	VK_F7 = 118, F7 = 118, f7 = 118,
	VK_F8 = 119, F8 = 119, f8 = 119,
	VK_F9 = 120, F9 = 120, f9 = 120,
	VK_F10 = 121, F10 = 121, f10 = 121,
	VK_F11 = 122, F11 = 122, f11 = 122,
	VK_F12 = 123, F12 = 123, f12 = 123,
	VK_F13 = 124, F13 = 124, f13 = 124,
	VK_F14 = 125, F14 = 125, f14 = 125,
	VK_F15 = 126, F15 = 126, f15 = 126,
	VK_F16 = 127, F16 = 127, f16 = 127,
	VK_F17 = 128, F17 = 128, f17 = 128,
	VK_F18 = 129, F18 = 129, f18 = 129,
	VK_F19 = 130, F19 = 130, f19 = 130,
	VK_F20 = 131, F20 = 131, f20 = 131,
	VK_F21 = 132, F21 = 132, f21 = 132,
	VK_F22 = 133, F22 = 133, f22 = 133,
	VK_F23 = 134, F23 = 134, f23 = 134,
	VK_F24 = 135, F24 = 135, f24 = 135,
	VK_NAVIGATION_VIEW = 136, NAVIGATION_VIEW = 136, navigation_view = 136,
	VK_NAVIGATION_MENU = 137, NAVIGATION_MENU = 137, navigation_menu = 137,
	VK_NAVIGATION_UP = 138, NAVIGATION_UP = 138, navigation_up = 138,
	VK_NAVIGATION_DOWN = 139, NAVIGATION_DOWN = 139, navigation_down = 139,
	VK_NAVIGATION_LEFT = 140, NAVIGATION_LEFT = 140, navigation_left = 140,
	VK_NAVIGATION_RIGHT = 141, NAVIGATION_RIGHT = 141, navigation_right = 141,
	VK_NAVIGATION_ACCEPT = 142, NAVIGATION_ACCEPT = 142, navigation_accept = 142,
	VK_NAVIGATION_CANCEL = 143, NAVIGATION_CANCEL = 143, navigation_cancel = 143,
	VK_NUMLOCK = 144, NUMLOCK = 144, numlock = 144,
	VK_SCROLL = 145, SCROLL = 145, scroll = 145,
	VK_OEM_NEC_EQUAL = 146, OEM_NEC_EQUAL = 146, oem_nec_equal = 146,
	VK_OEM_FJ_JISHO = 146, OEM_FJ_JISHO = 146, oem_fj_jisho = 146,
	VK_OEM_FJ_MASSHOU = 147, OEM_FJ_MASSHOU = 147, oem_fj_masshou = 147,
	VK_OEM_FJ_TOUROKU = 148, OEM_FJ_TOUROKU = 148, oem_fj_touroku = 148,
	VK_OEM_FJ_LOYA = 149, OEM_FJ_LOYA = 149, oem_fj_loya = 149,
	VK_OEM_FJ_ROYA = 150, OEM_FJ_ROYA = 150, oem_fj_roya = 150,
	VK_LSHIFT = 160, LSHIFT = 160, lshift = 160,
	VK_RSHIFT = 161, RSHIFT = 161, rshift = 161,
	VK_LCONTROL = 162, LCONTROL = 162, lcontrol = 162,
	VK_RCONTROL = 163, RCONTROL = 163, rcontrol = 163,
	VK_LMENU = 164, LMENU = 164, lmenu = 164,
	VK_RMENU = 165, RMENU = 165, rmenu = 165,
	VK_BROWSER_BACK = 166, BROWSER_BACK = 166, browser_back = 166,
	VK_BROWSER_FORWARD = 167, BROWSER_FORWARD = 167, browser_forward = 167,
	VK_BROWSER_REFRESH = 168, BROWSER_REFRESH = 168, browser_refresh = 168,
	VK_BROWSER_STOP = 169, BROWSER_STOP = 169, browser_stop = 169,
	VK_BROWSER_SEARCH = 170, BROWSER_SEARCH = 170, browser_search = 170,
	VK_BROWSER_FAVORITES = 171, BROWSER_FAVORITES = 171, browser_favorites = 171,
	VK_BROWSER_HOME = 172, BROWSER_HOME = 172, browser_home = 172,
	VK_VOLUME_MUTE = 173, VOLUME_MUTE = 173, volume_mute = 173,
	VK_VOLUME_DOWN = 174, VOLUME_DOWN = 174, volume_down = 174,
	VK_VOLUME_UP = 175, VOLUME_UP = 175, volume_up = 175,
	VK_MEDIA_NEXT_TRACK = 176, MEDIA_NEXT_TRACK = 176, media_next_track = 176,
	VK_MEDIA_PREV_TRACK = 177, MEDIA_PREV_TRACK = 177, media_prev_track = 177,
	VK_MEDIA_STOP = 178, MEDIA_STOP = 178, media_stop = 178,
	VK_MEDIA_PLAY_PAUSE = 179, MEDIA_PLAY_PAUSE = 179, media_play_pause = 179,
	VK_LAUNCH_MAIL = 180, LAUNCH_MAIL = 180, launch_mail = 180,
	VK_LAUNCH_MEDIA_SELECT = 181, LAUNCH_MEDIA_SELECT = 181, launch_media_select = 181,
	VK_LAUNCH_APP1 = 182, LAUNCH_APP1 = 182, launch_app1 = 182,
	VK_LAUNCH_APP2 = 183, LAUNCH_APP2 = 183, launch_app2 = 183,
	VK_OEM_1 = 186, OEM_1 = 186, oem_1 = 186,
	VK_OEM_PLUS = 187, OEM_PLUS = 187, oem_plus = 187,
	VK_OEM_COMMA = 188, OEM_COMMA = 188, oem_comma = 188,
	VK_OEM_MINUS = 189, OEM_MINUS = 189, oem_minus = 189,
	VK_OEM_PERIOD = 190, OEM_PERIOD = 190, oem_period = 190,
	VK_OEM_2 = 191, OEM_2 = 191, oem_2 = 191,
	VK_OEM_3 = 192, OEM_3 = 192, oem_3 = 192,
	VK_GAMEPAD_A = 195, GAMEPAD_A = 195, gamepad_a = 195,
	VK_GAMEPAD_B = 196, GAMEPAD_B = 196, gamepad_b = 196,
	VK_GAMEPAD_X = 197, GAMEPAD_X = 197, gamepad_x = 197,
	VK_GAMEPAD_Y = 198, GAMEPAD_Y = 198, gamepad_y = 198,
	VK_GAMEPAD_RIGHT_SHOULDER = 199, GAMEPAD_RIGHT_SHOULDER = 199, gamepad_right_shoulder = 199,
	VK_GAMEPAD_LEFT_SHOULDER = 200, GAMEPAD_LEFT_SHOULDER = 200, gamepad_left_shoulder = 200,
	VK_GAMEPAD_LEFT_TRIGGER = 201, GAMEPAD_LEFT_TRIGGER = 201, gamepad_left_trigger = 201,
	VK_GAMEPAD_RIGHT_TRIGGER = 202, GAMEPAD_RIGHT_TRIGGER = 202, gamepad_right_trigger = 202,
	VK_GAMEPAD_DPAD_UP = 203, GAMEPAD_DPAD_UP = 203, gamepad_dpad_up = 203,
	VK_GAMEPAD_DPAD_DOWN = 204, GAMEPAD_DPAD_DOWN = 204, gamepad_dpad_down = 204,
	VK_GAMEPAD_DPAD_LEFT = 205, GAMEPAD_DPAD_LEFT = 205, gamepad_dpad_left = 205,
	VK_GAMEPAD_DPAD_RIGHT = 206, GAMEPAD_DPAD_RIGHT = 206, gamepad_dpad_right = 206,
	VK_GAMEPAD_MENU = 207, GAMEPAD_MENU = 207, gamepad_menu = 207,
	VK_GAMEPAD_VIEW = 208, GAMEPAD_VIEW = 208, gamepad_view = 208,
	VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON = 209, GAMEPAD_LEFT_THUMBSTICK_BUTTON = 209, gamepad_left_thumbstick_button = 209,
	VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON = 210, GAMEPAD_RIGHT_THUMBSTICK_BUTTON = 210, gamepad_right_thumbstick_button = 210,
	VK_GAMEPAD_LEFT_THUMBSTICK_UP = 211, GAMEPAD_LEFT_THUMBSTICK_UP = 211, gamepad_left_thumbstick_up = 211,
	VK_GAMEPAD_LEFT_THUMBSTICK_DOWN = 212, GAMEPAD_LEFT_THUMBSTICK_DOWN = 212, gamepad_left_thumbstick_down = 212,
	VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT = 213, GAMEPAD_LEFT_THUMBSTICK_RIGHT = 213, gamepad_left_thumbstick_right = 213,
	VK_GAMEPAD_LEFT_THUMBSTICK_LEFT = 214, GAMEPAD_LEFT_THUMBSTICK_LEFT = 214, gamepad_left_thumbstick_left = 214,
	VK_GAMEPAD_RIGHT_THUMBSTICK_UP = 215, GAMEPAD_RIGHT_THUMBSTICK_UP = 215, gamepad_right_thumbstick_up = 215,
	VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN = 216, GAMEPAD_RIGHT_THUMBSTICK_DOWN = 216, gamepad_right_thumbstick_down = 216,
	VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT = 217, GAMEPAD_RIGHT_THUMBSTICK_RIGHT = 217, gamepad_right_thumbstick_right = 217,
	VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT = 218, GAMEPAD_RIGHT_THUMBSTICK_LEFT = 218, gamepad_right_thumbstick_left = 218,
	VK_OEM_4 = 219, OEM_4 = 219, oem_4 = 219,
	VK_OEM_5 = 220, OEM_5 = 220, oem_5 = 220,
	VK_OEM_6 = 221, OEM_6 = 221, oem_6 = 221,
	VK_OEM_7 = 222, OEM_7 = 222, oem_7 = 222,
	VK_OEM_8 = 223, OEM_8 = 223, oem_8 = 223,
	VK_OEM_AX = 225, OEM_AX = 225, oem_ax = 225,
	VK_OEM_102 = 226, OEM_102 = 226, oem_102 = 226,
	VK_ICO_HELP = 227, ICO_HELP = 227, ico_help = 227,
	VK_ICO_00 = 228, ICO_00 = 228, ico_00 = 228,
	VK_PROCESSKEY = 229, PROCESSKEY = 229, processkey = 229,
	VK_ICO_CLEAR = 230, ICO_CLEAR = 230, ico_clear = 230,
	VK_PACKET = 231, PACKET = 231, packet = 231,
	VK_OEM_RESET = 233, OEM_RESET = 233, oem_reset = 233,
	VK_OEM_JUMP = 234, OEM_JUMP = 234, oem_jump = 234,
	VK_OEM_PA1 = 235, OEM_PA1 = 235, oem_pa1 = 235,
	VK_OEM_PA2 = 236, OEM_PA2 = 236, oem_pa2 = 236,
	VK_OEM_PA3 = 237, OEM_PA3 = 237, oem_pa3 = 237,
	VK_OEM_WSCTRL = 238, OEM_WSCTRL = 238, oem_wsctrl = 238,
	VK_OEM_CUSEL = 239, OEM_CUSEL = 239, oem_cusel = 239,
	VK_OEM_ATTN = 240, OEM_ATTN = 240, oem_attn = 240,
	VK_OEM_FINISH = 241, OEM_FINISH = 241, oem_finish = 241,
	VK_OEM_COPY = 242, OEM_COPY = 242, oem_copy = 242,
	VK_OEM_AUTO = 243, OEM_AUTO = 243, oem_auto = 243,
	VK_OEM_ENLW = 244, OEM_ENLW = 244, oem_enlw = 244,
	VK_OEM_BACKTAB = 245, OEM_BACKTAB = 245, oem_backtab = 245,
	VK_ATTN = 246, ATTN = 246, attn = 246,
	VK_CRSEL = 247, CRSEL = 247, crsel = 247,
	VK_EXSEL = 248, EXSEL = 248, exsel = 248,
	VK_EREOF = 249, EREOF = 249, ereof = 249,
	VK_PLAY = 250, PLAY = 250, play = 250,
	VK_ZOOM = 251, ZOOM = 251, zoom = 251,
	VK_NONAME = 252, NONAME = 252, noname = 252,
	VK_PA1 = 253, PA1 = 253, pa1 = 253,
	VK_OEM_CLEAR = 254, OEM_CLEAR = 254, oem_clear = 254
};

uint64_t GetTickCount64();
]]

local keyboard = {}
local keyStates = {}
local union = ffi.new("enum WindowsVirtualKey[1]")
local NOW_PERIOD = 100 -- ms
local MAX_DOWN = 5000  -- ms

function toVirtualKey(name)
	union[0] = name
	return tonumber(union[0])
end

function keyboardHandler(key, repeats, scanCode, isExtended, isWithAlt, wasDownBefore, isUpNow)
	if key < 255 then
		local k = keyStates[key + 1]
		k.time = ffi.C.GetTickCount64()
		k.isWithAlt = isWithAlt
		k.wasDownBefore = wasDownBefore
		k.isUpNow = isUpNow
	end
end

function isKeyDown(key)
	if type(key) == "number" and key < 0 and key >= 255 then
		return false
	elseif type(key) == "string" then
		key = toVirtualKey(key)
	end

	local k = keyStates[key + 1]
	return ffi.C.GetTickCount64() < (k.time + MAX_DOWN) and (not k.isUpNow)
end

function isKeyJustUp(key, exclusive)
	if type(key) == "number" and key < 0 and key >= 255 then
		return false
	elseif type(key) == "string" then
		key = toVirtualKey(key)
	end

	if exclusive == nil then
		exclusive = true
	end

	local k = keyStates[key + 1]
	local result = ffi.C.GetTickCount64() < (k.time + NOW_PERIOD) and k.isUpNow

	if exclusive then
		resetKeyState(key)
	end

	return result
end

function resetKeyState(key)
	if type(key) == "number" and key < 0 and key >= 255 then
		return
	elseif type(key) == "string" then
		key = toVirtualKey(key)
	end

	local k = keyStates[key + 1]
	k.time = 0ULL
	k.isWithAlt = false
	k.wasDownBefore = false
	k.isUpNow = false
end

for i = 1, 255 do
	keyStates[i] = {}
	resetKeyState(i - 1)
end

keyboard.states = keyStates
keyboard.callback = keyboardHandler
keyboard.isKeyDown = isKeyDown
keyboard.isKeyJustUp = isKeyJustUp
keyboard.resetKeyState = resetKeyState

return keyboard
