bool inputLockFlag = false; //flag pentru blocarea inputului in anumite momente

const int pinLatch595   = 11; //pin pentru latch-ul 74HC595
const int pinCeas595    = 10; //pin pentru clock-ul 74HC595
const int pinDate595    = 12; //pin pentru datele 74HC595

const int numarCifreAfisaj = 4; //numarul de cifre pe afisajul 7 segmente
const int piniCifre[numarCifreAfisaj] = {4, 5, 6, 7}; //pinii care selecteaza fiecare cifra

const int pinJoystickX  = A0; //pin analog pentru axa X a joystick-ului
const int pinJoystickY  = A1; //pin analog pentru axa Y a joystick-ului
const int pinJoystickSw = 8; //pin digital pentru butonul joystick-ului

const int joyLeftLimit = 300; //limita minima pentru miscarea spre stanga
const int joyRightLimit = 700; //limita maxima pentru miscarea spre dreapta
const unsigned long debounceMenuMs = 300; //timp de debounce pentru miscare in meniu

unsigned long lastMenuMoveTime = 0; //timpul ultimei miscari in meniu
unsigned long joyPressStartTime = 0; //timpul cand a inceput apasarea pe joystick
const unsigned long joyLongPressTime = 800; //durata minima pentru o apasare lunga

const int pinButonPauza = 2; //pinul pentru butonul de pauza
const int pinBuzzer     = 9; //pinul pentru buzzer

struct intrareFont { //structura pentru asocierea caracterelor cu modelul de segmente
  char caracter;
  byte model;
};

const intrareFont font7seg[] = { //tabela de caractere pentru afisajul 7 segmente
  {'0', 0b11111100}, 
  {'1', 0b01100000}, 
  {'2', 0b11011010},
  {'3', 0b11110010}, 
  {'4', 0b01100110}, 
  {'5', 0b10110110},
  {'6', 0b10111110}, 
  {'7', 0b11100000}, 
  {'8', 0b11111110},
  {'9', 0b11110110},
  {'A', 0b11101110}, 
  {'C', 0b10011100}, 
  {'E', 0b10011110},
  {'L', 0b00011100}, 
  {'O', 0b11111100}, 
  {'P', 0b11001110},
  {'R', 0b00001010}, 
  {'S', 0b10110110}, 
  {'t', 0b00011110},
  {'U', 0b01111100}, 
  {'y', 0b01110110}, 
  {'H', 0b01101110},
  {' ', 0b00000000}
};

const int fontCount = sizeof(font7seg) / sizeof(font7seg[0]); //numarul de intrari in font

byte getCharPattern(char c) { //functie care returneaza modelul de segmente pentru un caracter
  for (int i = 0; i < fontCount; i++) {
    if (font7seg[i].caracter == c) 
      return font7seg[i].model;
  }
  return 0;
}

const bool enableSerialFeedback = true; //flag pentru activarea mesajelor in serial

void printGameMessage(const String &msg) { //functie pentru printarea mesajelor jocului
  if (enableSerialFeedback) {
    Serial.println(msg);
  }
}

char displayChars[numarCifreAfisaj] = {' ', ' ', ' ', ' '}; //buffer pentru caracterele afisate

enum blinkModeType { blinkModeNone, blinkModeFast, blinkModeSlow }; //moduri de clipire
blinkModeType digitBlinkMode[numarCifreAfisaj] = {
  blinkModeNone, blinkModeNone, blinkModeNone, blinkModeNone
};

unsigned long lastMultiplexTime = 0; //timpul ultimei actualizari de multiplexare
int currentDigitIndex = 0; //indexul cifrei curente afisate

const unsigned long multiplexIntervalMs = 2; //intervalul de multiplexare
const unsigned long blinkFastIntervalMs = 125; //interval de clipire rapida
const unsigned long blinkSlowIntervalMs = 500; //interval de clipire lenta

enum gameStateType { //enum pentru starile jocului
  gameStateIdleMenu,
  gameStateMenuAction,
  gameStateShowScoreMenu,
  gameStateShowSequence,
  gameStateInputPhase,
  gameStateCheckAnswer,
  gameStateResult,
  gameStatePaused
};

gameStateType gameState = gameStateIdleMenu; //starea curenta a jocului

const int sequenceLength = 4; //lungimea secventei de memorat
char sequenceChars[sequenceLength]; //buffer pentru secventa generata
int inputDigits[sequenceLength]; //buffer pentru inputul jucatorului
bool digitLocked[sequenceLength] = {false, false, false, false}; //flaguri pentru cifrele blocate

int activeDigitIndex = 0; //indexul cifrei active
bool digitEditing = false; //flag pentru modul de editare

int currentScore = 0; //scorul curent
int highScore = 0; //cel mai mare scor salvat

unsigned long sequenceDisplayTimeMs = 16000; //timpul de afisare al secventei initiale
const unsigned long sequenceDisplayMinMs = 2000; //timpul minim al secventei
const unsigned long sequenceDisplayStepMs = 4000; //pasul cu care scade timpul

unsigned long stateStartTime = 0; //momentul de inceput al starii curente
bool resultSuccessFlag = false; //flag daca raspunsul este corect
bool gameInProgress = false; //flag daca jocul este in desfasurare

const int menuItemCount = 4; //numarul de optiuni din meniu
int menuIndex = 0; //indexul curent din meniu

enum menuActionType { menuActionNone, menuActionPlay, menuActionScore, menuActionStop, menuActionHelp}; //actiunile posibile
menuActionType pendingMenuAction = menuActionNone; //actiunea curenta in asteptare

int joystickXDirection = 0, joystickYDirection = 0; //directii joystick
int joystickXDirectionLast = 0, joystickYDirectionLast = 0; //directii anterioare
bool joystickMoveLeftFlag, joystickMoveRightFlag, joystickMoveUpFlag, joystickMoveDownFlag; //flaguri pentru directii
bool joystickShortPressFlag, joystickLongPressFlag; //flaguri pentru apasari scurte/lungi

bool joystickSwLastState = HIGH; //ultima stare a butonului joystick
bool joystickSwPressed = false; //flag pentru apasare
unsigned long joystickSwPressTime = 0; //timpul de apasare
bool joystickSwLongPressFired = false; //flag daca s-a detectat apasare lunga
const unsigned long joystickLongPressThresholdMs = 700; //pragul pentru apasare lunga

bool pauseButtonLastState = HIGH; //ultima stare a butonului de pauza
unsigned long pauseButtonLastChangeTime = 0; //timpul ultimei schimbari
const unsigned long pauseButtonDebounceMs = 200; //debounce pentru butonul de pauza
bool pauseButtonPressedFlag = false; //flag daca s-a apasat pauza

const int joystickLowThreshold  = 300; //limita inferioara pentru joystick
const int joystickHighThreshold = 700; //limita superioara pentru joystick

bool buzzerActive = false; //flag daca buzzerul este activ
unsigned long buzzerEndTime = 0; //momentul opririi buzzerului

void startBuzzerTone(int freq, unsigned long dur) { //porneste sunetul
  tone(pinBuzzer, freq);
  buzzerActive = true;
  buzzerEndTime = millis() + dur;
}

void stopBuzzerTone() { 
  noTone(pinBuzzer); 
  buzzerActive = false; 
  } //opreste sunetul
void updateBuzzer() { 
  if (buzzerActive && millis() > buzzerEndTime) 
  stopBuzzerTone(); 
} //actualizeaza starea buzzerului

void playTickSound() { 
  startBuzzerTone(2000, 40); 
  } //sunet scurt pentru miscare

void playClickSound() { 
  startBuzzerTone(1500, 80); 
  } //sunet click

void playSuccessSound() { 
  startBuzzerTone(2200, 200); 
  } //sunet succes

void playErrorSound() { 
  startBuzzerTone(400, 400); } //sunet eroare

void setSegments(byte pattern) { //trimite patternul catre afisaj
  digitalWrite(pinLatch595, LOW);
  shiftOut(pinDate595, pinCeas595, MSBFIRST, pattern);
  digitalWrite(pinLatch595, HIGH);
}

void enableDigit(int index) { //activeaza cifra dorita
  for (int i = 0; i < numarCifreAfisaj; i++) {
    digitalWrite(piniCifre[i], i == index ? LOW : HIGH);
  }
}

void updateDisplayMultiplex() { //actualizeaza afisajul multiplexat
  unsigned long now = millis();

  if (now - lastMultiplexTime < multiplexIntervalMs) 
    return;

  lastMultiplexTime = now;

  currentDigitIndex = (currentDigitIndex + 1) % numarCifreAfisaj;

  bool visible = true;
  
  if (digitBlinkMode[currentDigitIndex] == blinkModeFast)
    visible = ((now / blinkFastIntervalMs) % 2) == 0;
  else 
    if (digitBlinkMode[currentDigitIndex] == blinkModeSlow)
      visible = ((now / blinkSlowIntervalMs) % 2) == 0;

  byte pattern = visible ? getCharPattern(displayChars[currentDigitIndex]) : 0;
  setSegments(pattern);
  enableDigit(currentDigitIndex);
}

void setDisplayText(const char *txt4) { //seteaza textul pe afisaj
  for (int i = 0; i < numarCifreAfisaj; i++) {
    displayChars[i] = txt4[i];
    digitBlinkMode[i] = blinkModeNone;
  }
}

void setDisplayNumber(int value) { //afiseaza un numar pe 4 cifre
  if (value < 0) value = 0;

  if (value > 9999) value = 9999;
    displayChars[0] = (value >= 1000) ? ('0' + (value / 1000) % 10) : ' ';
    displayChars[1] = (value >= 100)  ? ('0' + (value / 100) % 10)  : ' ';
    displayChars[2] = (value >= 10)   ? ('0' + (value / 10) % 10)   : ' ';
    displayChars[3] = '0' + (value % 10);

  for (int i = 0; i < numarCifreAfisaj; i++) 
    digitBlinkMode[i] = blinkModeNone;
}

void updateInputBlinkModes() { //actualizeaza modul de clipire pentru cifre
  for (int i = 0; i < sequenceLength; i++) {
    if (digitEditing && i == activeDigitIndex)
      digitBlinkMode[i] = blinkModeFast;
    else if (digitLocked[i])
      digitBlinkMode[i] = blinkModeSlow;
    else
      digitBlinkMode[i] = blinkModeNone;
  }
}

const unsigned long debounceMoveMs = 250; //debounce pentru joystick
unsigned long lastMoveTime = 0; //timp ultima miscare

void updateJoystickDirection() { //citeste directiile joystickului
  int xValue = analogRead(pinJoystickX);
  int yValue = analogRead(pinJoystickY);
  unsigned long now = millis();

  joystickMoveLeftFlag = false;
  joystickMoveRightFlag = false;
  joystickMoveUpFlag = false;
  joystickMoveDownFlag = false;

  if (xValue < 300 && now - lastMoveTime > debounceMoveMs) {
    joystickMoveLeftFlag = true;
    lastMoveTime = now;
  }
  if (xValue > 700 && now - lastMoveTime > debounceMoveMs) {
    joystickMoveRightFlag = true;
    lastMoveTime = now;
  }

  if (yValue < 300 && now - lastMoveTime > debounceMoveMs) {
    joystickMoveUpFlag = true;
    lastMoveTime = now;
  }
  if (yValue > 700 && now - lastMoveTime > debounceMoveMs) {
    joystickMoveDownFlag = true;
    lastMoveTime = now;
  }
}
void updateJoystickButton() { //citeste starea butonului joystickului

  bool sw = digitalRead(pinJoystickSw); //citire buton joystick
  unsigned long now = millis(); //momentul curent

  if (sw != joystickSwLastState) { //daca s-a schimbat starea
    joystickSwLastState = sw;

    if (sw == LOW) { //apasat
      joystickSwPressed = true;
      joystickSwPressTime = now;
      joystickSwLongPressFired = false;
    } else { //eliberat
      if (joystickSwPressed && !joystickSwLongPressFired && now - joystickSwPressTime < joystickLongPressThresholdMs)
        joystickShortPressFlag = true; //apasare scurta
      joystickSwPressed = false;
    }
  }

  if (joystickSwPressed && !joystickSwLongPressFired && now - joystickSwPressTime >= joystickLongPressThresholdMs) {
    joystickLongPressFlag = true; //apasare lunga detectata
    joystickSwLongPressFired = true;
  }
}

void updatePauseButton() { //citeste butonul de pauza cu debounce
  bool st = digitalRead(pinButonPauza); //citire stare
  unsigned long now = millis(); //timp curent

  if (st != pauseButtonLastState && (now - pauseButtonLastChangeTime) > pauseButtonDebounceMs) {
    pauseButtonLastChangeTime = now; //actualizare timp schimbare
    pauseButtonLastState = st; //actualizare stare

    if (st == LOW) 
    pauseButtonPressedFlag = true; //setare flag cand e apasat
  }
}

void updateInputs() { //actualizeaza toate inputurile
  updateJoystickDirection(); //citeste directiile joystickului
  updateJoystickButton(); //citeste butonul joystickului
  updatePauseButton(); //citeste butonul pauza
}

void clearInputFlags() { //reseteaza toate flagurile inputurilor
  joystickMoveLeftFlag = joystickMoveRightFlag = joystickMoveUpFlag = joystickMoveDownFlag = false;
  joystickShortPressFlag = joystickLongPressFlag = false;
  pauseButtonPressedFlag = false;
}

void startNewRound() { //porneste o noua runda de joc
  for (int i = 0; i < sequenceLength; i++)
    sequenceChars[i] = '0' + random(0, 10); //genereaza o secventa random de cifre

  setDisplayText(sequenceChars); //afiseaza secventa
  stateStartTime = millis(); //salveaza momentul
  gameState = gameStateShowSequence; //trece in starea de afisare
  gameInProgress = true; //setare flag joc activ
}

void startNewGame() { //porneste un joc nou
  pauseButtonPressedFlag = false; //reseteaza flagul de pauza
  pauseButtonLastState = digitalRead(pinButonPauza); //sincronizeaza starea curenta
  pauseButtonLastChangeTime = millis(); //actualizeaza timpul

  currentScore = 0; //reseteaza scorul
  sequenceDisplayTimeMs = 16000; //reseteaza timpul initial de afisare
  startNewRound(); //incepe prima runda
}

void enterInputPhase() { //trece in faza in care jucatorul introduce cifrele
  for (int i = 0; i < sequenceLength; i++) {
    inputDigits[i] = 0;
    digitLocked[i] = false;
    displayChars[i] = '0';
  }
  activeDigitIndex = 0; //selecteaza prima cifra
  digitEditing = false; //nu este in mod editare
  updateInputBlinkModes(); //actualizeaza clipirea
  gameState = gameStateInputPhase; //trece in starea de input
}

void goToIdleMenu() { //revine in meniul principal
  setDisplayText("PLAy"); //afiseaza PLAY
  menuIndex = 0; //reset index meniu
  gameState = gameStateIdleMenu; //seteaza starea meniu
  gameInProgress = false; //nu e in joc
}

void updateStateIdleMenu() { //actualizeaza logica meniului principal
  if (menuIndex == 0) 
    setDisplayText("PLAy");
  else if (menuIndex == 1) 
    setDisplayText("SCOR");
  else if (menuIndex == 2) 
    setDisplayText("StOP");
  else 
    setDisplayText("HELP");

  int xVal = analogRead(pinJoystickX); //citeste pozitia X a joystickului
  unsigned long now = millis();

  if (xVal < joyLeftLimit && now - lastMenuMoveTime > debounceMenuMs) {
    menuIndex--;
    if (menuIndex < 0) 
    menuIndex = menuItemCount - 1;
    playTickSound(); //sunet miscare
    lastMenuMoveTime = now;
  } else if (xVal > joyRightLimit && now - lastMenuMoveTime > debounceMenuMs) {
    menuIndex++;
    
    if (menuIndex >= menuItemCount) 
    menuIndex = 0;
    playTickSound();
    lastMenuMoveTime = now;
  }

  bool sw = digitalRead(pinJoystickSw); //citeste buton joystick
  if (sw == LOW) {
    if (joyPressStartTime == 0) 
      joyPressStartTime = now; 
  } else {
    if (joyPressStartTime != 0) {
      unsigned long pressLen = now - joyPressStartTime;
      joyPressStartTime = 0;

      if (pressLen >= joyLongPressTime) { //apasare lunga
        playClickSound();

        if (menuIndex == 0) {
          pendingMenuAction = menuActionPlay;
          setDisplayText("PLAy");

        } else if (menuIndex == 1) {
          pendingMenuAction = menuActionScore;
          setDisplayText("SCOR");

        } else if (menuIndex == 2) {
          pendingMenuAction = menuActionStop;
          setDisplayText("StOP");

        } else {
          pendingMenuAction = menuActionHelp;
          setDisplayText("HELP");
        }

        stateStartTime = millis();
        gameState = gameStateMenuAction;
      } else { //apasare scurta
        if (menuIndex == 1) {
          playClickSound();
          pendingMenuAction = menuActionScore;
          setDisplayText("SCOR");
          stateStartTime = millis();
          gameState = gameStateMenuAction;
        } else if (menuIndex == 3) {
          playClickSound();
          pendingMenuAction = menuActionHelp;
          setDisplayText("HELP");
          stateStartTime = millis();
          gameState = gameStateMenuAction;
        }
      }
    }
  }
}

void updateStateMenuAction() { //executa actiunea selectata in meniu
  if (millis() - stateStartTime >= 800) {
    if (pendingMenuAction == menuActionPlay) {
      printGameMessage("→ Starting new game...");
      startNewGame();
    }
    else if (pendingMenuAction == menuActionScore) {
      printGameMessage("→ Showing high score...");
      setDisplayNumber(highScore);
      gameState = gameStateShowScoreMenu;
      stateStartTime = millis();
    }
    else if (pendingMenuAction == menuActionStop) {
      printGameMessage("→ Stopping game and returning to menu...");
      goToIdleMenu();
    }
    else if (pendingMenuAction == menuActionHelp) { //afiseaza instructiuni in serial
      printGameMessage("");
      printGameMessage(" HOW TO PLAY:");
      printGameMessage("------------------------");
      printGameMessage(" Goal: Memorize the digits shown briefly.");
      printGameMessage("   Then re-enter them using the joystick.");
      printGameMessage("");
      printGameMessage("Controls:");
      printGameMessage(" - Move LEFT / RIGHT: select digit position");
      printGameMessage(" - Move UP / DOWN: change digit value");
      printGameMessage(" - Short press: toggle edit mode");
      printGameMessage(" - Long press: confirm all digits (submit)");
      printGameMessage("");
      printGameMessage(" Tip: Each round gets faster!");
      printGameMessage("------------------------");
      printGameMessage("");
      printGameMessage("Navigate again with joystick ← → to choose another option.");
      printGameMessage("");
      setDisplayText("HELP");
      stateStartTime = millis();
      gameState = gameStateShowScoreMenu; 
    }

    pendingMenuAction = menuActionNone;
    inputLockFlag = false;  
  }
}

void updateStateShowScoreMenu() { //afiseaza scorul pentru 2 secunde apoi revine in meniu
  if (millis() - stateStartTime >= 2000) 
  goToIdleMenu();
}

void updateStateShowSequence() { //afiseaza secventa de memorat
  unsigned long now = millis();

  if (now - stateStartTime < 800) {
    pauseButtonPressedFlag = false;
  }

  if (pauseButtonPressedFlag) { 
    setDisplayText("PAUS"); 
    printGameMessage("Game paused.");
    gameState = gameStatePaused; 
    stateStartTime = now; 
    return; 
  }

  if (now - stateStartTime >= sequenceDisplayTimeMs) {
    printGameMessage("Sequence display done → your turn!");
    enterInputPhase();
  }
}

void updateStateInputPhase() { //gestioneaza inputul jucatorului

  if (pauseButtonPressedFlag) { 
    setDisplayText("PAUS"); 
    gameState = gameStatePaused; 
    stateStartTime = millis(); 
    return; 
    }

  if (!digitEditing) { //navigare intre cifre
    if (joystickMoveLeftFlag) { 
      activeDigitIndex = (activeDigitIndex - 1 + sequenceLength) % sequenceLength; 
      playTickSound(); 
      }
    if (joystickMoveRightFlag){ 
      activeDigitIndex = (activeDigitIndex + 1) % sequenceLength; 
      playTickSound(); 
      }
  }

  if (joystickShortPressFlag) { //comuta intre mod editare si navigare
    digitEditing = !digitEditing;
    digitLocked[activeDigitIndex] = !digitEditing;
    playClickSound();
  }

  if (digitEditing) { //schimba valoarea cifrei active
    if (joystickMoveUpFlag) { 
      inputDigits[activeDigitIndex] = (inputDigits[activeDigitIndex] + 1) % 10; playTickSound(); 
      }
    if (joystickMoveDownFlag){ 
      inputDigits[activeDigitIndex] = (inputDigits[activeDigitIndex] + 9) % 10; playTickSound(); 
      }
  }

  for (int i = 0; i < sequenceLength; i++) 
  displayChars[i] = '0' + inputDigits[i];
  updateInputBlinkModes();

  if (joystickLongPressFlag) 
  gameState = gameStateCheckAnswer; //apasare lunga confirma raspunsul
}

void updateStateCheckAnswer() { //verifica raspunsul jucatorului
  bool correct = true;

  for (int i = 0; i < sequenceLength; i++)
    if ('0' + inputDigits[i] != sequenceChars[i]) correct = false;

  resultSuccessFlag = correct;
  if (correct) { //daca e corect
    currentScore++;
    if (currentScore > highScore) 
    highScore = currentScore;
    sequenceDisplayTimeMs = max(sequenceDisplayTimeMs - sequenceDisplayStepMs, sequenceDisplayMinMs);

    playSuccessSound();
    setDisplayNumber(currentScore);
    printGameMessage("Correct! ✅ Starting next round...");

  } else { //gresit
    playErrorSound();
    setDisplayText("Err ");
    printGameMessage(" Wrong! ❌ END ROUND → Game lost.");
    printGameMessage("Your final score: " + String(currentScore));
  }
  stateStartTime = millis();
  gameState = gameStateResult;
}

void updateStateResult() { //gestioneaza rezultatul dupa verificare
  unsigned long now = millis();

  if (pauseButtonPressedFlag) { 
    setDisplayText("PAUS"); 
    gameState = gameStatePaused; 
    stateStartTime = now; 
    return; 
  }

  if (resultSuccessFlag) {
    if (now - stateStartTime >= 800) 
      startNewRound();
  } else {
    if (now - stateStartTime < 700) 
      setDisplayText("Err ");
    else if (now - stateStartTime < 2000) 
      setDisplayNumber(currentScore);
    else 
      goToIdleMenu();
  }
}

void updateStatePaused() { //pauzeaza jocul temporar
   if (millis() - stateStartTime >= 1200) {
    printGameMessage("Resuming to main menu...");
    goToIdleMenu();
  }
}

void setup() { //initializare pinii si afisajul
  pinMode(pinLatch595, OUTPUT);
  pinMode(pinCeas595, OUTPUT);
  pinMode(pinDate595, OUTPUT);

  for (int i = 0; i < numarCifreAfisaj; i++) { 
    pinMode(piniCifre[i], OUTPUT); 
    digitalWrite(piniCifre[i], HIGH); 
    }
  pinMode(pinJoystickSw, INPUT_PULLUP);
  pinMode(pinButonPauza, INPUT_PULLUP);
  pinMode(pinBuzzer, OUTPUT);

  randomSeed(analogRead(A2));

  setDisplayText("PLAy");
  stateStartTime = millis();

  Serial.begin(9600);

  printGameMessage("=== MEMORY GAME READY ===");
  printGameMessage("Navigate with joystick ← →, press to select.");
  printGameMessage("Current option: START GAME → press to play.");
}

void loop() { //bucla principala
  updateInputs(); //citeste toate intrarile
  updateBuzzer(); //actualizeaza buzzerul

  switch (gameState) { //executa logica in functie de starea curenta
    case gameStateIdleMenu:       
    updateStateIdleMenu(); 
    break;

    case gameStateMenuAction:     
    updateStateMenuAction(); 
    break;

    case gameStateShowScoreMenu:  
    updateStateShowScoreMenu(); 
    break;

    case gameStateShowSequence:   
    updateStateShowSequence(); 
    break;

    case gameStateInputPhase:     
    updateStateInputPhase(); 
    break;

    case gameStateCheckAnswer:    
    updateStateCheckAnswer(); 
    break;

    case gameStateResult:         
    updateStateResult(); 
    break;

    case gameStatePaused:         
    updateStatePaused(); 
    break;
  }

  updateDisplayMultiplex(); //actualizeaza afisajul
  clearInputFlags(); //reseteaza flagurile inputurilor
}
