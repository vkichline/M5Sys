#include <M5Sys.h>
#include <atomic>

#define MICROPHONE            34
#define SPEAKER               25
#define BACKLIGHT             32

#define BUFFER_SIZE           ESP.getPsramSize() / 3
#define SAMPLING_FREQUENCY    22050

// #define LOWNOISE           true /* set to false to enable backlight dimming */

M5Sys                         m5sys;
TFT_eSPI                      tft                 = TFT_eSPI();
static hw_timer_t *           sampleTimer         = NULL;
int16_t*                      sampleBuffer;
std::atomic<std::uint32_t>    currentSample{0};   // https://baptiste-wicht.com/posts/2012/07/c11-concurrency-tutorial-part-4-atomic-type.html
unsigned int                  sampling_period_us  = round( 1000000 * ( 1.0 / SAMPLING_FREQUENCY ) );


//this ISR should run on core 0
static void IRAM_ATTR _sampleISR() {
  sampleBuffer[currentSample.load()] = analogRead( MICROPHONE );
  ++currentSample;
  if ( currentSample.load() == BUFFER_SIZE ) {
    timerEnd( sampleTimer );
    sampleTimer = NULL;
  }
}


bool startSampler() {
  VERBOSE("startSampler()\n");
  if ( NULL != sampleTimer ) return false;
  delay(170); /* to not record the click from releasing the button */
  currentSample.store( 0, std::memory_order_relaxed );
  sampleTimer = timerBegin( 3, 80, true );
  timerAttachInterrupt( sampleTimer, &_sampleISR, true );
  timerAlarmWrite( sampleTimer, sampling_period_us, true );
  timerAlarmEnable( sampleTimer );
  return true;
}


void IRAM_ATTR _playThroughDAC_ISR() {
  dacWrite( SPEAKER, map( sampleBuffer[currentSample.load()], 0, 2048, 0, 128 ) );
  ++currentSample;
  if ( currentSample.load() == BUFFER_SIZE ) {
    timerEnd( sampleTimer );
    sampleTimer = NULL;
    dacWrite( SPEAKER, 0 );
  }
}


bool startPlayback() {
  VERBOSE("startPlayback\n");
  if ( NULL != sampleTimer ) return false;
  currentSample.store( 0, std::memory_order_relaxed );
  sampleTimer = timerBegin( 0, 80, true );
  timerAttachInterrupt( sampleTimer, &_playThroughDAC_ISR, true );
  timerAlarmWrite( sampleTimer, sampling_period_us, true );
  timerAlarmEnable( sampleTimer );
  return true;
}


void setup() {
  VERBOSE("setup()\n");
  m5sys.begin("SoundRecorder", NETWORK_CONNECTION_NONE);
  m5.Lcd.setRotation(1);
  pinMode( MICROPHONE, INPUT );
  pinMode( SPEAKER, OUTPUT );
  DEBUG("Used PSRAM: %d from: %d", ESP.getPsramSize() - ESP.getFreePsram(), ESP.getPsramSize() );
  sampleBuffer = (int16_t*)ps_malloc( BUFFER_SIZE * sizeof( int16_t ) );
  // if(LOWNOISE) {
  //   pinMode( BACKLIGHT, OUTPUT );
  //   digitalWrite( BACKLIGHT, HIGH ); // This gives the least noise
  // }
  // else {
  //   ledcAttachPin( BACKLIGHT, 0);
  //   ledcSetup( 0, 1300, 16 );
  //   ledcWrite( 0, 0xFFFF / 16 ); // Dimming the BACKLIGHT will produce more base noise
  // }  
}


// Periodic (10 mS) event to sample sound
//
uint16_t sample() {
  uint32_t pos = currentSample.load();
  ez.canvas.font(sans26);
  ez.canvas.x(30);
  ez.canvas.y(160);
  ez.canvas.printf("%3i%% %7i/%7i   ", map( pos, 0, BUFFER_SIZE - 1, 0, 100 ), pos, BUFFER_SIZE);
  return 10;
}


void loop() {
  VERBOSE("loop()\n");
  ez.header.show("Sound Recorder");
  ez.buttons.show("Rec # Home # Play ## Stop #");
  ez.canvas.clear();
  ez.canvas.font(&FreeSans12pt7b);
  ez.canvas.lmargin(25);
  ez.canvas.y(50);
  ez.canvas.println("        Sound Recorder");
  ez.canvas.y(90);
  ez.canvas.printf("%3.1fkHz, %3.1f kB, %3.1fs", SAMPLING_FREQUENCY / 1000.0, ( BUFFER_SIZE * sizeof( int16_t ) ) / 1000.0, (float)BUFFER_SIZE / SAMPLING_FREQUENCY );
  
  while(true) {
    String result = ez.buttons.wait();
    INFO("result = %s\n", result.c_str());
    if(     0 == result.compareTo("Home"))  m5sys.goHome();
    else if(0 == result.compareTo("Rec"))   { ez.addEvent(sample); startSampler(); }
    else if(0 == result.compareTo("Play"))  startPlayback();
    else if(0 == result.compareTo("Stop")) {
      timerAlarmDisable( sampleTimer );
      timerEnd( sampleTimer );
      sampleTimer = NULL;
      dacWrite(SPEAKER, 0);
      ez.removeEvent(sample);
    }
  }
}
