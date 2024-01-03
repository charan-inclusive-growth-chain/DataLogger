
// uploads the messsge buffer it receives to the server (currrently only serial outputs, modify to upload to server )
#include <WiFi.h>
#include <ArduinoJson.h>

const char* ssid = "charanz";   // your network SSID (name) 
const char* password = "charan123";   // your network password

WiFiClient  client;

unsigned long myChannelNumber = 2326062;
const char * myWriteAPIKey = "WZNVK4JEOYWSODZ1";
// SIM card PIN (leave empty, if not defined)
const char simPIN[]   = "";
String SMS_Message;
// Your phone number to send SMS: + (plus sign) and country code,
//next digit should be your cell number
// SMS_TARGET Example for UK +44XXXXXXXXX
#define SMS_TARGET  "+919346031442"

// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800      // Modem is SIM800
#define TINY_GSM_RX_BUFFER   1024  // Set RX buffer to 1Kb

#include <Wire.h>
#include <TinyGsmClient.h>

// TTGO T-Call pins
#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26
#define I2C_SDA              21
#define I2C_SCL              22

// Set serial for debug console (to Serial Monitor, default speed 115200)
#define SerialMon Serial
// Set serial for AT commands (to SIM800 module)
#define SerialAT  Serial1

// Define the serial console for debug prints, if needed
//#define DUMP_AT_COMMANDS

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

#define IP5306_ADDR          0x75
#define IP5306_REG_SYS_CTL0  0x00

bool setPowerBoostKeepOn(int en) {
  Wire.beginTransmission(IP5306_ADDR);
  Wire.write(IP5306_REG_SYS_CTL0);
  if (en) {
    Wire.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
  } else {
    Wire.write(0x35); // 0x37 is default reg value
  }
  return Wire.endTransmission() == 0;
}
void updateSerial();
void setup()
{
  Serial.begin(115200);
  // Keep power when running from battery

  WiFi.mode(WIFI_STA);   
  


  // Connect or reconnect to WiFi
    if(WiFi.status() != WL_CONNECTED){
      Serial.print("Attempting to connect");
      while(WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid, password); 
        delay(5000);     
      } 
      Serial.println("\nConnected.");
    }

  Wire.begin(I2C_SDA, I2C_SCL);
  bool isOk = setPowerBoostKeepOn(1);
  SerialMon.println(String("IP5306 KeepOn ") + (isOk ? "OK" : "FAIL"));
  // Set modem reset, enable, power pins
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);

  // Set GSM module baud rate and UART pins
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(3000);

  // Restart SIM800 module, it takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println("Initializing modem...");
  modem.restart();
  // use modem.init() if you don't need the complete restart

  // Unlock your SIM card with a PIN if needed
  if (strlen(simPIN) && modem.getSimStatus() != 3 ) {
    modem.simUnlock(simPIN);
  }
  // // To send an SMS, call modem.sendSMS(SMS_TARGET, smsMessage)
  // String smsMessage = "Hello GSM";
  // if (modem.sendSMS(SMS_TARGET, smsMessage)) {
  //   SerialMon.println(smsMessage);
  // }
  // else {
  //   SerialMon.println("SMS failed to send");
  // }
  SerialAT.println("AT"); //Once the handshake test is successful, it will back to OK
  updateSerial();
  delay(200);
  SerialAT.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  delay(200);
  SerialAT.println("AT+CNMI=2,2,0,0,0"); // Decides how newly arrived SMS messages should be handled
  updateSerial();
}

char smsBuffer[250]; // Buffer to store SMS messages


void loop()
{

 // updateSerial();
  String receivedMessage = "STARTING";
  String mymsg = "%%%%%%%%%%%%%%%";
 // updateSerial();
 // static String currentLine = "";
while (SerialAT.available()) 

{
for (int i=0;i<5;i++)
{
  mymsg.concat(SerialAT.readString());
}
mymsg.concat("$$$$$$$$$$$$$$$$$$$$");

Serial.println("printing the message...");
// Use ArduinoJson to stringify mymsg as JSON
  DynamicJsonDocument doc(1024);
  doc["message"] = mymsg;

   // Serialize JSON document to a String
      String jsonString;
      serializeJson(doc, jsonString);

      // Print the JSON string
      Serial.println(jsonString);
      // upload to server

}






  // while (SerialAT.available()) {
  //   char c = SerialAT.read();

  //   if (c == '\n') {
  //     currentLine.trim(); // Remove leading/trailing whitespace
  //     if (currentLine.startsWith("+CMTI: \"SM\",")) {
  //       // Extract SMS index from the line
  //       int smsIndex = currentLine.substring(13).toInt();

  //       // Read the SMS with the extracted index using AT commands
  //       SerialAT.print("AT+CMGR=");
  //       SerialAT.println(smsIndex);
  //       delay(100); // Allow time for response

  //       // Process the SMS message
  //       if (SerialAT.find("+CMGR:")) {
  //         //String receivedMessage = SerialAT.readStringUntil('\n');
  //         receivedMessage = SerialAT.readStringUntil('\n');
  //         receivedMessage.trim();
  //         SerialMon.println("Received SMS: " + receivedMessage);

  //         //----------------------------------------------------------------------------------------------------------------------------------------------------------------
  //           // // To send an SMS, call modem.sendSMS(SMS_TARGET, smsMessage)
  // // String smsMessage = "receivedMessage";
  // // if (modem.sendSMS(SMS_TARGET, smsMessage)) {
  // //   SerialMon.println(smsMessage);
  // // }
  // // else {
  // //   SerialMon.println("SMS failed to send");
  // // }
  // //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  //         // Store the received message in the buffer
  //         receivedMessage.toCharArray(smsBuffer, sizeof(smsBuffer));
  //         SerialMon.println("Stored in buffer: " + String(smsBuffer));

  //         // Delete the SMS after processing
  //         SerialAT.print("AT+CMGD=");
  //         SerialAT.println(smsIndex);
  //         delay(100); // Allow time for response


  //       }


  //     }

  //     currentLine = ""; // Clear the line
  //   } else {
  //     currentLine += c; // Append character to the line
  //   }
  // }
  //int rdn = random(100); // random number trigger event based sms or conditinal sms trigger
  // SMS_Message = "HELLO FROM ESP32";
  // if (rdn == 10) {
  //   if (modem.sendSMS(SMS_TARGET, SMS_Message)) {
  //     SerialMon.println(SMS_Message);
  //   }
  //   else {
  //     SerialMon.println("SMS failed to send");
  //   }
  // }

  delay(1000);
}

void updateSerial()
{
  while (Serial.available())
  {
    SerialAT.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while (SerialAT.available())
  {
    Serial.write(SerialAT.read());//Forward what Software Serial received to Serial Port
  }
}