const uint8_t numChannels = 4;

const uint8_t input_pins[numChannels] = {2, 4, 8, 10};
const uint8_t output_pins[numChannels] = {3, 5, 9, 11};

const unsigned long minPulseWidth = 1000;
const unsigned long maxPulseWidth = 2000;

void setup()
{
  for (int i = 0; i < numChannels; ++i)
  {
    pinMode(input_pins[i], INPUT);
    pinMode(output_pins[i], OUTPUT);
  }
}

void loop()
{
  for (int i = 0; i < numChannels; ++i)
  {
    int dutyCycle = map(pulseIn(input_pins[i], HIGH), minPulseWidth, maxPulseWidth, 0, 255);
    dutyCycle = constrain(dutyCycle, 0, 255);
    analogWrite(output_pins[i], dutyCycle);
  }
}