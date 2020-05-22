IF NOT EXIST "V:\win\ProgramFiles\Rockstar Games\Red Dead Redemption 2\" GOTO Check2
copy /Y Playground.asi "V:\win\ProgramFiles\Rockstar Games\Red Dead Redemption 2\Playground.asi"
echo "Copied to installation in V drive"
:Check2
IF NOT EXIST "C:\Program Files\Rockstar Games\Red Dead Redemption 2\" GOTO Check3
copy /Y Playground.asi "C:\Program Files\Rockstar Games\Red Dead Redemption 2\Playground.asi"
echo "Copied to installation in C drive"
:Check3
