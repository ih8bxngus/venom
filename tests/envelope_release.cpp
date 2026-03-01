#include <cmath>
#include <iostream>

#include "../src/dsp/EnvelopeADSR.hpp"

int main() {
  venom::dsp::EnvelopeADSR env;
  env.setSampleRate(48000.0);
  env.setAttack(0.005f);
  env.setDecay(0.05f);
  env.setSustain(0.3f);
  env.setRelease(0.2f);
  env.setRetrigger(true);

  env.noteOn();
  float level = 0.0f;
  for (int i = 0; i < 4800; ++i)
    level = env.process();

  if (level < 0.2f) {
    std::cerr << "envelope never reached expected level before release\n";
    return 1;
  }

  env.noteOff();
  float prev = level;
  bool reachedZero = false;
  for (int i = 0; i < 20000; ++i) {
    const float v = env.process();
    if (v > prev + 1.0e-5f) {
      std::cerr << "release increased unexpectedly\n";
      return 2;
    }
    prev = v;
    if (v <= 0.0f) {
      reachedZero = true;
      break;
    }
  }

  if (!reachedZero) {
    std::cerr << "release did not reach zero\n";
    return 3;
  }

  std::cout << "envelope release test passed\n";
  return 0;
}
