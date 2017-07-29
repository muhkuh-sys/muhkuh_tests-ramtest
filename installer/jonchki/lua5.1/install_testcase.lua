local t, uiId, strName = ...
local tResult

local pl = t.pl
local tLogger = t.cLogger

local strID = 'org.muhkuh.tests.ramtest.lua5.1-ramtest'
tLogger:debug('Installing test case %s with ID %02d and name "%s".', strID, uiId, strName)

-- Load the "test.lua" script.
local strTestTemplatePath = pl.path.join(t.strCwd, 'templates', 'test.lua')
local strTestTemplate, strError = pl.utils.readfile(strTestTemplatePath, false)
if strTestTemplate==nil then
  tLogger:error('Failed to open the test template "%s": %s', strTestTemplatePath, strError)
else
  local astrReplace = {
    ['ID'] = string.format('%02d', uiId),
    ['NAME'] = strName
  }
  local strTestLua = string.gsub(strTestTemplate, '@([^@]+)@', astrReplace)

  -- Write the test script to the installation base directory.
  local strDestinationPath = t:replace_template(string.format('${install_base}/test%02d.lua', uiId))
  local tFileResult, strError = pl.utils.writefile(strDestinationPath, strTestLua, false)
  if tFileResult~=true then
    tLogger:error('Failed to write the test to "%s": %s', strDestinationPath, strError)
  else
    tResult = true
  end
end

return tResult