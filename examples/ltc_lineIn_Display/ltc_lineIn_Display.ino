/* Linear Timecode for Audio Library for Teensy 3.x / 4.x

 ltc_lineIn_Display Example

   Copyright (c) 2019, Frank Bösing, f.boesing (at) gmx.de
   Copyright (c) 2024, Michael Utz, mail (at) utzs.ch

   Development of this audio library was funded by PJRC.COM, LLC by sales of
   Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
   open source software by purchasing Teensy or other PJRC products.

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice, development funding notice, and this permission
   notice shall be included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

/*

 https://forum.pjrc.com/threads/41584-Audio-Library-for-Linear-Timecode-(LTC)

 LTC example audio at: https://www.youtube.com/watch?v=uzje8fDyrgg

Extended Version with USB Input and Analog Line Input, 7 Segment Display based on MAX7219 Chip, And Second-Pulse Output on PIN 13
The LTC TimeCode is only decoded on the Left Channels!
The USB and Analog Line In is mixed together and routed to the Line Out and Headphone Jack on the Teensy Audio Board.
Make shure you choose USB Type: Serial + MIDI + Audio, for compiling
 
*/

#include <Audio.h>
#include <analyze_ltc.h>
#include <elapsedMillis.h>
#include "LedControl.h"

// GUItool: begin automatically generated code
AudioInputUSB            usb1;           //xy=320,184
AudioInputI2S            i2s2;           //xy=326,238
AudioMixer4              mixer1;         //xy=506,198
AudioMixer4              mixer2;         //xy=506,272
AudioOutputI2S           i2s1;           //xy=698,195
AudioAnalyzeLTC          ltc1;           //xy=556,396
AudioControlSGTL5000     sgtl5000_1;     //xy=302,184
AudioConnection          patchCord1(usb1, 0, mixer1, 0);
AudioConnection          patchCord2(usb1, 1, mixer2, 0);
AudioConnection          patchCord3(i2s2, 0, mixer1, 1);
AudioConnection          patchCord4(i2s2, 1, mixer2, 1);
AudioConnection          patchCord5(mixer1, 0, i2s1, 0);
AudioConnection          patchCord6(mixer1, 0, ltc1, 0);
AudioConnection          patchCord7(mixer2, 0, i2s1, 1);
// GUItool: end automatically generated code

ltcframe_t ltcframe;

// define variables and set Display Controls
LedControl lc=LedControl(10,11,12,1);

// store the digit value for the 7 Segment display
volatile int hourDigit0 = 0;
volatile int hourDigit1 = 0;
volatile int minuteDigit0 = 0;
volatile int minuteDigit1 = 0;
volatile int secondDigit0 = 0;
volatile int secondDigit1 = 0;
volatile int frameDigit0 = 0;
volatile int frameDigit1 = 0;

// store the last LTC Second
volatile int lastLtcSecond = 0;

//define the minute pulse lenght
unsigned int minutePulseLenght = 500;

// timer for minute pulse out
elapsedMillis minutePulseTimer;

void setup() {
  // Setup the Audio Board
  AudioMemory(12);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.6);
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
  sgtl5000_1.lineOutLevel(20,20);
  sgtl5000_1.lineInLevel(2,2);
  mixer1.gain(0,1);
  mixer1.gain(1,1);
  mixer1.gain(2,1);
  mixer1.gain(3,1);
  mixer2.gain(0,1);
  mixer2.gain(1,1);
  mixer2.gain(2,1);
  mixer2.gain(3,1);

  //Configure Pins
  pinMode (13, OUTPUT);

  //Wake Up the Display at boot
  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,8);
  /* and clear the display */
  lc.clearDisplay(0);

  // Display that the Device is Ready
  lc.setChar(0,0,'0',false);
  lc.setChar(0,1,'l',false);
  lc.setChar(0,2,'l',false);
  lc.setChar(0,3,'e',false);
  lc.setChar(0,4,'H',false);
  delay(2000);
  lc.setChar(0,0,'p',false);
  lc.setChar(0,1,'f',false);
  lc.setChar(0,2,'0',false);
  lc.setChar(0,3,'3',false);
  lc.setChar(0,4,' ',false);
  lc.setChar(0,5,'C',false);
  lc.setChar(0,6,'-',false);
  lc.setChar(0,7,'L',false);
  delay(2000);
  lc.setChar(0,0,'0',false);
  lc.setChar(0,1,'0',false);
  lc.setChar(0,2,'0',true);
  lc.setChar(0,3,'0',false);
  lc.setChar(0,4,'0',true);
  lc.setChar(0,5,'0',false);
  lc.setChar(0,6,'0',true);
  lc.setChar(0,7,'0',false);

  // Turn Run LED on
  digitalWrite(13, HIGH);
}

// Get single digits from ltc Value for the 7 Segment Display
void displayCalc(){
  frameDigit0 = ltc1.frame(&ltcframe)% 10;
  frameDigit1 = (ltc1.frame(&ltcframe)/10) % 10;
  secondDigit0 = ltc1.second(&ltcframe)% 10;
  secondDigit1 = (ltc1.second(&ltcframe)/10) % 10;
  minuteDigit0 = ltc1.minute(&ltcframe)% 10;
  minuteDigit1 = (ltc1.minute(&ltcframe)/10) % 10;
  hourDigit0 = ltc1.hour(&ltcframe)% 10;
  hourDigit1 = (ltc1.hour(&ltcframe)/10) % 10;
}

void displaySet(){
  lc.setChar(0,0,frameDigit0, false);
  lc.setChar(0,1,frameDigit1, false);
  lc.setChar(0,2,secondDigit0, true);
  lc.setChar(0,3,secondDigit1, false);
  lc.setChar(0,4,minuteDigit0, true);
  lc.setChar(0,5,minuteDigit1, false);
  lc.setChar(0,6,hourDigit0, true);
  lc.setChar(0,7,hourDigit1, false);
}

void minutePulseOut(){
  if (ltc1.second(&ltcframe) != lastLtcSecond) {
    lastLtcSecond = ltc1.second(&ltcframe);
    // debug Serial.print("lastltcSecond ");
    // debug Serial.println(lastLtcSecond);
    digitalWrite(13, HIGH);
    minutePulseTimer = 0;
  }

  if (digitalRead(13) == HIGH && minutePulseTimer > minutePulseLenght) {
    digitalWrite(13,LOW);
  }
}

void loop() {
  if (ltc1.available()) {
    ltcframe = ltc1.read();
    Serial.printf("%02d:%02d:%02d.%02d\n", ltc1.hour(&ltcframe), ltc1.minute(&ltcframe), ltc1.second(&ltcframe), ltc1.frame(&ltcframe));
    displayCalc();
    displaySet();
    minutePulseOut();
  }
}
