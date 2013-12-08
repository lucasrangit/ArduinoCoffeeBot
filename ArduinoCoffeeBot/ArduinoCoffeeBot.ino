// SPI Support
#include <SPI.h>
// Ethernet Shield Support
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
long tweet_delay_ms = 1000;
char message[139] = { '\0' };

void fill_tweet(char msg[]) {
  static int previous_tweet = 0;
  //static long last_tweet_time = millis();
  //long time = millis();

  // Must use a different message each time because Twitter will block duplicate posts.  
  switch (previous_tweet++) {
    case (0):
      sprintf(msg, "CoffeeBot 0x%X: First brew of the day ready in %s.", id, location);
    case (1):
      sprintf(msg, "CoffeeBot 0x%X: Coffee is ready in %s.", id, location); 
      break;   
    case (2):
      sprintf(msg, "CoffeeBot 0x%X, Coffee in %s is ready.", id, location);
      break;
    default:
      previous_tweet = 1;
  }
  
  // do not exceed 140 characters
  msg[139] = '\0';
}

void tweet(char msg[]) {
  Serial.println("connecting ...");
  if (twitter.post(msg)) {
    int status = twitter.wait(&Serial);
    if (status == 200) {
      Serial.println("OK.");
    } else {
      Serial.print("failed : code ");
      Serial.println(status);
    }
  } else {
    Serial.println("connection failed.");
  }
}

void setup() {
  // Configure Serial Port Monitor
  Serial.begin(baud);
  Serial.print("Serial: ");
  Serial.print(baud);
  Serial.println(" bps");
  
  // Configure Ethernet for DHCP
  if (Ethernet.begin(mac) == 0)
    Serial.println("Ethernet DCHP configuration failed");  
  else {
    Serial.print("IP: ");
    Serial.println(Ethernet.localIP());
    ethernet_up = true;
  }
}

void loop() {
  bool coffee = false;
  
  if (Serial.available() > 0) {
    Serial.print("Hi, I am Arduino CoffeeBot #");
    Serial.println(id);
    Serial.print("IP: ");
    Serial.println(Ethernet.localIP());
  }
  
  // Check inputs
  
  
  // Report status
  if (coffee) {
    Serial.print("Coffee was detected at "); 
    Serial.println(millis());
    sprintf(message, "Coffee at #%i", id); 
    
  }
  
  // Temporary until everything is wired up to avoid excessive processing
  delay(tweet_delay_ms);
}
