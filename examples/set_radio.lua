-- This is an example LJScriptHookV script. It sets the vehicle radio
-- to Self Radio when player presses "\|" (US keyboard).
-- This code is in public domain.

-- By default, "require" looks at "ljscripts/libs" folder, so this just works ;)
-- See ljscripts/libs/keyboard.lua for more information about the script it loads.
local keyboard = require("keyboard")

-- This is our update loop function.
local function update()
	-- This function checks if "\|" (US keyboard) button is pressed.
	if keyboard.isKeyJustUp("oem_5") then
		-- Get "Ped" object.
		local player = PLAYER.PLAYER_PED_ID()

		-- Check if "Ped" is in any vehicle.
		if PED.IS_PED_IN_ANY_VEHICLE(player, false) then
			-- Get "Vehicle" object of "Ped"
			local veh = PED.GET_VEHICLE_PED_IS_USING(player);

			-- This checks if the vehicle is valid
			if veh.id ~= 0 then
				-- This set the vehicle radio station to Self Radio
				AUDIO.SET_VEH_RADIO_STATION(veh, "RADIO_19_USER");
			end
		end
	end
end

-- The script should return a table with these fields:
return {
	-- Update function which is called every game script loop. (REQUIRED)
	update = update,
	-- Keyboard handler function, which is called on every game input. (OPTIONAL)
	keyboardHandler = keyboard.callback
}
