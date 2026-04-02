const int TrigPIN = 6;
const int EchoPIN = 7; 
const int BuzzerPIN = 5; 

const int Rosu = 10; 
const int Verde = 11; 
const int LDR = A0;

bool setareInCurs = true; 
unsigned long ultimaMasurare = 0; 
int numarCitiri = 0;
float sumaSetare = 0;
float distantaInitiala = 0;

struct State {
  static const byte Dezarmat = 0; 
  static const byte SeArmeaza = 1; 
  static const byte Armat = 2; 
  static const byte PregatitDeActivare = 3;
  static const byte AlarmaActiva = 4;
};

byte state = State::Dezarmat;

bool asteaptaParola = false; 
byte indexParola = 0; 
char numeAlarma[35] = "\nAlarma antiefractie - SPY CAT SRL"; 
char parolaSetata[32] = "parola"; 
char parolaIntrodusa[32];

unsigned int distantaMax = 50; 
unsigned int buzzerFrecventa = 1000; 
unsigned int PragLDR = 0; 

unsigned long timerAlarmare = 0; 
bool inSubmeniuSetari = false; 
int vreauAfisareMeniu = 0; 
bool asteaptaParolaNoua = false;

void setup() {
  Serial.begin(9600); 

  pinMode(TrigPIN, OUTPUT);
  pinMode(EchoPIN, INPUT);
  pinMode(BuzzerPIN, OUTPUT);
  pinMode(Rosu, OUTPUT);
  pinMode(Verde, OUTPUT);
  pinMode(LDR, INPUT);

  digitalWrite(Rosu, LOW); 
  noTone(BuzzerPIN); 

  Serial.println(numeAlarma);
  Serial.println("Sistem pornit. Incep calibrarea ultrasonicului...");
  setareUltrasonic(); //apeleaza functia de calibrare
}

void afisareMeniu() {
  if (vreauAfisareMeniu == 1) { 
    Serial.println("\n~~~~~~~~~~~~~~~~~~~~~ Meniul Principal ~~~~~~~~~~~~~~~~~~~~~~");
    Serial.println("SPY CAT a spus: Buna!!! Cum te pot ajuta astazi?");
    Serial.println("1. Armare sistem"); 
    Serial.println("2. Setari/Configurare");
    Serial.println("3. Testare alarma"); 
    Serial.println("~~~~~~~~~~~~~~~~~~~~~~ Introdu optiunea dorita ~~~~~~~~~~~~~~~~~~~~~");
  }
  vreauAfisareMeniu = 0; 
}

void afisareSubmeniuSetari() {
  inSubmeniuSetari = true; 
  Serial.println("\n---------------------- Setari Avansate ----------------------");
  Serial.println("SPY CAT a spus: Ce doresti sa modifici?");
  Serial.println("1. Setare sensibilitate ultrasonic (distantaMax)");
  Serial.println("2. Setare prag lumina (PragLDR)");
  Serial.println("3. Setare frecventa buzzer (buzzerFrecventa)");
  Serial.println("4. Schimbare parola");
  Serial.println("5. Revenire la meniul principal");
  Serial.println("------------------------ Tasteaza optiunea! --------------------");
}

float masurareDistanta() {
  digitalWrite(TrigPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TrigPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TrigPIN, LOW);

  unsigned long durata = pulseIn(EchoPIN, HIGH, 30000); 
  if (durata == 0) 
    return -1; 

  return durata * 0.034 / 2.0;
}

void setareUltrasonic() {
  if (!setareInCurs) 
    return; 
  
  unsigned long t = millis(); 

  if (t - ultimaMasurare >= 100) {
    ultimaMasurare = t;
    float d = masurareDistanta(); 
    if (d > 0) {
      sumaSetare += d; 
      numarCitiri++; 
      Serial.print("Masurare ");
      Serial.print(numarCitiri);
      Serial.print(": ");
      Serial.print(d);
      Serial.println(" cm");
    }

    if (numarCitiri >= 5) {
      distantaInitiala = sumaSetare / numarCitiri; 
      Serial.print("\nDistanta de baza: ");
      Serial.print(distantaInitiala);
      Serial.println(" cm");
      setareInCurs = false; 

      Serial.println("Setare finalizata. Sistem gata.");
      Serial.println("Sistemul e dezarmat. Armeaza sa fii protejat de CAT SPY");
      vreauAfisareMeniu = 1; 
      afisareMeniu(); 
    }
  }
}

void clearSerialBuffer() {
  if(Serial.available() > 0) 
    Serial.read(); 
}

void handleMenu() {
  if (!asteaptaParola && Serial.available() > 0 && !inSubmeniuSetari) {
    char opt = Serial.read();

    switch (opt) {
      case '1': 
        if (state == State::Dezarmat) {
          Serial.println("Armare manuala initiata...");
          timerAlarmare = millis(); 
          state = State::SeArmeaza;
        }
        break;

      case '2': 
        if (state == State::Dezarmat) 
          afisareSubmeniuSetari();
        break;

      case '3': 
        if (state == State::Dezarmat) {
          Serial.println("Pornire test alarma...");
          state = State::AlarmaActiva;
        }
        break;
    }
  }
}

void handleSettingsMenu() {
  static char opt = 0;
  static bool asteaptaValoare = false;
  static bool schimbParola = false; 
  static bool asteptParolaCurenta = false; 
  static bool asteptParolaNoua = false; 

  if (!inSubmeniuSetari) 
    return;

  if (!schimbParola && !asteaptaValoare && Serial.available()) {
    String optStr = Serial.readStringUntil('\n'); 
    optStr.trim();
    if (optStr.length() == 0)
      return;
    opt = optStr.charAt(0);
    Serial.print("Ai ales optiunea: ");
    Serial.println(opt);

    if (opt == '4') {
      Serial.println("Introdu parola curenta:");
      schimbParola = true;
      asteptParolaCurenta = true;
      asteptParolaNoua = false;
      asteaptaValoare = false;
      return; 
    }
    asteaptaValoare = true; 
  }

  if (schimbParola && Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (asteptParolaCurenta) { 
      if (input == parolaSetata) {
        Serial.println("Parola corecta. Introdu noua parola:");
        asteptParolaCurenta = false;
        asteptParolaNoua = true;
      } else {
        Serial.println("Parola gresita. Iesire din schimbare parola.");
        asteptParolaCurenta = false;
        asteptParolaNoua = false;
        schimbParola = false;
        afisareSubmeniuSetari();
      }
    } else if (asteptParolaNoua) {
      input.toCharArray(parolaSetata, sizeof(parolaSetata));
      Serial.println("Parola a fost actualizata. SPY CAT te saluta!");
      asteptParolaNoua = false;
      schimbParola = false;
      afisareSubmeniuSetari();
    }
  }

  if (asteaptaValoare && Serial.available()) {
    String valStr = Serial.readStringUntil('\n');
    valStr.trim();

    switch (opt) {
      case '1': 
        distantaMax = valStr.toInt();
        Serial.print("Toleranta setata la ");
        Serial.print(distantaMax);
        Serial.println(" cm");
        break;
      case '2': 
        PragLDR = valStr.toInt();
        Serial.print("Prag lumina setat la ");
        Serial.println(PragLDR);
        break;
      case '3': 
        buzzerFrecventa = valStr.toInt();
        Serial.print("Frecventa setata la ");
        Serial.print(buzzerFrecventa);
        Serial.println(" Hz");
        tone(BuzzerPIN, buzzerFrecventa, 500);
        break;
      case '5':
        Serial.println("Revenire la meniul principal.");
        vreauAfisareMeniu = 1;
        afisareMeniu();
        asteaptaValoare = false;
        opt = 0;
        inSubmeniuSetari = false;
        return;
      default:
        Serial.println("Optiune invalida!");
        afisareSubmeniuSetari();
        break;
    }
    asteaptaValoare = false;
    opt = 0;

    Serial.println();
    afisareSubmeniuSetari(); 
  }
}

void checkSensors() {
  unsigned long timerPrezent = millis(); 
  int luminaPrezent = analogRead(LDR); 
  float distanta = masurareDistanta(); 

  if (state == State::Dezarmat) {
    digitalWrite(Verde, HIGH);
    digitalWrite(Rosu, LOW); 
    noTone(BuzzerPIN); 

    if (luminaPrezent < PragLDR) {
      state = State::SeArmeaza;
      timerAlarmare = timerPrezent;
      Serial.println("Se armeaza automat (lumina scazuta)");
    }
  }

  if (state == State::SeArmeaza)
    if (timerPrezent - timerAlarmare >= 3000UL) { 
      state = State::Armat;
      Serial.println("Sistemul a fost armat.");
      digitalWrite(Verde, LOW);
      Serial.println("SPY CAT este pe pozitii");

      vreauAfisareMeniu=1;
      afisareMeniu();
    } else {
      digitalWrite(Verde, (timerPrezent / 500) % 2); 
    }

 if (state == State::Armat && distanta > 0 && distanta < 400) {
    float diferenta = fabs(distanta - distantaInitiala);
    if (diferenta > distantaMax) { 
      timerAlarmare = timerPrezent;
      state = State::PregatitDeActivare;
      Serial.println("SPY CAT a detectat un posibil intrus! Introduceti parola de siguranta:");
      asteaptaParola = true;
    }
  }

  if (state == State::PregatitDeActivare && (timerPrezent - timerAlarmare >= 3000UL) && asteaptaParola == true) {
    Serial.println("Parola neintrodusa la timp — alarma pornita!");
    state = State::AlarmaActiva;
    asteaptaParola = true;
    Serial.println("!!!!ALARMA A FOST ACTIVATA!!!!");
  }
}

void handlePassword() {
  if (Serial.available() > 0 && asteaptaParola) {
    char c = Serial.read();
    
    if (c == '\n' || c == '\r') { 
      parolaIntrodusa[indexParola] = '\0';

      if (strcmp(parolaIntrodusa, parolaSetata) == 0) {
        Serial.println("Parola corecta. Sistemul a fost dezarmat.");
        state = State::Dezarmat; 
        asteaptaParola = false;
        noTone(BuzzerPIN); 
        digitalWrite(Rosu, LOW);
        digitalWrite(Verde, HIGH);
        Serial.println("Sistemul este DEZARMAT");
      } else {
        Serial.println("Parola incorecta!");
        state = State::AlarmaActiva;
        asteaptaParola = true;
        Serial.println("Parola gresita – ALARMA ACTIVATA!");
      }
      indexParola = 0;
      parolaIntrodusa[0] = '\0';
    } else {
      if (indexParola < sizeof(parolaIntrodusa) - 1)
        parolaIntrodusa[indexParola++] = c; 
    }
  }
}

void updateAlarm() {
  if (state == State::AlarmaActiva) {
    static unsigned long bipaitura = 0;
    if (millis() - bipaitura > 300) { 
      bipaitura = millis();
      digitalWrite(Rosu, !digitalRead(Rosu)); 
    }
    tone(BuzzerPIN, buzzerFrecventa); 
  } else {
    noTone(BuzzerPIN); 
    digitalWrite(Rosu, LOW); 
  }
}

void loop() {
  if (setareInCurs) { 
    setareUltrasonic(); 
    return;
  }

  if (state == State::AlarmaActiva) { 
    handlePassword(); 
    updateAlarm(); 
  } else {
    handleMenu(); 
    handleSettingsMenu(); 
    checkSensors();
    handlePassword(); 
    updateAlarm(); 
  }
}
