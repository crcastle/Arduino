#include <LowPower.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Adjust this value to your board's specific internal BG voltage
// This needs to be accurate to set per board to be able to report current battery voltage
const long InternalReferenceVoltage = 1078;

// This is an updated version of the Tester program that comes with the DallasTemp library
// It will drive a DS18x20 tempurature sensor plugged directly to the Arduino header pins 8,9, and 10.
// The flat side of the sensor should face into the center of the board.
// More info and a video here...
// http://wp.josh.com/2014/06/23/no-external-pull-up-needed-for-ds18b20-temp-sensor/#more-1892


// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 6
#define TEMPERATURE_PRECISION 9

// pin to turn on SIM900
int onModulePin= 2;

// Uncomment this line if you are using the updated dallas_temp_library that
// supports the busFail() method to diagnose bus problems
// #define BUSFAIL

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

void setup(void)
{
  // This lines just make it so you can plug a DS18B20 directly into
  // digitial pins 8-10.

  digitalWrite( 5 , LOW );
  pinMode( 5 , OUTPUT );
  digitalWrite( 7 , LOW );
  pinMode( 7 , OUTPUT );

  pinMode( onModulePin , OUTPUT );
}


void loop(void)
{

  #ifdef BUSFAIL

    Serial.print(" Test:");

    if (sensors.reset()) {

      Serial.print("good");

    } else {

      if (sensors.busFail()) {

        Serial.print("fail");

      } else {

        Serial.print("empty");

      }

    }

  #endif

  powerOn();                    // switch the SIM900 module ON

  int numberOfDevices; // Number of temperature devices found

  DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address

  // For testing purposes, reset the bus every loop so we can see if any devices appear or fall off
  sensors.begin();

  // Grab a count of devices on the wire
  numberOfDevices = sensors.getDeviceCount();

  sensors.requestTemperatures(); // Send the command to get temperatures

  // Loop through each device, send temp data to server
  for(int i=0;i<numberOfDevices; i++)
  {
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i))
    {
      // Output the device ID
      // Serial.print(" #");
      // Serial.print(i,DEC);
      // Serial.print("=");

                // get the temp in F
                float tempF = DallasTemperature::toFahrenheit(sensors.getTempC(tempDeviceAddress));

                // convert temp value float to array of characters
                char temp[10];
                dtostrf(tempF, 4, 2, temp);

                char message[100];
                sprintf(message, "1/home/refrigerator/1/temp_%s", temp);

                // create full AT command
                char atCommand[150];
                sprintf(atCommand, "AT+CUSD=1,\"*126*118*497*%s#\"", message);

                // send AT command
                sendATcommand(atCommand, "OK!", 5000);
    }
  }

  delay(3000); //wait 3 seconds before sending next USSD message

  sendVoltage();

  // Go to sleep for a bit!
  sleepTenMinutes();
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

//******************************************************
// User functions
//******************************************************

// Power on the SIM5218E module
void powerOn(){
  Serial.begin(115200);      // UART baud rate
  Serial.println("Starting...");

  uint8_t answer=0;

  // checks if the module is started
  answer = sendATcommand("AT", "OK", 2000);
  if (answer == 0)
  {
    // power on pulse
    digitalWrite(onModulePin,HIGH);
    delay(3000);
    digitalWrite(onModulePin,LOW);

    // waits for an answer from the module
    while(answer == 0){     // Send AT every two seconds and wait for the answer
      answer = sendATcommand("AT", "OK", 2000);
    }
  }
  delay(5000); // module returns OK a few seconds before it is actually ready
}

// Send AT command to SIM900 module
int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout){

  // digitalWrite(ledPin,HIGH);     // turn the led on while we send data

  uint8_t x=0,  answer=0;
  char response[100];
  unsigned long previous;

  memset(response, '\0', 100);    // Initialize the string

  delay(100);

  while( Serial.available() > 0) Serial.read();    // Clean the input buffer

  Serial.println(ATcommand);    // Send the AT command


  x = 0;
  previous = millis();

  // this loop waits for the answer
  do{
    if(Serial.available() != 0){
      // if there are data in the UART input buffer, reads it and checks for the asnwer
      response[x] = Serial.read();
      x++;
      // check if the desired answer  is in the response of the module
      if (strstr(response, expected_answer) != NULL)
      {
        answer = 1;
      }
    }
   // Waits for the asnwer with time out
  }while((answer == 0) && ((millis() - previous) < timeout));

  // digitalWrite(ledPin,LOW);      // turn the led off

  return answer;
}

int8_t powerOff() {
  uint8_t answer = 0;
  // answer = sendATcommand("AT+CPOF", "OK", 3000);
  answer = sendATcommand("AT+CPOWD=1", "NORMAL POWER DOWN", 10000);
  return answer;
}

void sleepTenMinutes() {
  uint8_t answer = 0;

  answer = powerOff();

  if (answer == 1) {
    for (int i = 0; i < 4; i++) { //loop 75 times for 10 minutes
      LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    }
  } else {
    Serial.println("Error powering down SIM900...");
    while(1); //stop doing everything TODO: blink LED and/or send error to server
  }
}

// From: http://gammon.com.au/power
// Code courtesy of "Coding Badly" and "Retrolefty" from the Arduino forum
// results are Vcc * 100
// So for example, 5V would be 500.
int getBandgap ()
  {
  // REFS0 : Selects AVcc external reference
  // MUX3 MUX2 MUX1 : Selects 1.1V (VBG)
   ADMUX = bit (REFS0) | bit (MUX3) | bit (MUX2) | bit (MUX1);
   ADCSRA |= bit( ADSC );  // start conversion
   while (ADCSRA & bit (ADSC))
     { }  // wait for conversion to complete
   int results = (((InternalReferenceVoltage * 1024) / ADC) + 5) / 10;
   return results;
  } // end of getBandgap

  void sendVoltage() {
    // read current voltage
    char voltage[5];
    itoa(getBandgap(), voltage, 10);

    // create mqtt string
    char message[100];
    sprintf(message, "1/home/refrigerator/1/voltage_%s", voltage);

    // create full AT command
    char atCommand[150];
    sprintf(atCommand, "AT+CUSD=1,\"*126*118*497*%s#\"", message);

    // send AT command
    sendATcommand(atCommand, "OK!", 5000);
  }
