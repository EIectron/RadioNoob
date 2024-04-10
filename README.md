<div align="center">
  <b>
   <h1>
      Radionoob AT9
   </h1>
   <h6>
     V1
   </h6>
  </b>

<img src="./Images/Logo/Bitmaps/SplashScreen.jpg">
</div>

## What is this project?
My aim in this project, which I started as a hobby using the box of my broken Radiolink AT9 radio transmitter, was to control the drone in a simple way using the [ExpressLRS](https://github.com/ExpressLRS) module. In addition, I designed equipment that will contribute to my personal development. 
[For hardware information](Hardware.md);


I have personally reached the end of this project that I created with these goals. Even though it is not completely professional, I have created a remote control that will do the job. I made both the hardware and software files open source to guide beginners who want to make their own remote control. Who knows, maybe someone will make the code I wrote more meaningful :)

### This short demonstration [Youtube](https://youtu.be/xDq1djXwKw4)


## Known bugs;
- [ ] DAC player has been disabled because it falling CRSF communication.
- [ ] I cannot refresh much on the main screen, the delay is too much.
- [ ] There is an error in the screen refresh when moving to the upper menu on the ExpressLRS settings page.
- [ ] I never looked at the battery reading section.

### Goals for advanced level?
- [ ] The software must be converted to task structure with FreeRTOS.
- [ ] When I connect the USB, it should offer the option "charging or SIMULATOR" and continue according to my choice. An animation of the battery charging, maybe a current graph, and in the simulator, it goes into HID mode and communicates with the computer.

### Sources that inspired me while doing this project:
- [@danxdz](https://www.github.com/danxdz) https://github.com/danxdz/simpleTx_esp32/tree/master
- [@kkbin505](https://www.github.com/kkbin505) https://github.com/kkbin505/Arduino-Transmitter-for-ELRS
- [@DeviationTX](https://www.github.com/DeviationTX) https://github.com/DeviationTX/deviation
- [@r-u-t-r-A](https://www.github.com/r-u-t-r-A) https://github.com/r-u-t-r-A/STM32-ELRS-Handset/tree/4b6b6f61b5f094afb384d3d6aad8195b43e8a78a
- [@opentx](https://www.github.com/opentx) https://github.com/opentx/opentx/tree/2.3
- [@crsf-wg](https://www.github.com/crsf-wg) https://github.com/crsf-wg/crsf
* https://caolan.uk/notes/2020-10-14_bramley_driving_a_sharp_memory_display_from_rust/