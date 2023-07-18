# Smart-home
Modello progetto IoT che prevede l'interazione tra microcontrollori Arduino ed ST e processo Java con comunicazione su Telegram. Per adattare il modello alle esigenze bisogna inserire la stringa identificativa della porta seriale di Arduino e di ST (file SerialPortArduino e SerialPortSt, istruzione serialPort = SerialPort.getCommPort("xxxxxxxx")), valorizzare l'attributo chatId e aggiustare il ritorno dei metodi getBotToken e getBotusername nel file SmartHomeBot

## COMPONENTI UTILIZZATI 
•  Scheda di sviluppo Stm32 Nucleo F401RE 
•  Scheda di sviluppo Arduino Uno 
•  Sensore di distanza ad ultrasuoni HC-SR04 (2) 
•  Sensore di movimento ad infrarossi PIR 
•  Interruttore ottico 
•  Sensore di livello dell’acqua 
•  Pompa idraulica 
•  Buzzer attivo 
•  Servomotore SG90 
•  Motore stepper 28BYJ-48 (pilotato da ULN2003) 
•  Motore DC con ventola (pilotato da L293D) 
•  Sensore di temperatura DHT11 
•  Fotoresistenza 
•  Tastierino numerico 4*4 
•  Display LCD 16*2 
•  Transistor NPN (2) 
•  Power Supply con batteria da 9V (5) 
•  Led di indicazione stato (6) 
•  Pulsanti di attivazione funzionalità (7) 

## CONNETTIVITÀ 
•  I2C: comunicazione tra board ST e display 
•  UART: comunicazione tra le schede di sviluppo e il PC 
•  Telegram Bot: monitoraggio da remoto delle funzionalità 

## SOFTWARE UTILIZZATI 
•  Eclipse 
o  gestione interazione tra le schede di sviluppo (prive di connettività ad Internet) e bot 
Telegram 
o  gestione comunicazione tramite porta seriale tra le due schede 
•  Arduino IDE: ambiente di sviluppo scheda Arduino 
•  STM32CubeIDE: ambiente di sviluppo scheda ST 

## LIBRERIE ESTERNE 
•  KeyPad - http://www.github.com/NimaLTD 
•  i2c-lcd - https://controllerstech.com/i2c-lcd-in-stm32/ 
•  SimpleDHT – Arduino Library Manager 
•  jSerialComm - https://mvnrepository.com/artifact/com.fazecast/jSerialComm/2.9.3 

## FUNZIONALITÀ PREVISTE 
•  Cancello automatico 
o  scheda di sviluppo ST 
o  motore stepper o  attivazione tramite pulsante 
o  rilevazione ostacolo in fase di chiusura (interruttore ottico) 
o  rilevazione distanza del cancello dai punti di fine corsa (sensori HC-SR04) 
o  comunicazione stato cancello 
•  Fontana d’acqua 
o  scheda di sviluppo Arduino 
o  rilevazione livello dell’acqua 
o  funzionamento temporizzato 
o  comunicazione stato fontana 
o  attivazione tramite pulsante 
•  Luci intelligenti 
o  Interna 
▪  scheda di sviluppo ST 
▪  attivazione tramite rilevazione movimento (sensore PIR) 
▪  funzionamento temporizzato 
▪  comunicazione stato luce 
o  esterne 
▪  scheda di sviluppo Arduino 
▪  attivazione e spegnimento tramite rilevazione luminosità (fotoresistenza) 
▪  comunicazione stato luce 
•  Porta  
o  scheda di sviluppo ST 
o  servomotore 
o  attivazione tramite pulsante 
o  apertura temporizzata 
o  comunicazione stato porta 
•  Sistema di allarme 
o  scheda di sviluppo ST 
o  attivato e disattivato tramite tastierino numerico 
o  comunicazione stato allarme 
•  Sistema di condizionamento 
o  scheda di sviluppo Arduino 
o  rilevazione temperatura (sensore DHT11) 
o  attivazione automatica 
o  comunicazione stato 
•  Configurazione 
o  scheda di sviluppo ST 
o  attivato tramite pulsante 
o  disattivato tramite tastierino numerico 
o  comunicazione parametri 
Ad ogni cambiamento di stato di una delle funzionalità corrisponde una notifica inviata sul bot Telegram 

## COMANDI TELEGRAM 
•  /get_info_smart_home 

## FORMATO PACCHETTO UART 
La comunicazione prevede 5 campi, ciascuno codificato con 1 byte: StartOfFrame, Operazione, 
Identificativo, Valore, EndOfFrame. Ad ogni componente è stato associato un identificativo univoco. Per ogni componente è stato definito un set predefinito di valori possibili. Le operazioni previste sono 
GET_INFO (recupero dei dati campionati e dei valori di parametrizzazione), SEND_VALUE (invio dati 
campionati e cambi di stato) e SET_PARAMETER (invio dati di parametrizzazione operazioni). Il pacchetto 
GET_INFO parte dal pc e le due schede rispondono (per ovvie ragioni i campi Identificativo e Valore sono 
valorizzati con del padding (‘s’)). I pacchetti SEND_VALUE e SET_PARAMETER partono dalle schede di 
sviluppo e il pc invia il messaggio sul bot. Gli identificativi univoci previsti sono 
•  StartOfFrame: ‘a’ 
•  Operazione: 
o  GET_INFO: ‘b’ 
o  SEND_VALUE: ‘c’ 
o  SET_PARAMETER ‘d’ 
•  Identificativo sensore: 
o  Cancello: ‘e’ 
o  Fontana: ‘f’ 
o  luce_interna: ‘g’ 
o  luce_esterna: ‘h’ 
o  porta: ‘i’ 
o  allarme: ‘j’ 
o  temperatura: ‘k’ 
o  configurazione_cancello: ‘l’ 
o  configurazione_fontana: ‘m’ 
o  configurazione_luce: ‘n’ 
o  configurazione_porta: ‘o’ 
o  configurazione_temperatura: ‘p’ 
o  sistema_condizionamento: ‘q’ 
•  Valore sensore: 
o  cancello: chiuso (‘0’), in apertura (‘1’), aperto (‘2’), in chiusura (‘3’) 
o  fontana: spenta (‘0’), accesa (‘1’), malfunzionamento (‘2’) 
o  luce_interna: spenta (‘0’), accesa (‘1’) 
o  luce_esterna: spenta (‘0’), accesa (‘1’) 
o  porta: chiusa (‘0’), aperta (‘1’) 
o  allarme: spento (‘0’), acceso ma non attivo (‘1’), acceso ed attivo (‘2’) 
o  temperatura: valore rilevato (3 byte al massimo): invio ogni cifra numerica come carattere 
o  configurazione: valore inserito (2 byte al massimo), invio ogni cifra numerica come 
carattere 
o  sistema_condizionamento: spento (‘0’), acceso (‘1’) 
•  EndOfFrame: ‘r’ 
Il numero di byte scambiati tra le due board è 7 (valore con 3 caratteri di temperatura e configurazione 
dati) nel peggiore dei casi e 5 nel migliore dei casi. Per questo motivo vengono inviati sempre 7 byte 
riempendo i 2 byte mancanti con padding (‘s’). 
