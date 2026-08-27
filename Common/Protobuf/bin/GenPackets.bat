@echo off
setlocal
pushd "%~dp0"

protoc.exe -I=./ --cpp_out=./ ./Enum.proto
if errorlevel 1 goto :fail
protoc.exe -I=./ --cpp_out=./ ./Struct.proto
if errorlevel 1 goto :fail
protoc.exe -I=./ --cpp_out=./ ./Protocol.proto
if errorlevel 1 goto :fail

GenPackets.exe --path=./Protocol.proto --output=ClientPacketHandler --recv=C_ --send=S_
if errorlevel 1 goto :fail
GenPackets.exe --path=./Protocol.proto --output=ServerPacketHandler --recv=S_ --send=C_
if errorlevel 1 goto :fail

powershell.exe -NoProfile -ExecutionPolicy Bypass -File "PatchPacketHandlers.ps1" -ClientPath "ClientPacketHandler.h" -ServerPath "ServerPacketHandler.h"
if errorlevel 1 goto :fail

copy /Y "Enum.pb.h" "../../../GameServer/Enum.pb.h" >nul
if errorlevel 1 goto :fail
copy /Y "Enum.pb.cc" "../../../GameServer/Enum.pb.cc" >nul
if errorlevel 1 goto :fail
copy /Y "Struct.pb.h" "../../../GameServer/Struct.pb.h" >nul
if errorlevel 1 goto :fail
copy /Y "Struct.pb.cc" "../../../GameServer/Struct.pb.cc" >nul
if errorlevel 1 goto :fail
copy /Y "Protocol.pb.h" "../../../GameServer/Protocol.pb.h" >nul
if errorlevel 1 goto :fail
copy /Y "Protocol.pb.cc" "../../../GameServer/Protocol.pb.cc" >nul
if errorlevel 1 goto :fail
copy /Y "ClientPacketHandler.h" "../../../GameServer/ClientPacketHandler.h" >nul
if errorlevel 1 goto :fail

copy /Y "Enum.pb.h" "../../../NewClient/Protocol/Enum.pb.h" >nul
if errorlevel 1 goto :fail
copy /Y "Enum.pb.cc" "../../../NewClient/Protocol/Enum.pb.cc" >nul
if errorlevel 1 goto :fail
copy /Y "Struct.pb.h" "../../../NewClient/Protocol/Struct.pb.h" >nul
if errorlevel 1 goto :fail
copy /Y "Struct.pb.cc" "../../../NewClient/Protocol/Struct.pb.cc" >nul
if errorlevel 1 goto :fail
copy /Y "Protocol.pb.h" "../../../NewClient/Protocol/Protocol.pb.h" >nul
if errorlevel 1 goto :fail
copy /Y "Protocol.pb.cc" "../../../NewClient/Protocol/Protocol.pb.cc" >nul
if errorlevel 1 goto :fail
copy /Y "ServerPacketHandler.h" "../../../NewClient/Protocol/ServerPacketHandler.h" >nul
if errorlevel 1 goto :fail

del /Q /F "Enum.pb.h" "Enum.pb.cc"
del /Q /F "Struct.pb.h" "Struct.pb.cc"
del /Q /F "Protocol.pb.h" "Protocol.pb.cc"
del /Q /F "ClientPacketHandler.h" "ServerPacketHandler.h"

echo [Protocol] Generation completed successfully.
popd
endlocal
exit /B 0

:fail
echo [Protocol] Generation failed. Temporary files were preserved.
popd
endlocal
exit /B 1
