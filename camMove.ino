#include <FS.h>     //File System [ https://github.com/espressif/arduino-esp32/tree/master/libraries/FS ]
#include <SPIFFS.h> //SPI Flash File System [ https://github.com/espressif/arduino-esp32/tree/master/libraries/SPIFFS ]

#define PIN_MOVE_UP 4
#define PIN_MOVE_DOWN 18
#define PIN_MOVE_LEFT 19
#define PIN_MOVE_RIGHT 21
#define DUTYCYCLE_MAX 255
#define DUTYCYCLE_MIN 150   // 0 - 255 - posição de início do POT
#define PERCENT_POT ( DUTYCYCLE_MAX - DUTYCYCLE_MIN ) / DUTYCYCLE_MAX
#define MAX_ADC 4095
#define RANGE ((MAX_ADC / 2) * 0.2)
#define LOW_RANGE ((MAX_ADC / 2) - RANGE)
#define HIGH_RANGE ((MAX_ADC / 2) + RANGE)
#define FREQ 500
#define RESOLUTION 8

const String PATH_N_FILE_NAME = "/configParam.bin";
const int POT_PIN = 14, PIN_VER_JOYSTICK = 34, PIN_HOR_JOYSTICK = 35;
const int MOVE_UP = 0, MOVE_DOWN = 1, MOVE_LEFT = 2, MOVE_RIGHT = 3;

int dutyCycle = 0, verPositionJoystick = 0, horPositionJoystick = 0, velocity = 0;
bool verParam, horParam;
float percent = 1.0, percentPot;

void setup() {
  initialize();
}

void loop() {
  percentPot = ( analogRead(POT_PIN) / ( MAX_ADC / 1.0 ));
  dutyCycle = (( percentPot * PERCENT_POT * DUTYCYCLE_MAX ) + DUTYCYCLE_MIN );
  verPositionJoystick = analogRead(PIN_VER_JOYSTICK);
  if ( verPositionJoystick < LOW_RANGE || verPositionJoystick > HIGH_RANGE ) {
    if ( verPositionJoystick < LOW_RANGE ) {
      percent = ((LOW_RANGE - verPositionJoystick) / LOW_RANGE);
      velocity = dutyCycle * percent;
      verMove( verParam, velocity );
    } else {
      percent = ((verPositionJoystick - HIGH_RANGE) /  (MAX_ADC - HIGH_RANGE));
      velocity = dutyCycle * percent;
      verMove( !verParam, velocity );
    }
  } else {
    stopMove(MOVE_UP);
    stopMove(MOVE_DOWN);
  }
  horPositionJoystick = analogRead(PIN_HOR_JOYSTICK);
  if ( horPositionJoystick < LOW_RANGE || horPositionJoystick > HIGH_RANGE ) {
    if ( horPositionJoystick < LOW_RANGE ) {
      percent = ((LOW_RANGE - horPositionJoystick) / LOW_RANGE);
      velocity = dutyCycle * percent;
      horMove( horParam, velocity );
    } else {
      percent = ((horPositionJoystick - HIGH_RANGE) /  (MAX_ADC - HIGH_RANGE));
      velocity = dutyCycle * percent;
      horMove( !horParam, velocity );
    }
  } else {
    stopMove(MOVE_LEFT);
    stopMove(MOVE_RIGHT);
  }
}

void initialize(){
  Serial.begin(115200);
  delay(1000);
  const int pins[4] = {PIN_MOVE_UP,PIN_MOVE_DOWN, PIN_MOVE_LEFT, PIN_MOVE_RIGHT};
  for (int i = 0 ; i < (sizeof(pins)/sizeof(int)) ; i++){
    ledcSetup(i, FREQ, RESOLUTION);
    ledcAttachPin(pins[i], i);
    stopMove(pins[i]);
    }
  parameterize();
}

void parameterize(){
  bool adjusted = false;
  verPositionJoystick = analogRead(PIN_VER_JOYSTICK);
  horPositionJoystick = analogRead(PIN_HOR_JOYSTICK);
  if (( verPositionJoystick < LOW_RANGE || verPositionJoystick > HIGH_RANGE ) &&
      ( horPositionJoystick < LOW_RANGE || horPositionJoystick > HIGH_RANGE ))
    {
    if ( verPositionJoystick < LOW_RANGE || verPositionJoystick > HIGH_RANGE ) {
      if ( verPositionJoystick < LOW_RANGE ) {
        verParam = true;
      } else {
        verParam = false;
      }
      adjusted = true;
    }
    if ( horPositionJoystick < LOW_RANGE || horPositionJoystick > HIGH_RANGE ) {
      if ( horPositionJoystick < LOW_RANGE ) {
        horParam = true;
      } else {
        horParam = false;
      }
      adjusted = true;
    }
  }
  saveParam(adjusted, verParam, horParam);
}

void verMove(const bool &up, int &pwmDutyCycle){
  if (up) {
    ledcWrite(MOVE_UP, pwmDutyCycle);
    ledcWrite(MOVE_DOWN, 0);
  } else {
    ledcWrite(MOVE_UP, 0);
    ledcWrite(MOVE_DOWN, pwmDutyCycle );
  }
}

void horMove(const bool &left, int &pwmDutyCycle){
  if (left) {
    ledcWrite(MOVE_LEFT, pwmDutyCycle);
    ledcWrite(MOVE_RIGHT, 0);
  } else {
    ledcWrite(MOVE_LEFT, 0);
    ledcWrite(MOVE_RIGHT, pwmDutyCycle );
  }
}

void stopMove(const int &stopM){
    ledcWrite(stopM, 0);
}

void saveParam(const bool &hasAdj, const bool &verAdj, const bool &horAdj){
  if (hasAdj){
    char fileParam;
    if (hasFile()){
      readFile(hasAdj, verAdj, horAdj);
      return;
    }else{
      if ( verAdj ) {
        bitSet(fileParam, 0);
      }else{
        bitClear(fileParam, 0);
      }
      if ( horAdj ) {
        bitSet(fileParam, 1);
      }else{
        bitClear(fileParam, 1);
      }
    }
    writeFile(fileParam);
  }else{
    readFile(hasAdj, verAdj, horAdj);
  }
}

bool hasFile() {
  const String PATH = "/";
  SPIFFS.begin(true);
  File root = SPIFFS.open(PATH);
  if (!root) {
    Serial.println("- Failed to open directory");
    return false;
  }
  if (!root.isDirectory()) {
    Serial.println("- Not a directory: " + PATH);
    return false;
  }
  File file = root.openNextFile();
    if (file) {
    if ( PATH_N_FILE_NAME == file.name() ){
      return true;
    }else{
      return false;
    }
  }
}

void readFile(const bool &adjParam, const bool &verAdjParam, const bool &horAdjParam) {
  bool fileVerParam, fileHorParam;
  char fileParam = 0x00;
  SPIFFS.begin(true);
  File rFile = SPIFFS.open(PATH_N_FILE_NAME, "rb");
  char values; 
  if (!rFile) {
    Serial.println("- Failed to open file.");
  } else {
    while (rFile.available()) {
      values = rFile.read();
    }
    fileVerParam = bitRead(values, 0);
    fileHorParam = bitRead(values, 1);
  }
  rFile.close();
  if ( adjParam ) {
    
    if ( verAdjParam != fileVerParam || horAdjParam != fileHorParam ){
      if ( verAdjParam != fileVerParam ) {
        if ( verAdjParam ) {
          bitSet(fileParam, 0);
        }
      }else if (fileVerParam) {
         bitSet(fileParam, 0);
      }
      if ( horAdjParam != fileHorParam ) {
        if ( horAdjParam ) {
          bitSet(fileParam, 1);
        }
      }else if (fileHorParam) {
         bitSet(fileParam, 1);
      }
      writeFile(fileParam);
    }
  }else{
    verParam = fileVerParam;
    horParam = fileHorParam;
  }
}

void writeFile(const char &newValue) {
  char *mode = "w+b"; //open for writing (creates file if it doesn't exist). Deletes content and overwrites the file.
  Serial.println("- Writing file: " + PATH_N_FILE_NAME);
  Serial.printf("- newValue: %x\n", newValue);
  SPIFFS.begin(true);
  File wFile = SPIFFS.open(PATH_N_FILE_NAME, mode);
  if (!wFile) {
    Serial.println("- Failed to write file.");
  } else {
    wFile.print(newValue);
    Serial.println("- Written!");
  }
  wFile.close();
}
