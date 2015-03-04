-- 28.07.14 SL try all combinations of sdclk_phase and data_sample_phase
-- 03.03.15 SL adapt to new API taking a netx type and no extra ulTiming value

-- NXHX51-ETM Rev.2, netX51 Step A
--                  sample phase
-- clock phase     0  1  2  3  4  5
--         0       0  0  0  0  0  0
--         1       0  0  0  0  0  0
--         2       0  0  0  0  0  1
--         3       0  0  0  0  1  1
--         4       0  0  0  1  1  1
--         5       0  0  1  1  1  1
--

require("muhkuh_cli_init")
require("ramtest")


tPlugin = tester.getCommonPlugin()
if tPlugin==nil then
	error("No plug-in selected, nothing to do!")
end


${SDRAM_ATTRIBUTES}


ramtest.test_phase_parameters(tPlugin, atSdramAttributes, ramtest.SDRAM_INTERFACE_MEM, 2)

-- Disconnect the plug-in.
tPlugin:Disconnect()
