//****************************************************
// Projet :
//    Allumage progressif d'un escalier, marche par marche via ruban LED.
//    Ruban LED positionné sous chacune des marches, en haut de la contre-marche.
// Principe :
//    Des capteurs de présence Haut et Bas permettent de définir si une personne entre ou sort de l'escalier. 
//    Un capteur centrale permet de définir si quelqu'un est toujours présent dans l'escalier.
// Copyright :
//    Dominique PAUL
//    Création :05/12/2015 
//****************************************************
//
#define ALLUME LOW   // Les valeurs sont inversées sur les relais on défini donc nos propres variables
#define ETEINT HIGH  // Les valeurs sont inversées sur les relais on défini donc nos propres variables
#define PIR_BAS 2    // Défini sur quel pin est connecté le PIR du Bas
#define PIR_HAUT 3   // Défini sur quel pin est connecté le PIR du Haut
#define PIR_VERIF 4  // Défini sur quel pin est connecté le PIR de sécurité génral à l'escalier.

// On défini les variables de paramètrage de l'application.
int nbrPins = 14;    // On enregistre le nombre de pins (= nbre de marche)
int myPins[] =  {6, 7, 8, 9, 10, 11, 12, 13, A0, A1, A2, A3, A4, A5};
int timer = 50;     // Délai d'allumage ou extinction entre chaque marches.

// On défini les variables qui permettront d'enregistrer l'etat des éléments.
int varAllume = 0;           // Défini si l'escalier est allumé (1) ou éteint (0).
int TempoMaxi = 5;           // Temps maxi avant extinction si l'escalier ne s'est pas étaeint et que plus personne n'est détecté.
char* eteintSens = "";       // Sens d'extinction enregistré dans l'application ("haut-bas" ou "bas-haut").
int TempoEteint = 000;      // Tempo (en ms) avant extinction lors de la sortie de l'escalier.

void setup() {
  Serial.begin(115200);
  // On initialise les pins Des Leds en sortie, et on les éteint.
  int i;
  for (i = 0; i < nbrPins; i = i + 1) {
    pinMode(myPins[i], OUTPUT);
    digitalWrite(myPins[i], ETEINT);
  }
  
  // On initialise les pins des détecteurs PIR en entrée.
  pinMode(PIR_BAS, INPUT);
  pinMode(PIR_HAUT, INPUT);
  pinMode(PIR_VERIF, INPUT);
  delay(10000);   // Délai de calibration des PIR (30s)
  
  // On fait clignoter les LED pour annoncer la fin d'initialisation.
  finInit();
}

void loop() {
  // On vérifie sur le PIR Haut est détecté ?
  if (digitalRead(PIR_HAUT) == HIGH) {
    // PIR Haut détecté, On effectue la fonction "PIR Haut détecté".
    Serial.print("PIR Haut detecte");
    debug();
    pirHautDetecte();
    
  } else if (digitalRead(PIR_BAS) == HIGH) { // On vérifie sur le PIR Bas est détecté ?
    // PIR Bas détecté, On effectue la fonction "PIR Bas détecté".
    Serial.print("PIR Bas detecte");
    debug();
    pirBasDetecte();
    
  } else if (varAllume == 1) { // Est-ce que l'escalier est allumé ?
    // Les LED de l'escalier sont allumée.
    verifEteindre();
  }
  
  // On active une pause de 1s.
  delay(1000);
}

void pirHautDetecte() {
  if (varAllume == 1) {   // On vérifie si l'escalier est allumé.
    // Escalier allumé, en enregistre le sens d'extinction, et on vérifie si on doit l'éteindre.
    eteintSens = "bas-haut";   // On enregistre l'ordre d'extinction.
    testEteintLed();           // On va vérifier si quelqu'un est dans l'escalier.
  } else {  // Escalier éteint, on l'allume.
    anime(ALLUME,"haut-bas");
    varAllume = 1;             // On enregistre que l'escalier est allumé.
    eteintSens = "bas-haut";   // On enregistre l'ordre d'extinction.
    delay(2000);               // Pause de 2s, c'est le temps de "collage" du PIR.
  }
}

void pirBasDetecte() {
  if (varAllume == 1) {   // On vérifie si l'escalier est allumé.
    // Escalier allumé, en enregistre le sens d'extinction, et on vérifie si on doit l'éteindre.
    eteintSens = "haut-bas";   // On enregistre l'ordre d'extinction.
    testEteintLed();           // On va vérifier si quelqu'un est dans l'escalier.
  } else {  // Escalier éteint, on l'allume.
    anime(ALLUME,"bas-haut");
    varAllume = 1;             // On enregistre que l'escalier est allumé.
    eteintSens = "haut-bas";   // On enregistre l'ordre d'extinction.
    delay(2000);               // Pause de 2s, c'est le temps de "collage" du PIR.
  }
}

void verifEteindre() {
  // L'escalier étant allumé, on vérifie si il reste quelqu'un de présent.
  Serial.print("verifEteindre : Verif presence escalier");
  debug();
  if (digitalRead(PIR_VERIF) == HIGH) {
    // Quelqu'un est présent.
    Serial.print("Presence PIR verif escalier");
    debug();
  } else if (digitalRead(PIR_HAUT) == LOW && digitalRead(PIR_BAS) == LOW) {
    // Plus personne présent dans l'escalier.
    Serial.println("Escalier allume et aucune detection de mouvement");
    // On vérifie si la tempo est arrivée à son maximum.
    eteindreLed(TempoEteint);
  }
}

void testEteintLed() {
  // On vérifie si il y a une présence dans l'escalier.
  if (digitalRead(PIR_VERIF) != HIGH && digitalRead(PIR_HAUT) != HIGH && digitalRead(PIR_BAS) != HIGH) {
    // Personne dans l'escalier, on éteint les LED.
    eteindreLed(TempoEteint);   // Redirection vers la fonction qui éteint l'escalier.
  }
}

void eteindreLed(int Tempo) {
  // On éteint les LED.
  delay(Tempo);              // Tempo avant exteinction des LED.
  anime(ETEINT, eteintSens); // Extinction des LED.
  varAllume = 0;             // On déclare que l'escalier est éteint.
}

void anime(int ordre, char* sens) {
  // Annime l'allumage ou extinction, de bas en haut, ou de haut en bas suivant variables.
  int i;
  if (sens == "bas-haut") {
    for (i = 0; i < nbrPins; i = i + 1) {
      digitalWrite(myPins[i], ordre);
      delay(timer);
    }
  } else {
    for (i = nbrPins-1; i >= 0; i = i - 1) {
      digitalWrite(myPins[i], ordre);
      delay(timer);
    }
  }
  if (ordre != 1) {
    Serial.println("Allume escalier");
  } else {
    Serial.println("Eteint escalier");
    Serial.println(" ");
  }
}

void finInit() {
  // Fin d'initialisation, On fait clignoter 2 fois les LED pour l'annoncer.
  int timeTemp = 20;
  anime(ALLUME,"bas-haut");
  delay(timeTemp);
  anime(ETEINT,"bas-haut");
  delay(timeTemp);
  anime(ALLUME,"haut-bas");
  delay(timeTemp);
  anime(ETEINT,"haut-bas");
  delay(timeTemp);
}

void debug() {
  Serial.print(", varAllume = ");
  Serial.print(varAllume);
  Serial.print(", Sens = ");
  Serial.println(eteintSens);
}
