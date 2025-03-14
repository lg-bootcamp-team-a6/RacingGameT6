# RacingGameT6

ㅎㅇ

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

   Porting in board
   U-boot> run IMAGE
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
   
