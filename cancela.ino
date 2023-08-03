/* RFID  
Pino SDA  ligado na porta 45 do MEGA 
Pino SCK  ligado na porta 52 do MEGA 
Pino MOSI ligado na porta 51 do MEGA
Pino MISO ligado na porta 50 do MEGA 
Pino NC – Não conectado
Pino GND  ligado no pino GND do MEGA
Pino RST ligado na porta 5 do MEGA
Pino 3.3 – ligado ao pino 3.3 V do MEGA 

*/

#include <MFRC522.h>
#include <SPI.h> //interna
#include <Servo.h>
#include <Ultrasonic.h>


#define RST_PIN  5 //PINO DE RESET
#define SS_PIN_II 45 //PINO SDA

#define NR_OF_READERS   1
#define InpOut_I 0 //Data Input
//#define InpOut_O 1 //Data Output

byte SSPins[] = {SS_PIN_II};

// Create an MFRC522 instance :
MFRC522 mfrc522[NR_OF_READERS];
String IDtag = "";
int max_uid = 255, i = 0;
String UID_I[255];
bool found;
bool hld;
bool insertedCard;




//Define os pinos para o trigger e echo
Ultrasonic sensorE1(22,23); //Sensor da Entrada Num 1
Ultrasonic sensorE2(24,25);
Ultrasonic sensorS1(26,27);
Ultrasonic sensorS2(28,29);

long dist_E1, dist_E2, dist_S1, dist_S2;  //Distancia do Sensor 1, 2, 3 e 4

#define servo_Ent 8
#define servo_Sai 9
#define buzzer_E 10
#define buzzer_S 11
Servo Entrada;
Servo Saida;
int pos_E; //posicao do Servo da Entrada
int pos_S;
int posicao;
int distancia = 5;

#define ledR_I 2 //Led Vermelho Entrada
#define ledG_I 3 //Led Vermelho Entrada
#define ledR_O 4 //Led Amarelo Saída
#define ledG_O 6 //Led Vermelho Saída


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); //observar os valores coletados
  pinMode(ledR_I,OUTPUT);
  pinMode(ledG_I,OUTPUT);
  pinMode(ledR_O,OUTPUT);
  pinMode(ledG_O,OUTPUT);
  pinMode(buzzer_E, OUTPUT);
  pinMode(buzzer_S, OUTPUT);
  Entrada.attach(servo_Ent);
  Entrada.write(0); //iniciar o servo E na posicao 0
  Saida.attach(servo_Sai);
  Saida.write(0);
  //attachInterrupt(digitalPinToInterrupt(6), ISR1, LOW);

  SPI.begin(); //INICIALIZA O BARRAMENTO SPI  
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {    
    mfrc522[reader].PCD_Init(SSPins[reader], RST_PIN);
    Serial.print(F("Reader "));
    Serial.print(reader);
    Serial.print(F(": "));
    mfrc522[reader].PCD_DumpVersionToSerial();  
  }

}


void loop() {
  // put your main code here, to run repeatedly: 
  digitalWrite(ledR_I,HIGH);
  digitalWrite(ledG_I,LOW);
  digitalWrite(ledR_O,HIGH);
  digitalWrite(ledG_O, LOW);
  Entrada.write(0);
  Saida.write(0);
  dist_E1 = sensorE1.Ranging(CM); //obter a distancia para o objeto

// teste sensor
//Serial.println(sensorS1.Ranging(CM));
///*

  delay(20);
  dist_S1 = sensorS1.Ranging(CM);  
  delay(20);         
  
  insertedCard = false;  
  controlTag (); 

  if (dist_E1 <= distancia || dist_S1 <= distancia || insertedCard) {
     verifDistancia();
  }    
   insertedCard = false;
 //  */

}

void verifDistancia() {    

  bool veiculo_E=false, veiculo_S=false;
  bool controle_entrada = false, controle_saida=false;
  
  bool flag = false;
  
  
  while (!flag) {    
  pos_S = 0;
  pos_E = 0;
  digitalWrite(ledR_I,HIGH);
  digitalWrite(ledG_I,LOW);  
  digitalWrite(ledR_O,HIGH);
  digitalWrite(ledG_O,LOW);  

   
  dist_E1 = sensorE1.Ranging(CM); //obter a distancia para o objeto
  delay(10);  
  dist_S1 = sensorS1.Ranging(CM);
  delay(10);
  
  if (insertedCard) {
      veiculo_E = true;
  }

  if (dist_E1 <= distancia ) {
    veiculo_E = true;        
  }
  if (dist_S1 <= distancia ) {
    veiculo_S = true;
  }
  
  
  bool abertura_E = false, abertura_S = false;  
  while ( veiculo_E || veiculo_S) {      
    posicao = 0;
    dist_E2 = sensorE2.Ranging(CM);
    delay(10);
    dist_S2 = sensorS2.Ranging(CM);  
    delay(10);
    while (posicao < 90 ) {    
      if (veiculo_E && dist_E2 > distancia ){
        Entrada.write(posicao); 
        abertura_E = true;         
      }
      if (veiculo_S && dist_S2 > distancia){
        Saida.write(posicao);        
        abertura_S = true;
      }
      
      if (sensorS1.Ranging(CM) <= distancia && !veiculo_S && dist_S2 > distancia){                                             
         if (pos_S < 90) {
           Saida.write(pos_S);                                                            
         }
         pos_S++;
      }         
      if (sensorE1.Ranging(CM) <= distancia && !veiculo_E && dist_E2 > distancia){                                    
         if (pos_E < 90) {
           Entrada.write(pos_E);            
         }            
         pos_E++;
      }
      delay(40);     
      posicao++;     
    }        
    break;   
  } 
   
  if (pos_E > 0) {        
    for (posicao = pos_E; posicao < 90; posicao++){          
      Entrada.write(posicao);                  
      veiculo_E = true;
      abertura_E = true;
      delay(40);
    }
  }  

  if (pos_S > 0) {        
    for (posicao = pos_S; posicao < 90; posicao++){    
      Saida.write(posicao);  
      veiculo_S = true;                
      abertura_S = true;
      delay(40);
    }
  
  } 


  if (veiculo_E && abertura_E ) {   
      digitalWrite(ledR_I,LOW);
      digitalWrite(ledG_I,HIGH);      
    }  
  if (veiculo_S && abertura_S ) {  
      digitalWrite(ledR_O,LOW);
      digitalWrite(ledG_O,HIGH);
    }     
      
  //controle de travessia

  if (veiculo_E || veiculo_S){        
    int contador = 0;    
    while (contador < 5000){      
      dist_E1 = sensorE1.Ranging(CM); //obter a distancia para o objeto
      delay(10);
      dist_E2 = sensorE2.Ranging(CM);
      delay(10);
      dist_S1 = sensorS1.Ranging(CM);
      delay(10);
      dist_S2 = sensorS2.Ranging(CM);  
      delay(10);  
      delay(260);
      contador = contador + 300;
    }    
    if (veiculo_E ) {
          digitalWrite(ledR_I,HIGH);
          digitalWrite(ledG_I,LOW);
    }  
    if (veiculo_S) {  
          digitalWrite(ledR_O,HIGH);
          digitalWrite(ledG_O,LOW);
    }
        
    
    controle_entrada =false; controle_saida=false;  
    noTone(buzzer_E);
    noTone(buzzer_S);     
    for (posicao = 90; posicao >0; posicao--){                                  
      if (veiculo_E && abertura_E){                    
        Entrada.write(posicao);
        tone(buzzer_E,392);                        
        delay(10);
        noTone(buzzer_E);
        controle_entrada = true;
      }       
      if (veiculo_S && abertura_S){  
        Saida.write(posicao);   
        tone(buzzer_S,392);
        delay(10);
        noTone(buzzer_S);
        controle_saida = true;                 
      }
      delay(100);
    }
           
    
    if ( veiculo_E && dist_E2 > distancia && controle_entrada){            
      delay(2000);   
      veiculo_E = false;
    }
    
    if (veiculo_S && dist_S2 > distancia && controle_saida){            
      veiculo_S = false;
      delay(2000);
    }    
     
  }
 
  if (!veiculo_E && !veiculo_S){    
    flag = true;    
  }  
}
}

void controlTag() {    
    
    for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
      if (mfrc522[reader].PICC_IsNewCardPresent() && mfrc522[reader].PICC_ReadCardSerial()) {
      // display UID in serial       
      Serial.print(F("Reader - "));
      Serial.print(reader);      
      Serial.print(F(": Card UID:"));    

      /***START BLOCK OF CODE RESPONSIBLE FOR GENERATING THE RFID TAG READ  I***/
      String IDtag = "";       
      for (byte i = 0; i < mfrc522[reader].uid.size; i++) {    
       IDtag += (mfrc522[reader].uid.uidByte[i] < 0x10 ? "0" : "") 
       + String(mfrc522[reader].uid.uidByte[i], HEX) +
       (i!=3 ? ":" : "");
      }     
      IDtag.toUpperCase();
      Serial.println(IDtag);      
      sectionID(IDtag,reader);   
     }
    mfrc522[reader].PICC_HaltA(); //CARD READING STOP
    mfrc522[reader].PCD_StopCrypto1(); //ENCRYPTION STOP IN PCD          
    }        
}

void sectionID(String uid, int readerNumber){     
   found = false;
   for (i = 0; i <= max_uid; i++){     
    if (UID_I[i].equals(uid)){ 
      insertedCard = true;            
      found = true;
    }
   }
   if (!found && (readerNumber == InpOut_I)) {       
    for (i = 0; i <= max_uid; i++){  
      if (UID_I[i] == ""){
          UID_I[i] = uid;
          i = max_uid + 1;
          insertedCard = true;
      }
    }     
   }

/*
  if (found && (readerNumber == InpOut_O)) {    
    for (i = 0; i <= max_uid; i++){  
     if (UID_I[i].equals(uid)){               
        UID_I[i] = "";
        i = max_uid + 1;
      }
    } 
  } 
  */
   //CHECK  
   Serial.println(" ****Cards********  ");
   hld = false;
   for (i=0; i<=max_uid; i++){
    if (UID_I[i] != ""){
       hld = true;
       Serial.print(UID_I[i]);  
       Serial.print("-");
       Serial.println(i);        
    }
   }
   

   Serial.println(" **** END ********  ");    
}


void ISR1()
{

}