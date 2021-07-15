# Pico-USB-audio
This library streams Audio over USB to Raspberry Pi Pico microcontrollers. It then uses a 4th order pulse density modulator to output audio through the Pico's Programmable Input Outputs (PIO). It only requires two resistors and two capacitors to output pretty good audio. This will eventaully transition into an Aruduino library but in the meantime I will attempt to provide cmake files and UF2 files. Check out the video below

<p align="center">

[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/kuy6rja3Azc/0.jpg)](https://www.youtube.com/watch?v=kuy6rja3Azc) 

</p>


<a name="a"></a>
## Building
I'm a Cmake Noob, so apologies in advance if the Cmake stuff doesn't work. If it doesn't please contribute better code. There is a [UF2 file](build) in the build folder if you want to just see what the library can do. Hold down the BOOTSEL button on the PICO, plug in over USB and then drag and drop the UF2 file into the drive that appears. 
Follow this [guide](https://shawnhymel.com/2096/how-to-set-up-raspberry-pi-pico-c-c-toolchain-on-windows-with-vs-code/#Update_Environment_Variables) if you're building on windows. This is currently outlandishly complicated. If you want to help me wrap this into an arduino library please contribute some code. 

<a name="b"></a>
## The Circuit
The Circuit is pretty simple, it is composed of A lowpass filter at bout 20 KHz and a decoupling capactitor to removed the DC offset. The Output voltage is 1.6V peak-peak. This can be made approximately 1.1V peak to peak in software if that is preferable. 

<p align="center">
 <img src="readme/circuit (2).png" width="600" />
</p>

and in reality it looks like this.

<p align="center">
 <img src="readme/realCircuit.jpg" width="600" />
</p>

