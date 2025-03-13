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

   
   >Device Drivers > STMicroeletronics LIS3LV02Dx three-axis digital accelerometer (SPI)
   

4. Build

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
#ls /sys/bus/spi/devices/
spi0.0 확인
```
   
