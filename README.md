# RacingGameT6

ㅎㅇ

# Set up the SPI controller and gyro sensor
1. Enable SPI0
   arch/arm/boot/dtc/bcm283x.dtsi
   
2. Implement device tree for lis302
   arch/arm/boot/dtc/wt2837.dts
   
3. Set kernel config
   $make menuconfig
   Device Drviers > SPI support > BCM2835 SPI controller
   Device Drivers > STMicroeletronics LIS3LV02Dx three-axis digital accelerometer (SPI)
   
