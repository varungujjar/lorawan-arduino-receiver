#include <SPI.h>
#include <Wire.h>
#include <LoRa.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <avr/wdt.h>


//OLED Configurations
#define SCREEN_WIDTH    128 // OLED display width, in pixels
#define SCREEN_HEIGHT   32 // OLED display height, in pixels
#define OLED_RESET      -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
static unsigned long previousMillisDisplay = 0;
boolean enableDisplay = false;


//Lora Config
#define csPin 10
#define resetPin 9
#define irqPin 2  

String outgoing;              // outgoing message
byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xBB;     // address of this device
byte destination = 0xFF;      // destination to send to
long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends


//Vibration Config
boolean vibrate = false;
boolean enable = false;
int vibrateTrackCount = 0;

int vibrateCount = 3;
int vibrateDuration = 500;
int vibrateInterval = 200;



//Splash Screen Config
boolean showSplash = true;
static unsigned long splashInterval = 2000;
unsigned long currentMillisSplash = millis();



// String message = "";
static unsigned long previousMillis = 0;
static unsigned long previousMillisInterval = 0;
unsigned long currentMillis = millis();








void displayText(String text, int cursorX, int cursorY, int fontSize){
  display.clearDisplay();
  display.setTextSize(fontSize);
  display.setTextColor(WHITE);
  display.setCursor(cursorX, cursorY);
  display.print(text);
  display.display();
}

void enableVibrate(){
  enable = true;
  vibrateTrackCount = 0;
  previousMillis = millis();
};

void disableVibrate(){
   if(vibrateTrackCount == (vibrateCount+1)){
        vibrateTrackCount = 0;
        vibrate = false;
        enable = false;
    }
}

void doVibrate(){
  if(showSplash) return;
  currentMillis = millis();

  if (enable && vibrate && (currentMillis - previousMillis >= vibrateInterval))
  {
    previousMillisInterval = currentMillis;
    digitalWrite(5, HIGH);
    vibrate = false;
  }

  if (enable && !vibrate && (currentMillis - previousMillisInterval >= vibrateDuration))
  {
    previousMillis = currentMillis;
    digitalWrite(5, LOW);
    vibrate = true;
    vibrateTrackCount++;
  }
  disableVibrate();
}


void splashScreen(){
  if(showSplash){
    displayText("Ronin Labs",33, 15, 1);
  }

  currentMillisSplash = millis();
  if (showSplash && (currentMillisSplash >= splashInterval))
  {
    displayText("...",55, 15, 1);
    showSplash = false;
  }
}


void setup() {
  Serial.begin(9600); 
  while (!Serial);

  // wdt_enable(WDTO_2S);
  LoRa.setPins(csPin, resetPin, irqPin);

  //Initialize display by providing the display type and its I2C address.
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  Serial.println("OLED Display init succeeded");
 
  if (!LoRa.begin(434E6)) {             // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (1);                       // if failed, do nothing
  }

  // LoRa.enableInvertIQ();
  // LoRa.setSPI(SPI);
  // LoRa.setSPIFrequency(4e6);
  // LoRa.setSpreadingFactor(10);
  // LoRa.setSignalBandwidth(42.5E3);
  // LoRa.crc();
  LoRa.setSpreadingFactor(12); //7 For Low Power
  LoRa.setSignalBandwidth(250000); //500000 For Low Power
  LoRa.setPreambleLength(8);
  LoRa.setCodingRate4(8); //5 For Low Power
  LoRa.enableCrc();
  Serial.println("LoRa init succeeded.");

  //Setup Vibration PIN
  pinMode(5, OUTPUT);
  digitalWrite(5, LOW);
  Serial.println("Vibration init succeeded");
}


void onReceive(int packetSize) {
  if(showSplash) return;
  if (packetSize == 0) return;          // if there's no packet, return

  // Requires first byte to be a string and so we discard it
  LoRa.read();

  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address

  vibrateCount = LoRa.read(); 
  vibrateDuration = LoRa.read()*100; 
  vibrateInterval = LoRa.read()*100; 


  byte incomingLength = LoRa.read();    // incoming msg length
  String incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
    //  uint8_t input = (uint8_t)LoRa.read();
    //  Serial.print(input);
    //  Serial.print("(0x");
    //  Serial.print(input, HEX);
    //  Serial.print(")");
  }

  if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("error: message length does not match length");
    return;  
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return;                            
  }

  if(incoming.length()==incomingLength){
    displayText(incoming,0, 1, 2);
    enableVibrate();
  }

  // if message is for this device, or broadcast, print details:
  Serial.println("\n\n");
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + incoming);
  Serial.println("Vibrate Count: " + String(vibrateCount));
  Serial.println("Vibrate Duration: " + String(vibrateDuration));
  Serial.println("Vibrate Interval: " + String(vibrateInterval));
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println("\n\n");
}



void loop(){
  // LoRa.idle();
  // LoRa.sleep();
  // wdt_reset();
    splashScreen();
    onReceive(LoRa.parsePacket());
    doVibrate();
}

