class MotorDriver {
  uint8_t voltage_pin = 2; //D2;
  uint8_t ina_pin = 3; //D3;
  uint8_t inb_pin = 4; //D4;
  uint8_t pwm_pin = 5; //D5;

public:
  MotorDriver(){
    pinMode(voltage_pin, OUTPUT);
    pinMode(ina_pin, OUTPUT);
    pinMode(inb_pin, OUTPUT);
    pinMode(pwm_pin, OUTPUT);

    digitalWrite(voltage_pin, HIGH);
    digitalWrite(ina_pin, LOW);
    digitalWrite(inb_pin, LOW);

    analogWrite(pwm_pin, 0);
  }

  void set_normalized_velocity(double normalized_velocity){

    if(normalized_velocity < 0.0){
      digitalWrite(ina_pin, LOW);
      digitalWrite(inb_pin, HIGH);
    } else{
      digitalWrite(ina_pin, HIGH);
      digitalWrite(inb_pin, LOW);
    }

    normalized_velocity = constrain(normalized_velocity, -1.0, 1.0);

    int MULTIPLIER = 1000;
    uint8_t velocity = map(abs(normalized_velocity)*MULTIPLIER, 0, 1.0*MULTIPLIER, 0, 255);
    
    analogWrite(pwm_pin, velocity);
  }
};


class PwmInput {
  uint8_t pwm_input_pin = 6;
  uint8_t ground_pin = 7;

  unsigned long minPulseWidth = 1000;
  unsigned long maxPulseWidth = 2000;

public:
  PwmInput(){
    pinMode(pwm_input_pin, INPUT);
    pinMode(ground_pin, OUTPUT);
    digitalWrite(ground_pin, LOW);
  }

  int readChannel(){
    int ch = pulseIn(pwm_input_pin, HIGH, 30000);

    if (ch < 100) return (maxPulseWidth+minPulseWidth)/2;
    
    return constrain(ch, minPulseWidth, maxPulseWidth);
  }
};


class Blinker {
public:
    uint8_t ledPin;
    unsigned long previousMillis = 0;
    int blinkState = LOW;
    int blinksPerSecond;
    
    Blinker(uint8_t pin) : ledPin(pin) {
        pinMode(ledPin, OUTPUT);
    }

    void update() {
        unsigned long currentMillis = millis();
        unsigned long interval;

        switch (blinksPerSecond) {
            case 1:
                interval = 1000;
                break;
            case 2:
                interval = 250; 
                break;
            case 4:
                interval = 125;
                break;
            default:
                interval = 1000;
        }

        if (currentMillis - previousMillis >= interval) {
            previousMillis = currentMillis;
            blinkState = !blinkState;
            digitalWrite(ledPin, blinkState);
        }
    }
};



MotorDriver motorDriver;
PwmInput pwmInput;
Blinker ledBlinker(LED_BUILTIN);

void setup() {

  // Serial.begin(9600);

  motorDriver = MotorDriver();
  pwmInput = PwmInput();

}

float pwm_to_normalized_velocity(int pulse_in_us){
  return ((pulse_in_us - 1000) / 1000.0) * 2.0 - 1.0;
}

void loop() {
  int input = pwmInput.readChannel();

  float normalized_velocity = pwm_to_normalized_velocity(input);

  // Serial.println(normalized_velocity);

  motorDriver.set_normalized_velocity(normalized_velocity);

  if (normalized_velocity > 0.5) {
    ledBlinker.blinksPerSecond = 4;
  } else if (normalized_velocity < -0.5) {
    ledBlinker.blinksPerSecond = 2;
  } else {
    ledBlinker.blinksPerSecond = 1;
  }

  ledBlinker.update();

}



