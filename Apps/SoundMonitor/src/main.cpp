
#include <arduinoFFT.h>
#include <M5Sys.h>

M5SysBase   m5sys;
arduinoFFT  FFT     = arduinoFFT();

#define MIC_PIN             34
#define SCALE               512
#define SAMPLES             1024    // Must be a power of 2
#define SAMPLING_FREQUENCY  80000
//// Determines maximum frequency that can be analysed by the FFT Fmax=sampleF/2.

struct eqBand {
  const char*   freqname;
  uint16_t      amplitude;
  int           peak;
  int           lastpeak;
  uint16_t      lastval;
  unsigned long lastmeasured;
};


eqBand audiospectrum[8] = {
  //Adjust the amplitude values to fit your microphone
  // Lower values increase sensitivity to the freq
  // Can be used to compensate for ambient noise in certain situations
  { "63Hz",   50, 0, 0, 0, 0 },
  { "160Hz",  40, 0, 0, 0, 0 },
  { "400Hz",  40, 0, 0, 0, 0 },
  { "1KHz",   40, 0, 0, 0, 0 },
  { "2.5KHz", 25, 0, 0, 0, 0 },
  { "6.2KHz", 25, 0, 0, 0, 0 },
  { "10KHz",  65, 0, 0, 0, 0 },
  { "20KHz",  55, 0, 0, 0, 0 }
};


unsigned int  sampling_period_us;
unsigned long microseconds;
double        vReal[SAMPLES];
double        vImag[SAMPLES];
unsigned long newTime;
unsigned long oldTime;
uint16_t      tft_width   = 320; // ILI9341_TFTWIDTH;
uint16_t      tft_height  = 240; // ILI9341_TFTHEIGHT;
uint8_t       bands = 8;
uint8_t       bands_width = floor( tft_width / bands );
uint8_t       bands_pad   = bands_width - 10;
uint16_t      colormap[255];    // color palette for the band meter (pre-fill in setup)


void setup() {
  m5sys.begin("SoundMonitor", "None");
  dacWrite(25, 0); // Speaker OFF
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextColor(BLUE, BLACK);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setRotation(1);

  // ADC setup
  pinMode(MIC_PIN, INPUT);        // Pin 34 is the M5Stack Fire mic input
  analogReadResolution(9);        // Resolution of 0-511
  analogSetWidth(9);              // Resolution of 0-511
  analogSetCycles(4);             // Number of cycles per sample.  Function of the SAR ADC on the ESP32.  Increases accuracy at cost of performance
  analogSetSamples(4);            // Number of samples for the result.  Function of the SAR ADC on the ESP32.  Increases accuracy at cost of performance.
  analogSetClockDiv(1);           // Clock divider for timing
  analogSetAttenuation(ADC_0db);  // ADC Gain.   Valid:  0, 2_5, 6, 11
  adcAttachPin(MIC_PIN);          // Set the pin your microphone is attached to.

  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
  for(uint8_t i=0;i<tft_height;i++) {
    colormap[i] = M5.Lcd.color565(tft_height-i, tft_height+i, i);
  }
  for (byte band = 0; band <= 7; band++) {
    M5.Lcd.setCursor(bands_width*band + 2, 0);
    M5.Lcd.print(audiospectrum[band].freqname);
  }
}


// Graphics for the Equalizer
void displayBand(int band, int dsize){
  uint16_t hpos = bands_width*band;
  int dmax = 200;
  if(dsize>tft_height-10) {
    dsize = tft_height-10; // leave some hspace for text
  }
  if(dsize < audiospectrum[band].lastval) {
    // lower value, delete some lines
    M5.Lcd.fillRect(hpos, tft_height-audiospectrum[band].lastval,
                    bands_pad, audiospectrum[band].lastval - dsize, BLACK);
  }
  if (dsize > dmax) dsize = dmax;
  for (int s = 0; s <= dsize; s=s+4){
    M5.Lcd.drawFastHLine(hpos, tft_height-s, bands_pad, colormap[tft_height-s]);
  }
  if (dsize > audiospectrum[band].peak) {
    audiospectrum[band].peak = dsize;
  }
  audiospectrum[band].lastval = dsize;
  audiospectrum[band].lastmeasured = millis();
}


// Associates the frequency to the appropriate band being displayed.
// If you modify the resolution of the ADC, be sure to update these appropriately.
// These were loosley calibrated using a tone generator.
byte getBand(int i) {
  if (i<=4 )             return 0;  // 125Hz
  if (i >6   && i<=10  ) return 1;  // 250Hz
  if (i >10  && i<=14  ) return 2;  // 500Hz
  if (i >14  && i<=30  ) return 3;  // 1000Hz
  if (i >30  && i<=60  ) return 4;  // 2000Hz
  if (i >60  && i<=106 ) return 5;  // 4000Hz
  if (i >106 && i<=400 ) return 6;  // 8000Hz
  if (i >400           ) return 7;  // 16000Hz
  return 8;
}


void loop() {
  for (int i = 0; i < SAMPLES; i++) {
    adcStart(MIC_PIN);
    newTime   = micros() - oldTime;
    oldTime   = newTime;
    vReal[i]  = adcEnd(MIC_PIN);
    vImag[i]  = 0;
  }
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

  for(int i = 2; i < (SAMPLES/2); i++){
    // Each array element represents a frequency and its value the amplitude.
    if (vReal[i] > 500) { // Add a crude noise filter
      byte bandNum = getBand(i);
      if(bandNum != 8) {
        displayBand(bandNum, (int)vReal[i]/audiospectrum[bandNum].amplitude);
      }
    }
  }
  long vnow = millis();
  for(byte band = 0; band <= 7; band++) {
    // auto decay every 135ms on low activity bands
    if(vnow - audiospectrum[band].lastmeasured > 135) {
      displayBand(band, audiospectrum[band].lastval>4 ? audiospectrum[band].lastval-4 : 0);
    }
    if (audiospectrum[band].peak > 0) {
      audiospectrum[band].peak -= 2;
      if(audiospectrum[band].peak <= 0) {
        audiospectrum[band].peak = 0;
      }
    }
    // only draw if peak changed
    if(audiospectrum[band].lastpeak != audiospectrum[band].peak) {
    // delete last peak
      M5.Lcd.drawFastHLine(bands_width*band,tft_height-audiospectrum[band].lastpeak,bands_pad,BLACK);
      audiospectrum[band].lastpeak = audiospectrum[band].peak;
      M5.Lcd.drawFastHLine(bands_width*band, tft_height-audiospectrum[band].peak,bands_pad, colormap[tft_height-audiospectrum[band].peak]);
    }
  }
}
