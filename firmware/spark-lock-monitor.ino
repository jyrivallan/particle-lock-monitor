/*
 
 ---=== SPARK DOOR MONITOR ===---
 
 Sends Spark Cloud events on door monitoring switch status change.
 Requires a Spark Photon or firmware updated Spark Core
 More info at http://docs.spark.io/firmware/#system-spark-sleep
          and https://github.com/spark/firmware/tree/bootloader-patch-update

*/

// Pin connected to a switch (or serial connected switches) monitoring lock status
#define BUTTON D0
// Default led pin
#define LED D7
// Seconds before deep sleep
#define TIMEOUT 20
// Monitor polling interval (milliseconds)
#define DELAY 100

const char eventName[] = "statusChange";
const char opmsg[] = "open";
const char clmsg[] = "closed";

// Current event state
char *state;
// Current lock status
int read = LOW;
// Previous lock status
int was = LOW;
// Epoch time of next deep sleep, or 0 if it needs to be reprocessed (on boot or after wakeup)
int sleep = 0;

void setup()
{
    pinMode(BUTTON, INPUT_PULLUP);
    pinMode(LED, OUTPUT);
}

void loop()
{
    read = digitalRead(BUTTON);
    digitalWrite(LED, read);
    
    if (read == HIGH) {
        state = (char*)opmsg;
    } else {
        state = (char*)clmsg;
    }
    
    // Wait for TIMEOUT seconds if just got back from sleep or is state just changed
    if (sleep == 0 || read != was) {
        sleep = Time.now() + TIMEOUT;
        Spark.publish(eventName, state);
    }
    
    was = read;

    if (Time.now() > sleep) {
        sleep = 0;
        if (read == HIGH) {
            Spark.sleep(BUTTON, FALLING);
        } else {
            // RISING does not seem to work here. CHANGE works instead
            Spark.sleep(BUTTON, CHANGE);
        }
    } else {
        // Polling interval of DELAY milliseconds
        delay(DELAY);
    }
}