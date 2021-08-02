# Pico-USB-audio
This library streams Audio over USB to Raspberry Pi Pico microcontrollers. It then uses a 4th order pulse density modulator to output audio through the Pico's Programmable Input Outputs (PIO). It only requires two resistors and two capacitors to output pretty good audio. Check out the video below. 


<p align="center">

[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/kuy6rja3Azc/0.jpg)](https://www.youtube.com/watch?v=kuy6rja3Azc) 

</p>

## Table of contents
0. [Preface](#pref)
1. [Arduino Code](#a)
2. [The Circuit](#b)
3. [Building for pico-sdk](#c)

<a name="pref"></a>
## Preface
There are a couple of things to be aware of when using this library. 
1. It overclocks (230.4 Mhz for 64 times oversampling) and underclocks (115.2 MHz for 32 X oversampling) the Pico.
2. Sigma delta modulaters can get unstable and produce unpleasant sounds. 
3. I won't guarantee this code won't blow up your favourite speakers or headphones (it shouldn't) but no guarantees.

<a name="a"></a>
## Arduino Mbed Code
An Arduino compatible USB sound card version is available. This code uses the official Arduino RP2040 core. This greatly simplifes the install and development process. The necessary mbed files can be easily installed with the Arduino board manager. Search "mbed rp2040" and install. Then run the code in the [mbedUSB folder](mbedUSB/mbedUSB.ino) or install the [Arduino library](SDM) in your Arduino libraries folder. Then run the [mbedUSB example](https://github.com/tierneytim/Pico-USB-audio/tree/main/SDM/examples/mbedUSB)  for 32 times oversampling and the [mbedUSB64 example](https://github.com/tierneytim/Pico-USB-audio/tree/main/SDM/examples/mbedUSB64)
for 64 times oversampling.
<p align="center">
 <img src="README/board managerSearch.PNG" width="600" />
</p

<a name="b"></a>
## The Circuit
The Circuit is pretty simple, it is composed of A lowpass filter at bout 20 KHz and a decoupling capactitor to removed the DC offset. The Output voltage is 1.6V peak-peak. This can be made approximately 1.1V peak to peak in software if that is preferable. 

<p align="center">
 <img src="README/circuit (2).png" width="600" />
</p>

and in reality it looks like this.

<p align="center">
 <img src="README/realCircuit.jpg" width="600" />
</p>


<a name="c"></a>
# Building for pico-sdk
I'm a Cmake Noob, so apologies in advance if the Cmake stuff doesn't work. If it doesn't please contribute better code. There is a [UF2 file](build) in the build folder if you want to just see what the library can do. Hold down the BOOTSEL button on the PICO, plug in over USB and then drag and drop the UF2 file into the drive that appears. 
Follow this [guide](https://shawnhymel.com/2096/how-to-set-up-raspberry-pi-pico-c-c-toolchain-on-windows-with-vs-code/#Update_Environment_Variables) if you're building on windows. This is currently outlandishly complicated. If you want to help me simplify then please contribute.
