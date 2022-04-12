#qemu-system-i386 -machine q35 -drive file=doggOS.img,format=raw -m 512
qemu-system-i386 -machine pc -cpu Penryn-v1 -drive file=doggOS.img,format=raw -m 512 -full-screen