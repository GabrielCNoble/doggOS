#qemu-system-i386 -machine pc -drive file=doggOS.img,format=raw -m 512
qemu-system-i386 -machine pc -drive file=doggOS.img,format=raw -m 512 -enable-kvm -display gtk,full-screen=off -smp cpus=8