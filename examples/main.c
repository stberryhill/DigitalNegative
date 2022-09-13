#define DIGITAL_NEGATIVE_IMPLEMENTATION
#include "../src/DigitalNegative.h"

int main() {
  DigitalNegative negative;
  DigitalNegative_Decode("../res/adobe.DNG", &negative);

  return 0;
}