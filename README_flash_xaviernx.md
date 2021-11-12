Basic flashing info
```console
foo@bar:~$ sudo -s
root@bar:~# tar -xf nvidia/L4T/32.5.1/xavier/tegra186_linux_r32.5.1_aarch64.tbz2
root@bar:~# cd rootfs
root@bar:~# tar -xf nvidia/L4T/32.5.1/Tegra_Linux_Sample-Root-Filesystem_R32.5.1_aarch64.tbz2 
cd ..
root@bar:~# ./apply_binaries.sh
root@bar:~# ./flash.sh jetson-xavier-nx-devkit mmcblk0p1
```
