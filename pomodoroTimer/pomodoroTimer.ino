/*
 * pomodoro timer
 *
 * Turn Kyrio Hourglass DIY Kit into pomodoro timer.
 *
 * Original kit: https://www.amazon.de/-/en/Electronic-Hourglass-Flashing-Soldering-Adjustable-Green/dp/B082HP7TFX/
 *
*/

/*
 * for HW board: Seeeduino XIAO
*/
static const int pinData = 1;
static const int pinClk = 3;
static const int orientationPin = 0;  // see isUpright() for encoding

static const size_t NR_OF_LEDS = 15U;


static const size_t longDurationSecs = 25 * 60 / NR_OF_LEDS;
static const size_t shortDurationSecs = 5 * 60 / NR_OF_LEDS;

////////////////////////


static int position;
static bool isOrientationDown;
static int remainingBlinkingDuration;

static void shift() {
    digitalWrite(pinClk, HIGH);
    delay(1);
    digitalWrite(pinClk, LOW);
    delay(1);
}

static void applyToAll(int state) {
  digitalWrite(pinData, state);
  for (int i=0; i!=NR_OF_LEDS; ++i) {
    shift();
  }
}

static void setAndShift(bool turnOff) {
  digitalWrite(pinData, turnOff ? LOW : HIGH);
  shift();
}

static void blink(bool down) {
  for (int i=0; i != 10; ++i) {
    applyToAll(down ? HIGH : LOW);
    delay(50);
    applyToAll(down ? LOW : HIGH);
    delay(50);
  }
}

static void blink2(bool down) {
  for (int i=0; i != 100; ++i) {
    setAndShift(i % 2 == 0);
    delay(100);
  }
  applyToAll(down ? HIGH : LOW);
}

static bool isUpright() {
  return digitalRead(orientationPin) == 0;
}

void setup() {
# ifdef DEBUG
  Serial.begin(115200);
# endif
  pinMode(pinData, OUTPUT);
  digitalWrite(pinData, LOW);
  pinMode(pinClk, OUTPUT);
  digitalWrite(pinClk, LOW);
  pinMode(orientationPin, INPUT);

  const bool down = isUpright();
  applyToAll(down ? HIGH : LOW);
  isOrientationDown = down;

  position = NR_OF_LEDS + 1;
  remainingBlinkingDuration = 0;
}

static int adjustAfterClockFlip(int currentPos, bool isCurrentlyFacingDown) {
  const int newPosition = NR_OF_LEDS + 1 - currentPos;
  // turning upside down -> let sand "reflow" according gravity:
  // 1. finish remaining steps, 2. forward to position
  for (int i=0; i != NR_OF_LEDS+1; ++i) {
    setAndShift(i < newPosition ? isCurrentlyFacingDown : !isCurrentlyFacingDown);
  }
  return newPosition;
}

void loop() {
  const bool down = isUpright();
# ifdef DEBUG
  Serial.print("position: ");  Serial.print(position);
  Serial.print(", down: ");  Serial.print(down);
  Serial.print(", remainingBlinkingDuration: ");  Serial.println(remainingBlinkingDuration);
# endif
  if (down != isOrientationDown) {
    position = adjustAfterClockFlip(position, isOrientationDown);
    isOrientationDown = down;
    remainingBlinkingDuration = 0;
  }
  if (remainingBlinkingDuration <= 0) {
    position = max(position-1, 0);
    remainingBlinkingDuration = down ? longDurationSecs : shortDurationSecs;
  }
  if (position == 0) {
    blink(down);
  } else {
    delay(1000);
    remainingBlinkingDuration -= 1;
  }

  if (remainingBlinkingDuration <= 0) {
    setAndShift(down);
  }
}
