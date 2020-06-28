#define M5STACK_MPU6886   // define must ahead #include <M5Stack.h>
#include <M5Sys.h>
#include <M5Accl.h>
#include <M5AHRS.h>

enum    prog_mode { run_accl, run_ahrs };

M5Sys   m5sys;

M5Accl  accl;
M5AHRS  ahrs;

void setup() {
  m5sys.begin("IMU", NETWORK_CONNECTION_NONE);
  M5.IMU.Init();
}

void loop() {
  VERBOSE("loop()\n");
  prog_mode mode = run_accl;
  while(true) {
    // The two objects, accl and ahrs, display their output interactively on the screen
    // Either can exit the program and return to M5Home, or exit their UI loop and return
    // here. In that case, the intention (as indicated by the UI) is to switch to the
    // alternate object.
    INFO("Display loop. Mode = %s", run_accl == mode ? "Accl" : "AHRS");
    switch(mode) {
      case run_accl:
        mode = run_ahrs;
        accl.show();
        break;
      case run_ahrs:
        mode = run_accl;
        ahrs.show();
        break;
    }
  }
}
