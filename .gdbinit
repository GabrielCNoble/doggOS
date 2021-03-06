set auto-load safe-path /

define hook-stop
    printf "[%4x:%4x]", $cs, $eip
    x/i $cs*16+$eip
end

set disassembly-flavor intel
layout asm
layout reg
target remote localhost:26000
set architecture i8086
b *(0x7c00)