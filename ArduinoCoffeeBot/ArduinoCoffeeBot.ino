// SPI Support
#include <SPI.h>
// Ethernet Shield Sup]port
#include <Dhcp.h>
#include <Dns.h>
#include <EthernetClient.h>
#include <EthernetUdp.h>
#include <EthernetServer.h>
#include <util.h>
#include <Ethernet.h>
// Twitter Client Support
// Uses Twitter proxy http://arduino-tweet.appspot.com/
#include <Twitter.h>

// Device Settings
byte id = 0x00; // 0x00 = Reserved for prototype #1
char location[] = "1-218D";

// Serial Port 0 Settings
long baud = 9600;

// Ethernet Settings
byte mac[] = { 0xC0, 0xFF, 0xEE, 0xBA, 0xD0, id };
bool ethernet_up = false;

// Twitter Settings
Twitter twitter("123456789-abcdefghijklmnopqrstuvwxyz");
unsigned long tweet_delay_ms = 1*60*1000;
char message[139] = { '\0' };

// Push Button Settings
const int BUTTON = 22;

// Pressure Sensor Settings
const int PRESSURE = 8; // A8
const int EMPTY = 10; // Calibrated value of empty
const int FULL = 50; // Calibrated value of full

bool check_button() {
  if (digitalRead(BUTTON) == HIGH) 
    return true;
  else 
    return false; 
}

/*
 * Detect that the coffee pot is full by looking for a stable rate of change.
 */
bool check_pressure() {
  static int full_count = 0;
  static int previous = 0;
  int current = analogRead(PRESSURE);
  int difference = current - previous;

  // debug monitor
  #if 1
  Serial.print("Pressure ");
  Serial.print(previous);
  Serial.print(" -> ");
  Serial.print(current);
  Serial.print(" difference ");
  Serial.print(difference);
  Serial.print(" count ");
  Serial.println(full_count);
  #endif 
  
  previous = current;
  
  if (current < EMPTY) {
    full_count = 0;
    return false;
  }

  if (abs(difference) > 10) {
    full_count = 0;
    return false;
  }
  
  if (current < FULL) {
    full_count = 0;
    return false; 
  }
    
  if (full_count++ < 5) {
    return false;
  }
  
  full_count = 0;
  return true;
}

void create_tweet(char msg[]) {
  static int previous_tweet = 0;

  // TODO detect night and reset to 0

  // Must use a different message each time because Twitter will block duplicate posts.  
  switch (previous_tweet++) {
    case (0):
      sprintf(msg, "CoffeeBot 0x%02X: First brew of the day ready in #%s.", id, location);
      break;
    case (1):
      sprintf(msg, "CoffeeBot 0x%02X: Coffee is ready in #%s.", id, location); 
      break;   
    case (2):
      sprintf(msg, "CoffeeBot 0x%02X, Coffee in #%s is ready.", id, location);
      break;
    default:
      sprintf(msg, "CoffeeBot 0x%02X, Coffee in #%s! Come and get it!", id, location);
      previous_tweet = 1;
  }
  
  // do not exceed 140 characters
  msg[139] = '\0';
}

// Send message to Twitter. Don't send if minimum delay has not been met.
// Return True if message was sent successfully.
bool tweet(char msg[]) {
  static unsigned long next_tweet_ms = 0;
  bool sent = false;
  
  Serial.println(next_tweet_ms);
  
  if (millis() < next_tweet_ms) {
    Serial.print("Tweets blocked for ");
    Serial.print((next_tweet_ms - millis())/1000);
    Serial.println(" seconds.");  
    return false;
  }
  
  Serial.println("connecting ...");
  if (twitter.post(msg)) {
    int status = twitter.wait(&Serial);
    if (status == 200) {
      Serial.println("OK.");
      sent = true;
    } else {
      Serial.print("failed : code ");
      Serial.println(status);
    }
  } else {
    Serial.println("connection failed.");
  }
  
  if (sent || !ethernet_up)
    next_tweet_ms = millis() + tweet_delay_ms; // handles rollover
  
  return sent;
}

void setup() {
  // Configure Serial Port Monitor
  Serial.begin(baud);
  Serial.print("Serial: ");
  Serial.print(baud);
  Serial.println(" bps");
  
  // Configure Ethernet for DHCP
  #if 0
  Serial.println("Connecting to network...");
  if (Ethernet.begin(mac) == 0)
    Serial.println("Ethernet DCHP configuration failed");  
  else {
    Serial.print("IP: ");
    Serial.println(Ethernet.localIP());
    ethernet_up = true;
  }
  #endif
  
  // Configure Push Button
  pinMode(BUTTON, INPUT);
  
  // Configure Pressure Sensor
  /* analog inputs are inputs by default
  pinMode(PRESSURE, INPUT);
  */
}

void loop() {
  if (Serial.available() > 0) {
    Serial.read(); // remove byte from Rx FIFO
    Serial.print("Hi, I am Arduino CoffeeBot #");
    Serial.println(id);
    Serial.print("IP: ");
    Serial.println(Ethernet.localIP());
  }
  
  // Check inputs
  if (check_button() || check_pressure()) {
    // Report status
    Serial.print("Coffee was detected at "); 
    Serial.println(millis());
    //sprintf(message, "Coffee at #%i", id); 
    create_tweet(message);
    if (ethernet_up)
      tweet(message);
    else
      Serial.println(message);
  }
  
  delay(1000);
}
