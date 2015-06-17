
int8_t answer;
const int onModulePin = 2;      // the pin to switch on the module (without press on button)

const int button1Pin = 3;        // the number of the pushbutton1 pin
const int button2Pin = 4;        // the number of the pushbutton2 pin
const int button3Pin = 5;        // the number of the pushbutton3 pin
const int button4Pin = 6;        // the number of the pushbutton4 pin
const int button5Pin = 7;        // the number of the pushbutton5 pin

const int ledPin = 8;            // the number of the led pin

int button1State = 0;            // variable for reading the pushbutton1 status
int button2State = 0;            // variable for reading the pushbutton2 status
int button3State = 0;            // variable for reading the pushbutton3 status
int button4State = 0;            // variable for reading the pushbutton4 status
int button5State = 0;            // variable for reading the pushbutton5 status

void setup() {
  pinMode(onModulePin, OUTPUT); // initialize the onModulePin as output
  pinMode(button1Pin, INPUT);   // initialize the pushbutton1 pin as an input
  pinMode(button2Pin, INPUT);   // initialize the pushbutton2 pin as an input
  pinMode(button3Pin, INPUT);   // initialize the pushbutton3 pin as an input
  pinMode(button4Pin, INPUT);   // initialize the pushbutton4 pin as an input
  pinMode(button5Pin, INPUT);   // initialize the pushbutton5 pin as an input
  pinMode(ledPin, OUTPUT);      // initialize the led pin as output

  powerOn();                    // switches the SIM5218 module ON
  
  // sendATcommand("AT+COPS=?", "OK", 2000);
}




void loop() {
  // read the state of the pushbutton value:
  button1State = digitalRead(button1Pin);
  button2State = digitalRead(button2Pin);
  button3State = digitalRead(button3Pin);
  button4State = digitalRead(button4Pin);
  button5State = digitalRead(button5Pin);

  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:
  // send data
  if (button1State == HIGH) {
    sendATcommand("AT+CUSD=1,\"*126*118*497*23#\"", "Thx", 5000);
  } else if (button2State == HIGH) {
    sendATcommand("AT+CUSD=1,\"*126*118*497*37#\"", "Thx", 5000);
  } else if (button3State == HIGH) {
    sendATcommand("AT+CUSD=1,\"*126*118*497*53#\"", "Thx", 5000);
  } else if (button4State == HIGH) {
    sendATcommand("AT+CUSD=1,\"*126*118*497*67#\"", "Thx", 5000);
  } else if (button5State == HIGH) {
    sendATcommand("AT+CUSD=1,\"*126*118*497*71#\"", "Thx", 5000);
  }
  // delay(1000);
  // sendATcommand("AT+CPOF", "OK", 2000);
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

// Send AT command to SIM5218E module
int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout){

  digitalWrite(ledPin,HIGH);     // turn the led on while we send data  

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

  digitalWrite(ledPin,LOW);      // turn the led off

  return answer;
}

int8_t powerOff() {
  uint8_t answer = 0;
  answer = sendATcommand("AT+CPOF", "OK", 2000);
  return answer;
}
