#include <NewPing.h>
#include <Servo.h>

// Ultrasonic constants
#define SONAR_NUM 2 // Number or sensors.
#define MAX_DISTANCE 100 // Maximum distance (in cm) to ping.
//#define MAX_DISTANCE 50 // Maximum distance (in cm) to ping.
#define PING_INTERVAL 100 // Milliseconds between sensor pings (29ms is about the min to avoid cross-sensor echo).

// Servo constants
#define HORZ_SERVO_PIN 8
#define VERT_SERVO_PIN 9
#define HORZ_MAX_RIGHT 30
#define HORZ_CENTER 90
#define HORZ_MAX_LEFT 140
#define VERT_MAX_UP 45
#define VERT_CENTER 90
#define VERT_MAX_DOWN 150

// LED constants
#define RED_PIN 3
#define GREEN_PIN 5
#define BLUE_PIN 6

unsigned long pingTimer[SONAR_NUM]; // Holds the times when the next ping should happen for each sensor.
unsigned int cm[SONAR_NUM];         // Where the ping distances are stored.
uint8_t currentSensor = 0;          // Keeps track of which sensor is active.

int lastPos = 1000; // left=-1, center=0, right=1, undefined=1000

// Sensor object array. Each sensor's trigger pin, echo pin, max distance to ping
NewPing sonar[SONAR_NUM] = {
  NewPing(11, 10, MAX_DISTANCE), // left
  NewPing(13, 12, MAX_DISTANCE), // right
};

Servo horzServo;
Servo vertServo;

long actionCounter = 0;

void setup() {
  Serial.begin(115200);
  
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);  
  
  pingTimer[0] = millis() + 75;           // First ping starts at 75ms, gives time for the Arduino to chill before starting.
  for (uint8_t i = 1; i < SONAR_NUM; i++) { // Set the starting time for each sensor.
    pingTimer[i] = pingTimer[i - 1] + PING_INTERVAL;
  }
  
  horzServo.attach(HORZ_SERVO_PIN);
  vertServo.attach(VERT_SERVO_PIN);

  horzServo.write(HORZ_CENTER);
  vertServo.write(VERT_CENTER);  
  
  moveEyeballToCenter();
  actionCounter = millis();
}

int randHorzPos() {
  int rand = micros() % 3;
  Serial.println(rand);
  if (rand == 0) {
    return HORZ_CENTER;
  } else if (rand == 1) {
    return HORZ_MAX_LEFT;
  } else {
    return HORZ_MAX_RIGHT;
  }
}

int randVertPos() {
  int rand = micros() % 3;
  Serial.println(rand);
  if (rand == 0) {
    return VERT_CENTER;
  } else if (rand == 1) {
    return VERT_MAX_UP;
  } else {
    return VERT_MAX_DOWN;
  }
}

void loop() {
  for (uint8_t i = 0; i < SONAR_NUM; i++) { // Loop through all the sensors.
    if (millis() >= pingTimer[i]) {         // Is it this sensor's time to ping?
      pingTimer[i] += PING_INTERVAL * SONAR_NUM;  // Set next time this sensor will be pinged.
      if (i == 0 && currentSensor == SONAR_NUM - 1) {
        oneSensorCycle(); // Sensor ping cycle complete, do something with the results.
      }
      sonar[currentSensor].timer_stop();          // Make sure previous timer is canceled before starting a new ping (insurance).
      currentSensor = i;                          // Sensor being accessed.
      cm[currentSensor] = 0;                      // Make distance zero in case there's no ping echo for this sensor.
      sonar[currentSensor].ping_timer(echoCheck); // Do the ping (processing continues, interrupt will call echoCheck to look for echo).
    }
  }
}

void echoCheck() { // If ping received, set the sensor distance to array.
  if (sonar[currentSensor].check_timer())
    cm[currentSensor] = sonar[currentSensor].ping_result / US_ROUNDTRIP_CM;
}

void oneSensorCycle() {
  Serial.println("*");
  
  if ((millis() - actionCounter) <= 100) {
    return;
  }
  actionCounter = millis();
  
  Serial.print(cm[0]);
  Serial.print(" - ");
  Serial.print(cm[1]);
  Serial.println();
  
  // If the spread between left and right is minimal, then look in the center
  if (abs(cm[0] - cm[1]) <= 5) {
    moveEyeballToCenter();
  }
  
  // There's something closer either on the left or right side. So look in that direction.
  else {
    if (cm[0] < cm[1]) {
      moveEyeballToRight();
    } else {
      moveEyeballToLeft();
    }
  }
  
}

void moveEyeballToCenter() {
  if (lastPos == 0) {
    return;
  }
  lastPos = 0;
  
  Serial.print("moving eyeball to center: ");
  setColor(0, 0, 255);
  horzServo.write(HORZ_CENTER);
  vertServo.write(VERT_CENTER);
}

void moveEyeballToLeft() {
  if (lastPos == -1) {
    return;
  }
  lastPos = -1;
  
  Serial.print("moving eyeball to left: ");
  setColor(255, 0, 0);
  horzServo.write(HORZ_MAX_LEFT);
  moveEyeballToVerticalRandom();
}

void moveEyeballToRight() {
  if (lastPos == 1) {
    return;
  }
  lastPos = 1;
  
  Serial.print("moving eyeball to right: ");
  setColor(0, 255, 0);
  horzServo.write(HORZ_MAX_RIGHT);
  moveEyeballToVerticalRandom();
}

void moveEyeballToVerticalRandom() {
  int rand = micros() % 3;
  if (rand == 0) {
    vertServo.write(VERT_CENTER);
  } else if (rand == 1) {
    vertServo.write(VERT_MAX_UP);
  } else {
    vertServo.write(VERT_MAX_DOWN);
  }
}

void setColor(int rgb[]) {
  setColor(rgb[0], rgb[1], rgb[2]);
}

// Set RGB color for a common anode RGB LED
void setColor(int red, int green, int blue) {
  red = 255 - red;
  green = 255 - green;
  blue = 255 - blue;
  analogWrite(RED_PIN, red);
  analogWrite(GREEN_PIN, green);
  analogWrite(BLUE_PIN, blue);
}

