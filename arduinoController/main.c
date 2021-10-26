
#include <Arduino_LSM9DS1.h>

#include <ArduinoBLE.h>

#include <nrfx_pwm.h>

REDIRECT_STDOUT_TO(Serial);

float x;
float y;
float z;

class Output {
  private:
    int pin;
    int old;
  public:
    int value;
    Output(int pin) {
      this->pin = pin;
      old = 0;
      value = 0;
      // pinMode(pin, OUTPUT);
    }

    void Display() {
      /*if(value == 255){
        pinMode(pin, OUTPUT);
        digitalWrite(pin,HIGH);
        }else if(value == 0){
        pinMode(pin, OUTPUT);
        digitalWrite(pin,LOW);
        }else{
        analogWrite(pin, value);
        }
      */

    }

    void Switch() {
      if (value != 0) {
        Off();
      } else {
        On();
      }
    }

    void Off() {
      if (value != 0) {
        old = value;
      }
       value = 0;
    }

    void On() {
      if(value == 0){
        value = old;
        old =0;
      }
    }

    void Debug() {

      printf("[%i]-[%i]-[%i|", value, old, pin);
      printf("%i",digitalPinToPinName(pin));
      printf("]\n");
    }

};

class Color {
  public:
    int R;
    int G;
    int B;
    int W;
    Color(int R, int G, int B, int W = 0) {
      this->R = R;
      this->G = G;
      this->B = B;
      this->W = W;
    }
};
Color Red(255, 0, 0);
Color Blue(0, 0, 255);
Color Green(0, 255, 0);
Color Orange(255, 127, 0);
Color White(0, 0, 0, 255);



int pwmModuleCount = 0;
/* Allocate PWM instances. */
static nrfx_pwm_t nordic_nrf5_pwm_instance[] = {
  NRFX_PWM_INSTANCE(0),
  NRFX_PWM_INSTANCE(1),
  NRFX_PWM_INSTANCE(2),
  NRFX_PWM_INSTANCE(3),
};


class RGBW {
  private:
    Output R;
    Output G;
    Output B;
    Output W;
    int count;
    int brakeCount;


    nrfx_pwm_config_t config;
    int pwmUnit;
    nrf_pwm_sequence_t sequence;
    nrf_pwm_values_individual_t seq_values;
  public:
    bool Blink;
    RGBW(int RPin, int GPin, int BPin, int WPin): R(RPin), G(GPin), B(BPin), W(WPin) {
      count = 0;

      config = NRFX_PWM_DEFAULT_CONFIG;
      config.output_pins[0]  = digitalPinToPinName(RPin);
      config.output_pins[1]  = digitalPinToPinName(GPin);
      config.output_pins[2]  = digitalPinToPinName(BPin);
      config.output_pins[3]  = digitalPinToPinName(WPin);
      config.top_value    = 256;
      // config.load_mode    = NRF_PWM_LOAD_INDIVIDUAL;
      config.irq_priority = PWM_DEFAULT_CONFIG_IRQ_PRIORITY;
      config.base_clock   = NRF_PWM_CLK_1MHz;
      config.count_mode   = NRF_PWM_MODE_UP;

      config.load_mode    = NRF_PWM_LOAD_COMMON;
      config.step_mode    = NRF_PWM_STEP_AUTO;

    }
    
    void SetRGBW(int R, int G, int B, int W, int Intensity = 100) {
      this->R.value = R * Intensity / 100;
      this->G.value = G * Intensity / 100;
      this->B.value = B * Intensity / 100;
      this->W.value = W * Intensity / 100;


      On();

    }

    void SetRGBW(Color color, int intensity = 100) {
      SetRGBW(color.R, color.G, color.B, 0, intensity);
    }

    void On() {
      R.On();
      G.On();
      B.On();
      W.On();
      Display();
    }

    void Off() {
      R.Off();
      G.Off();
      B.Off();
      W.Off();
      Display();
    }

    void Switch() {
      R.Switch();
      G.Switch();
      B.Switch();
      W.Switch();
      Display();
    }

    void Init() {
      pwmUnit = pwmModuleCount++;
      uint32_t err_code = nrfx_pwm_init(&nordic_nrf5_pwm_instance[pwmUnit],
                                        &config,
                                        NULL);
      APP_ERROR_CHECK(err_code);
    }

    void Display() {

      seq_values.channel_0 = R.value;//(255 - R.value) / 4;
      seq_values.channel_1 = G.value;//(255 - R.value) / 4;
      seq_values.channel_2 = B.value;//(255 - R.value) / 4;
      seq_values.channel_3 = W.value;//(255 - R.value) / 4;

      sequence.values.p_individual = &seq_values;
      sequence.length = NRF_PWM_VALUES_LENGTH(seq_values);
      sequence.repeats = 1;
      sequence.end_delay = 0;

      nrfx_pwm_simple_playback(&nordic_nrf5_pwm_instance[pwmUnit], &sequence, 1, NRFX_PWM_FLAG_LOOP);

      // R.Display();
      //G.Display();
      //B.Display();
      //W.Display();
    }
    /*
        void StartBrake(Nano33BLEAccelerometerData accelerometerData, int delay = 10) {
          if (!Blink) {
            brakeCount = 1000 / delay;
            int intensity = (accelerometerData.y * -50) + 50;
            if (intensity > 100) intensity = 100;
            if (intensity < 50) intensity = 50;
            SetRGBW(Red, intensity);
          }
        }*/

    void EndBrake() {
      SetRGBW(Red, 50);
    }

    //Gets called  at each loop turn
    void Refresh(int delay = 10) {
      count++;
      count = count % 1000000;

      if (brakeCount > 0) {
        brakeCount--;
        if (brakeCount == 0) {
          EndBrake();
        }
      }

      if (count % (500 / delay) == 0 && Blink) {
        Switch();
      }

      if (count % (5200 / delay) == 0) {
        Debug();
      }
    }

    void Debug() {

      printf("\n\nCount:[%i]  - Blink: [%s]", count, Blink ? "true" : "false");
      printf("\nR: ");
      R.Debug();
      printf("G: ");
      G.Debug();
      printf("B: ");
      B.Debug();
      printf("W: ");
      W.Debug();


    }

};



BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // BLE LED Service

BLEStringCharacteristic  commandCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite, 5);


RGBW Left(3, 4, 5, 2);
RGBW Right(7, 8, 9, 6);



// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(4800);

  Right.Init();
  Right.SetRGBW(Orange, 100);
  Right.Blink = true;

  Right.Switch();

  Left.Init();
  Left.SetRGBW(Red, 50);
  Left.Blink = true;


  // begin initialization
  if (!BLE.begin()) {
    printf("starting BLE failed!\n");

  }

  // set advertised local name and service UUID:
  BLE.setLocalName("LED");
  BLE.setAdvertisedService(ledService);

  // add the characteristic to the service
  ledService.addCharacteristic(commandCharacteristic);

  // add service
  BLE.addService(ledService);

  // set the initial value for the characeristic:
  commandCharacteristic.writeValue("");

  // start advertising
  BLE.advertise();

  printf("BLE LED Peripheral\n");


  if (!IMU.begin()) {
    printf("Failed to initialize IMU!\n");
  } else {
    printf("initialized IMU!\n");
  }

}

BLEDevice central;
int loopCount = 0;
bool connected = false;
// the loop routine runs over and over again forever:
void loop() {

  loopCount = loopCount % (60 * 100);
  loopCount++;

  if (!central) {
    central = BLE.central();


  }

  if (!connected && central.connected()) {
    printf("Connected\n");
    connected = true;
  }

  if (connected && !central.connected()) {
    printf("DisConnected\n");
    connected = false;
  }


  if (central && central.connected()) {

    if (commandCharacteristic.written()) {
      printf("Command written\n");
      String command = commandCharacteristic.value();

      if (command.startsWith("LEFT")) {
        Left.SetRGBW(Orange);
        Left.Blink = true;
        Right.SetRGBW(Red);
        Right.Blink = false;

      } else if (command.startsWith("RIGHT")) {
        Left.SetRGBW(Red);
        Right.SetRGBW(Orange);
        Right.Blink = true;
      } else if (command.startsWith("STOP")) {
        Left.SetRGBW(Red);
        Right.SetRGBW(Red);
        Right.Blink = false;
      }

    }
  }



  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);


  }

  Right.Refresh();
  Left.Refresh();




  delay(10);
}
