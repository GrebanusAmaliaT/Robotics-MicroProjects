const int Rosu_Masini=10;
const int Galben_Masini=9;
const int Verde_Masini=8;

const int Verde_Pieton=5;
const int Rosu_Pieton=3;

const int Pin_Buton=2;
const int Pin_Buzzer=6;

volatile bool buton_apasat=false;
unsigned long timer_start=0;

bool se_face_galben=false;
unsigned long timer_galben=0;

bool se_face_rosu=false;
unsigned long timer_rosu=0;

bool warning=false;
unsigned long timer_warning=0;

const int pinii_timer[8] = {7, 4, 11, 12, 13, A5, A4, A3};

const byte numere[10][7] = {
  {1,1,1,1,1,1,0},
  {0,1,1,0,0,0,0},
  {1,1,0,1,1,0,1},
  {1,1,1,1,0,0,1},
  {0,1,1,0,0,1,1},
  {1,0,1,1,0,1,1},
  {1,0,1,1,1,1,1},
  {1,1,1,0,0,0,0},
  {1,1,1,1,1,1,1},
  {1,1,1,1,0,1,1}
};

void afisareNumaratoare(int num) {
  if (num < 0 || num > 9) 
    return;
  for (int i = 0; i < 7; i++)
    digitalWrite(pinii_timer[i], numere[num][i]);
}

void stergereNumaratoare() {
  for (int i = 0; i < 7; i++)
    digitalWrite(pinii_timer[i], LOW);
}

void idle() {
  digitalWrite(Verde_Masini, HIGH);
  digitalWrite(Galben_Masini, LOW);
  digitalWrite(Rosu_Masini, LOW);

  digitalWrite(Verde_Pieton, LOW);
  digitalWrite(Rosu_Pieton, HIGH);

  stergereNumaratoare();
  noTone(Pin_Buzzer);
}

void se_apasa_butonul() {
  if (!buton_apasat) {
    buton_apasat = true;
  }
}

void setup() {
  pinMode(Rosu_Masini, OUTPUT);
  pinMode(Galben_Masini, OUTPUT);
  pinMode(Verde_Masini, OUTPUT);

  pinMode(Rosu_Pieton, OUTPUT);
  pinMode(Verde_Pieton, OUTPUT);

  pinMode(Pin_Buzzer, OUTPUT);

  for (int i = 0; i < 7; i++) 
    pinMode(pinii_timer[i], OUTPUT);

  pinMode(Pin_Buton, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(Pin_Buton), se_apasa_butonul, FALLING);

  idle();
}

void loop() {
  unsigned long timer_in_prezent = millis();

  if (buton_apasat && !se_face_galben && !se_face_rosu && !warning && timer_start == 0) {
    buton_apasat = false;
    timer_start = timer_in_prezent;
    se_face_galben = false;
    se_face_rosu = false;
    warning = false;
  }

  if (timer_start > 0 && !se_face_galben && !se_face_rosu && !warning) {
    if (timer_in_prezent - timer_start >= 8000UL) {
      se_face_galben = true;
      timer_galben = timer_in_prezent;
    } else {
      unsigned long diferenta = (timer_start + 8000UL) - timer_in_prezent;
      int secunde = (diferenta / 1000) + 1;
      afisareNumaratoare(secunde);
    }
  }

  if (se_face_galben) {
    digitalWrite(Verde_Masini, LOW);
    digitalWrite(Galben_Masini, HIGH);
    digitalWrite(Rosu_Masini, LOW);
    digitalWrite(Rosu_Pieton, HIGH);
    digitalWrite(Verde_Pieton, LOW);
    noTone(Pin_Buzzer);

    unsigned long diferenta = (timer_galben + 3000UL) - timer_in_prezent;
    int secunde = (diferenta / 1000) + 1;
    afisareNumaratoare(secunde);

    if (timer_in_prezent - timer_galben >= 3000UL) {
      se_face_galben = false;
      se_face_rosu = true;
      timer_rosu = timer_in_prezent;
    }
  }

  if (se_face_rosu) {
    digitalWrite(Verde_Masini, LOW);
    digitalWrite(Galben_Masini, LOW);
    digitalWrite(Rosu_Masini, HIGH);
    digitalWrite(Rosu_Pieton, LOW);
    digitalWrite(Verde_Pieton, HIGH);

    unsigned long diferenta = (timer_rosu + 8000UL) - timer_in_prezent;
    int secunde = (diferenta / 1000) + 1;
    afisareNumaratoare(secunde);

    if ((timer_in_prezent / 1000) % 2 == 0) 
      tone(Pin_Buzzer, 1000);
    else 
      noTone(Pin_Buzzer);

    if (timer_in_prezent - timer_rosu >= 8000UL) {
      se_face_rosu = false;
      warning = true;
      timer_warning = timer_in_prezent;
      noTone(Pin_Buzzer);
    }
  }

  if (warning) {
    digitalWrite(Rosu_Masini, HIGH);
    digitalWrite(Galben_Masini, LOW);
    digitalWrite(Verde_Masini, LOW);
    digitalWrite(Rosu_Pieton, LOW);

    unsigned long diferenta = (timer_warning + 4000UL) - timer_in_prezent;
    int secunde = (diferenta / 1000) + 1;
    afisareNumaratoare(secunde);

    if ((timer_in_prezent / 300) % 2 == 0) {
      digitalWrite(Verde_Pieton, HIGH);
      tone(Pin_Buzzer, 1500);
    } else {
      digitalWrite(Verde_Pieton, LOW);
      noTone(Pin_Buzzer);
    }

    if (timer_in_prezent - timer_warning >= 4000UL) {
      warning = false;
      idle();
      timer_start = 0;
      buton_apasat = false;
    }
  }
}
