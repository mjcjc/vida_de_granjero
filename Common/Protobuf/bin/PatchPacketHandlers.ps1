param(
    [Parameter(Mandatory = $true)]
    [string] $ClientPath,
    [Parameter(Mandatory = $true)]
    [string] $ServerPath
)

$ErrorActionPreference = 'Stop'
$Paths = @($ClientPath, $ServerPath)

$oldReceive = @'
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](session, buffer, len);
'@
$newReceive = @'
		if (buffer == nullptr || len < static_cast<int32>(sizeof(PacketHeader))) return false;
		PacketHeader header{};
		::memcpy(&header, buffer, sizeof(header));
		if (header.size != len) return false;
		return GPacketHandler[header.id](session, buffer, len);
'@
$oldSend = @'
		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		header->size = packetSize;
		header->id = pktId;
		ASSERT_CRASH(pkt.SerializeToArray(&header[1], dataSize));
'@
$newSend = @'
		PacketHeader header{};
		header.size = packetSize;
		header.id = pktId;
		::memcpy(sendBuffer->Buffer(), &header, sizeof(header));
		ASSERT_CRASH(pkt.SerializeToArray(sendBuffer->Buffer() + sizeof(PacketHeader), dataSize));
'@

foreach ($path in $Paths) {
    $content = [System.IO.File]::ReadAllText($path)
    $content = $content -replace '#pragma once\r?\n', "#pragma once`r`n#include `"CorePch.h`"`r`n"
    $content = $content -replace 'PacketHeader\* header = reinterpret_cast<PacketHeader\*>\(buffer\);\s*return GPacketHandler\[header->id\]\(session, buffer, len\);', $newReceive.Trim()
    $content = $content -replace 'PacketHeader\* header = reinterpret_cast<PacketHeader\*>\(sendBuffer->Buffer\(\)\);\s*header->size = packetSize;\s*header->id = pktId;\s*ASSERT_CRASH\(pkt.SerializeToArray\(&header\[1\], dataSize\)\);', $newSend.Trim()
    if ($content.Contains('reinterpret_cast<PacketHeader*>')) {
        throw "Unsafe PacketHeader cast remains in $path"
    }
    [System.IO.File]::WriteAllText($path, $content, [System.Text.Encoding]::ASCII)
}
