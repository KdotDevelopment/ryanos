cd ../gnu-efi
make bootloader
cd ../kernel
make kernel
make buildimg
make run
cd bin
qemu-img convert -O vmdk RyanOS.img RyanOS.vmdk