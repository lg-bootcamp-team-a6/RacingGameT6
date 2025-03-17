# RacingGameT6

# Precondition

타겟 보드에 audio/ 하위 리소스를 copy해 주세요.
```
cp -r {레포 경로}/audio/*.wav /nfsroot
```

# Set up the ALSA driver settings

부팅 직후 아래 커맨드 실행 필요 (부팅 후 최초 1회만)
혹은 /etc/profile 하에 아래 커맨드 추가하여 RFS 재빌드 필요

```
source /mnt/nfs/alsa.sh
```

# Set up the SPI controller and gyro sensor
1. Enable SPI0


   ```$cp device_tree/bcm283x.dtsi ~/work/linux/arch/arm/boot/dts/bcm283x.dtsi```
   
2. Implement device tree for lis302


   ```$cp device_tree/wt2837.dts ~/work/linux/arch/arm/boot/dts/wt2837.dts```
   
3. Set kernel config
 -> Make config status "*" : included in kernel
   
   ```$make menuconfig```
   
   >Device Drviers > SPI support > BCM2835 SPI controller

   
   >Device Drivers > Misc devices > STMicroeletronics LIS3LV02Dx three-axis digital accelerometer (SPI)
   

   (SPI)를 꼭 확인할 것!
   

5. Build

  ```
   Linux kernel build
   $make Image
   $make dtbs
   $./copy_images.sh

   Porting in board
   U-boot> run Image
   U-boot> run DTB
   U-boot> boot
```

5. Check
  ```
#cd /sys/class/input/event0
#cat device/name
ST LIS3LV02DL Accelerometer 확인
```

6. Result
```
/dev/input/event0 : ST LIS3LV02DL Accelerometer
/dev/input/event1 : ByQDtech (touch screen)
/dev/input/event2 : bcm2837-gpio-keys (key devices)
```
   
