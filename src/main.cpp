#include <LedControl.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <NewPing.h>
#include <Arduino.h>

// Define the pin connections for the MAX7219
const int DIN_PIN = 11; // Data input
const int CS_PIN = 10;  // Chip select
const int CLK_PIN = 13; // Clock

// Define the pin connections for the ultrasonic sensor
const int TRIG_PIN = 7;
const int ECHO_PIN = 6;
#define MAX_DISTANCE 200 // Maximum distance to measure (in cm)

// Define the pin for the button
const int BUTTON_PIN = 2;

// Define the pin for the buzzer
const int BUZZER_PIN = 8;

// Define the pins for the focus and rest LEDs
const int FOCUS_LED_PIN = 4;
const int REST_LED_PIN = 5;

// Create instances of the libraries
LedControl lc = LedControl(DIN_PIN, CLK_PIN, CS_PIN, 1);
LiquidCrystal_I2C lcd(0x27, 16, 2);
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

// Function Prototypes
void fillDots(int numDots);
void displayDigit(int num);
void startPomodoro();
void resetPomodoro();
void stopPomodoro();
bool isHandNearSensor();
void buzz(int duration);

// 5x7 Font for digits 0-9
const byte digits[10][7] = {
    {0b0111110, 0b1100011, 0b1100011, 0b1100011, 0b1100011, 0b1100011, 0b0111110}, // 0
    {0b0001100, 0b0011100, 0b0111100, 0b0001100, 0b0001100, 0b0001100, 0b1111111}, // 1
    {0b0111110, 0b1100011, 0b0000011, 0b0000110, 0b0011100, 0b0110000, 0b1111111}, // 2
    {0b0111110, 0b1100011, 0b0000011, 0b0011110, 0b0000011, 0b1100011, 0b0111110}, // 3
    {0b0000110, 0b0001110, 0b0011110, 0b0110110, 0b1100110, 0b1111111, 0b0000110}, // 4
    {0b1111111, 0b1100000, 0b1111110, 0b0000011, 0b0000011, 0b1100011, 0b0111110}, // 5
    {0b0011110, 0b0110000, 0b1100000, 0b1111110, 0b1100011, 0b1100011, 0b0111110}, // 6
    {0b1111111, 0b0000011, 0b0000110, 0b0001100, 0b0011000, 0b0110000, 0b1100000}, // 7
    {0b0111110, 0b1100011, 0b1100011, 0b0111110, 0b1100011, 0b1100011, 0b0111110}, // 8
    {0b0111110, 0b1100011, 0b1100011, 0b0111111, 0b0000011, 0b0000110, 0b0111100}  // 9
};

// Time tracking variables
unsigned long startTime;
unsigned long elapsedTime;
unsigned long buttonPressTime = 0; // Time when button was pressed
bool buttonPressed = false;        // Flag to track button press state
int buttonPressCount = 0;          // Counter for button presses
bool timerRunning = false;         // Flag to track timer state
bool firstRun = true;              // Flag to track the first run
int pomoCount = 0;                 // Counter for Pomodoro iterations
bool inRestPeriod = false;         // Flag to track rest period
unsigned long restStartTime;       // Start time for the rest period
int restDuration = 0;              // Duration of the rest period
bool focusLedState = false;        // State of the focus LED
bool restLedState = false;         // State of the rest LED
bool ledMatrixState = false;       // State of the LED matrix

// Spiral fill order for an 8x8 matrix
const int spiralOrder[64][2] = {
    {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}, {1, 7}, {2, 7}, {3, 7}, {4, 7}, {5, 7}, {6, 7}, {7, 7}, {7, 6}, {7, 5}, {7, 4}, {7, 3}, {7, 2}, {7, 1}, {7, 0}, {6, 0}, {5, 0}, {4, 0}, {3, 0}, {2, 0}, {1, 0}, {1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {2, 6}, {3, 6}, {4, 6}, {5, 6}, {6, 6}, {6, 5}, {6, 4}, {6, 3}, {6, 2}, {6, 1}, {5, 1}, {4, 1}, {3, 1}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {2, 5}, {3, 5}, {4, 5}, {5, 5}, {5, 4}, {5, 3}, {5, 2}, {4, 2}, {3, 2}, {3, 3}, {4, 3}};

int previousDotsToFill = -1;    // Track the previous number of dots filled
int previousCountdown = -1;     // Track the previous countdown value
int previousRestCountdown = -1; // Track the previous rest countdown value

void setup()
{
  Serial.begin(115200); // Start the serial communication for debugging
  Serial.println("Pomodoro Timer");

  // Initialize the MAX7219 display
  lc.shutdown(0, false); // Wake up display
  lc.setIntensity(0, 8); // Set brightness level (0 to 15)
  lc.clearDisplay(0);    // Clear display register

  // Initialize the I2C LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press button to");
  lcd.setCursor(0, 1);
  lcd.print("start Pomodoro");

  // Initialize the button pin
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Use internal pull-up resistor

  // Initialize the buzzer pin
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); // Ensure the buzzer is off initially

  // Initialize the LED pins
  pinMode(FOCUS_LED_PIN, OUTPUT);
  pinMode(REST_LED_PIN, OUTPUT);
  digitalWrite(FOCUS_LED_PIN, LOW); // Ensure the focus LED is off initially
  digitalWrite(REST_LED_PIN, LOW);  // Ensure the rest LED is off initially
}

void loop()
{
  // Handle button presses for initial start
  if (!timerRunning && !inRestPeriod && digitalRead(BUTTON_PIN) == LOW && !buttonPressed)
  {
    buttonPressed = true;
    buttonPressTime = millis();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Place hand to");
    lcd.setCursor(0, 1);
    lcd.print("sensor to start");
    firstRun = false; // Set the firstRun flag to false after the button is pressed
  }

  if (digitalRead(BUTTON_PIN) == HIGH && buttonPressed)
  {
    buttonPressed = false;
  }

  // Stop or reset Pomodoro when the button is held down
  if (digitalRead(BUTTON_PIN) == LOW && buttonPressed && (millis() - buttonPressTime > 2000))
  {
    if (timerRunning)
    {
      stopPomodoro();
      buttonPressed = false; // Reset button press flag
    }
    else
    {
      resetPomodoro();
      buttonPressed = false; // Reset button press flag
    }
  }

  // Start Pomodoro when hand is near the sensor and the button has been pressed at least once
  if (!timerRunning && !inRestPeriod && !firstRun && isHandNearSensor())
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Starting timer");
    startPomodoro();
  }

  if (timerRunning)
  {
    elapsedTime = millis() - startTime;
    int totalSeconds = elapsedTime / 1000;

    if (totalSeconds <= 40)
    {
      if (!focusLedState)
      {
        digitalWrite(FOCUS_LED_PIN, HIGH); // Turn on the focus LED
        digitalWrite(REST_LED_PIN, LOW);   // Turn off the rest LED
        focusLedState = true;
        restLedState = false;
      }

      if (!ledMatrixState)
      {
        lc.clearDisplay(0); // Clear the LED matrix display
        ledMatrixState = true;
      }

      int countdown = 40 - totalSeconds;
      if (countdown != previousCountdown)
      {
        lcd.setCursor(0, 1);
        lcd.print("Focus: ");
        lcd.print(countdown);
        lcd.print(" sec ");
        previousCountdown = countdown;
      }

      int dotsToFill = (totalSeconds * 64) / 40;
      if (dotsToFill != previousDotsToFill)
      {
        lc.clearDisplay(0);
        fillDots(dotsToFill);
        previousDotsToFill = dotsToFill;
      }
    }

    if (totalSeconds >= 40)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Pomodoro Done");
      buzz(1000);
      pomoCount++;
      lcd.setCursor(0, 1);
      lcd.print("Pomos: ");
      lcd.print(pomoCount);
      delay(2000); // Display "Pomodoro Done" for 2 seconds
      lcd.clear();

      if (pomoCount % 3 == 0)
      {
        restDuration = 15; // 15 seconds rest after every 3 focus periods
      }
      else
      {
        restDuration = 10; // 10 seconds rest period
      }
      inRestPeriod = true;
      restStartTime = millis();
      timerRunning = false; // Stop the timer
      buzz(1000);           // Buzz when rest starts
    }
  }
  else if (inRestPeriod)
  {
    unsigned long restElapsed = millis() - restStartTime;
    int restTotalSeconds = restElapsed / 1000;

    if (!restLedState)
    {
      digitalWrite(FOCUS_LED_PIN, LOW); // Turn off the focus LED
      digitalWrite(REST_LED_PIN, HIGH); // Turn on the rest LED
      focusLedState = false;
      restLedState = true;
    }

    if (ledMatrixState)
    {
      lc.clearDisplay(0); // Clear the LED matrix display
      ledMatrixState = false;
    }

    if (restTotalSeconds <= restDuration)
    {
      int restCountdown = restDuration - restTotalSeconds;
      if (restCountdown != previousRestCountdown)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Rest: ");
        lcd.print(restDuration);
        lcd.print(" sec ");
        lcd.setCursor(0, 1);
        lcd.print("Remaining: ");
        lcd.print(restCountdown);
        lcd.print(" sec ");
        previousRestCountdown = restCountdown;
      }
    }

    if (restTotalSeconds >= restDuration)
    {
      buzz(1000); // Buzz when rest ends
      inRestPeriod = false;
      resetPomodoro(); // Reset Pomodoro to wait for next hand detection
    }
  }
  else
  {
    if (focusLedState || restLedState || ledMatrixState)
    {
      digitalWrite(FOCUS_LED_PIN, LOW); // Turn off the focus LED
      digitalWrite(REST_LED_PIN, LOW);  // Turn off the rest LED
      lc.clearDisplay(0);               // Clear the LED matrix display
      focusLedState = false;
      restLedState = false;
      ledMatrixState = false;
    }
  }
}

void startPomodoro()
{
  startTime = millis();
  timerRunning = true;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pomodoro started");
  lcd.setCursor(0, 1);
  lcd.print("Focus: 40 sec ");
  buzz(1000);
}

void stopPomodoro()
{
  timerRunning = false;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Timer Stopped");
  lcd.setCursor(0, 1);
  lcd.print("Long press to");
  delay(2000);
  lcd.setCursor(0, 1);
  lcd.print("reset Pomodoro");
}

void resetPomodoro()
{
  timerRunning = false;
  previousDotsToFill = -1;    // Reset previousDotsToFill
  previousCountdown = -1;     // Reset previousCountdown
  previousRestCountdown = -1; // Reset previousRestCountdown
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Hold hand near");
  lcd.setCursor(0, 1);
  lcd.print("the sensor");
  lc.clearDisplay(0); // Clear the LED matrix display
}

bool isHandNearSensor()
{
  unsigned int distance = sonar.ping_cm();
  return (distance > 0 && distance <= 10); // Adjust distance threshold as needed
}

void fillDots(int numDots)
{
  for (int i = 0; i < numDots && i < 64; i++)
  {
    int row = spiralOrder[i][0];
    int col = spiralOrder[i][1];
    lc.setLed(0, row, col, true);
  }
}

void displayDigit(int num)
{
  if (num < 0 || num > 9)
    return;

  for (int row = 0; row < 7; row++)
  {
    byte rowData = digits[num][row];
    for (int col = 8; col >= 0; col--)
    {
      bool ledState = (rowData >> col) & 0x01;
      lc.setLed(0, row, 7 - col, ledState);
    }
  }
}

void buzz(int duration)
{
  unsigned long startMillis = millis();
  while (millis() - startMillis < duration)
  {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1); // Adjust the delay to change the tone frequency
    digitalWrite(BUZZER_PIN, LOW);
    delay(1); // Adjust the delay to change the tone frequency
  }
  digitalWrite(BUZZER_PIN, LOW); // Ensure the buzzer is turned off
}
