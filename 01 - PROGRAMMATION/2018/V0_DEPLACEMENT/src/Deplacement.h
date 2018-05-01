#include <AccelStepper.h>
#include <MultiStepper.h>
#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include <FastCRC.h>
//#include <Capteur.h>

//Adresse I2C du module de navigation
#define ADRESSE 60

//Etat des d�placements
#define FINI 0
#define EN_COURS 1
#define PREVU 2


//Etat de la nouvelle position demand�e
#define VALIDEE 0 // Nouvelle position valid�e et prise en compte
#define DISPONIBLE 1 // Nouvelle position enregistr�e
#define ERRONEE 2 // nouvelle position erron�e. CRC nok.



const int16_t centerOffset[2] = {0, 0};

//Variable Moteur
const int pinStep1=4;
const int pinDir1=5;
const int pinSleep1=3;
const int pinReset1=2;

const int pinStep2=8;
const int pinDir2=9;
const int pinSleep2=7;
const int pinReset2=6;


AccelStepper MGauche(AccelStepper::DRIVER,pinStep1, pinDir1);
AccelStepper MDroit(AccelStepper::DRIVER,pinStep2, pinDir2);


//Variable I2C
FastCRC8 CRC8;
byte bufNavRelatif[6]={0,0,0,0,0,0}; // Buffer de reception des ordres de navigation relatifs + le CRC
byte crcNavRelatif = 0; // CRC de controle des ordres de navigation relatifs
byte bufNavAbsolu[8]={0,0,0,0,0,0,0,0}; // Buffer de reception des ordres de navigation relatifs + le CRC
byte crcNavAbsolu = 0; // CRC de controle des ordres de navigation relatifs



byte fonction ;
int16_t relativeRequest[2] ; // rotation, distance
int16_t absoluteRequest[3] ; // rotation, X, Y
int16_t currentPos[3]; //rotation, X, Y

byte newPos = VALIDEE;

// Declaration des broches d'ES
// Broches analogiques :
int ana_1 = A6 ; // 1 - pin 20 - PWM
int ana_2 = A7 ; // 2 - pin 21 - PWM
int ana_3 = A8 ; // 3 - pin 22 - PWM
int ana_4 = A9 ; // 4 - pin 23 - PWM
int ana_5 = A0 ; // 5 - pin 14
int ana_6 = A1 ; // 6 - pin 15
int ana_7 = A2 ; // 7 - pin 16
int ana_8 = A3 ; // 8 - pin 17
// Broches numeriques : ( Utilisé par le module de moteurs pas-à-pas )
// int digi_1 = 5 ; // 1 - PWM
// int digi_2 = 4 ; // 2 - PWM
// int digi_3 = 3 ; // 3 - PWM
// int digi_4 = 2 ; // 4
// int digi_5 = 9 ; // 5 - PWM
// int digi_6 = 8 ; // 6
// int digi_7 = 7 ; // 7
// int digi_8 = 6 ; // 8 - PWM


bool PRESENCE_ARRIERE = 0, PRESENCE_AVANT = 0;
int ADVERSAIRE_ARRIERE = 22;
int ADVERSAIRE_AVANT = 23;

double AskX, AskRot, TempGauche, TempDroit, NewX, NewRot ;

int sensorTime = 2000;
int avantTimeInit = 0;
int arriereTimeInit = 0;

bool optionAdversaire = false;
bool optionRecalage = false;
bool optionRalentit = false;

char etatRotation, etatAvance;
bool etatABS = false;
bool etatLastRot = false;

int16_t targetRot = 0;

const float FacteurX= 1.09; //Ancien : 154.8
const float FacteurDroit = 8.0; //Ancien : 154.8
const float FacteurGauche = 8.0; //Ancien : 154.8
const float FacteurRot = 4.63; //Ancien : 19.64

const float VitesseMaxDroite = 1800.0; //Ancien : 8000
const float VitesseMaxGauche = 1800.0; //Ancien : 8000
const float VitesseMinDroite = 800.0; //Ancien : 5000
const float VitesseMinGauche = 800.0; //Ancien : 5000
const float AccelRot = 800.0; //Ancien : 2000
const float AccelMin = 300.0; //Ancien : 2000
const float AccelMax = 800.0; //Ancien : 5000
const float AccelStop = 2000.0; //Ancien : 8000

byte BORDURE = 0 ;
// AV_DROIT , AV_GAUCHE , AR_DROIT , AR_GAUCHE
//int PIN_BORDURE[4] = {20,17,16,21};

void updatePos();

void turnGo();

void goTo();

void recalage();

void bordure();

void adversaire();

//Fin de match
void FIN_MATCH();

void receiveEvent(int howMany);
void requestEvent();
