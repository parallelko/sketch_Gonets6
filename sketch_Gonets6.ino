#include <SPI.h>   //подключаем библиотеку для работы с SPI
#include <Ethernet.h>  //Библиотека для работы в сети
#include <string.h> //Библиотека для работ со строками
#include <stdio.h> //Библиотека для преобразования
#include <EEPROM.h> //Библиотека для работы с памятью

//my defines
#define MAX_NUM_ATTEMPTS 3
#define GONETS_PORT 80
#define OUT_BODY 0
#define IN_BODY 1
#define IN_SELF_IP 2
#define IN_DEST_IP 3
#define IN_FROM 4
#define IN_TO 5
#define IN_MODE 6
#define ENDING 7
#define SELFIP_SHIFT 1
#define SENDIP_SHIFT 5
#define FROM_SHIFT 10
#define TO_SHIFT 20
#define MODE_SHIFT 30
#define REQ_BUF_SIZE 128
#define EEPROM_CHECK 0
#define POWER_UP_PIN 7
#define SEND_BUFFER_LENGHT 128
#define WAITING_MSG 0
#define WARMUP 1
#define WAIT_FOR_TRANSMIT 2
#define SERIAL_READY 1
#define SERIAL_BUSY 0

int fromTerm = 1025;
int toTerm = 7;
byte my_IP[] = {192,168,1,127};
byte send_IP[] = {192,168,1,55};
IPAddress GonetsIP(send_IP);//IP устройства назначения
uint8_t Serial_MODE = SERIAL_READY;
char sendBuff[SEND_BUFFER_LENGHT];
uint8_t main_state = WAITING_MSG ;

boolean parseconfig(const char *reqbuffer) {
  byte read_state = OUT_BODY;
  String strBuff = "";
  byte count = 0;
  byte ch;
  //char *s;
  int num;
  for (byte i = 0; i < REQ_BUF_SIZE; i++ ) {
       switch (read_state) {
         case ENDING : {
          break;
         }
         case OUT_BODY : {
          if (reqbuffer[i] == '?') read_state = IN_BODY;
          break;
         }
         case IN_BODY : {
          if (reqbuffer[i] != '=') {
            strBuff += reqbuffer[i];
            break;
          }
          else {
            if (strBuff == "ip"){
              read_state = IN_SELF_IP;
              strBuff = "";
              break;
            }  
            else if (strBuff == "ip_dest") {
              read_state = IN_DEST_IP;
              strBuff = "";
              break;
            }  
            else if (strBuff == "from") {
              read_state = IN_FROM;
              strBuff = "";
              break;
            }  
            else if (strBuff == "to") {
              read_state = IN_TO;
              strBuff = "";
              break;
            }
            else if (strBuff == "mode") {
              read_state = IN_MODE;
              strBuff = "";
              break;
            }  
            else return false;
          }
         }
         case IN_SELF_IP :{
            if (reqbuffer[i] == '&') {
              num = atoi(strBuff.c_str());
              if (num > 255) return false;
              my_IP[3] = num;
              count = 0;
              strBuff = "";
              read_state = IN_BODY;
              break;
            }
            if (reqbuffer[i] == '.') {
              if (count > 4) return false;
              num = atoi(strBuff.c_str());
              if (num > 255) return false;
              my_IP[count] = num;
              strBuff = "";
              count++;
              break;
            }
            else if (isdigit(reqbuffer[i])){
                  strBuff += reqbuffer[i];
                  break;
                  }
                  else return false;
                        
         }
         case IN_DEST_IP : {
            if (reqbuffer[i] == '&') {
              num = atoi(strBuff.c_str());
              if (num > 255) return false;
              send_IP[3] = num;
              count = 0;
              strBuff = "";
              read_state = IN_BODY;
              break;
            }
            if (reqbuffer[i] == '.') {
              if (count > 4) return false;
              num = atoi(strBuff.c_str());
              if (num > 255) return false;
              send_IP[count] = num;
              strBuff = "";
              count++;
              break;
            }
            else if (isdigit(reqbuffer[i])){
                  strBuff += reqbuffer[i];
                  break;
                  }
                  else return false;
                        
         }
         case IN_FROM : {
             if (reqbuffer[i] == '&') {
              fromTerm = atoi(strBuff.c_str());
              strBuff = "";
              read_state = IN_BODY;
              break;
             }
             else if (isdigit(reqbuffer[i])){
                      strBuff += reqbuffer[i];
                      break;
                  }
                  else return false;
         }
         case IN_TO : {
             if (reqbuffer[i] == '&') {
              toTerm = atoi(strBuff.c_str());
              strBuff = "";
              read_state = IN_BODY;
              break;
             }
             else if (isdigit(reqbuffer[i])){
                      strBuff += reqbuffer[i];
                      break;
                  }
                  else return false;
         }
         case IN_MODE : {
           if (isdigit(reqbuffer[i])){ 
              char s[]= {reqbuffer[i], '\0'} ;
              Serial_MODE = atoi(s);
              read_state = ENDING;
           }
           else return false;
           break;
         }
      }            
  }
  EEPRROMsetconfig();
  return true;
}

void EEPRROMsetconfig() {
 EEPROM.write(MODE_SHIFT, Serial_MODE);
 delay(50);
 for (byte i = 0; i<4; i++) {
 EEPROM.write(SELFIP_SHIFT+i, my_IP[i]);
 delay(50);
 EEPROM.write(SENDIP_SHIFT+i, send_IP[i]);
 delay(50);
 }
 EEPROM.write(FROM_SHIFT, highByte(fromTerm));
 delay(50);
 EEPROM.write(FROM_SHIFT+1, lowByte(fromTerm));
 delay(50);
 EEPROM.write(TO_SHIFT, highByte(toTerm));
 delay(50);
 EEPROM.write(TO_SHIFT+1, lowByte(toTerm));
 delay(50);
 EEPROM.write(EEPROM_CHECK, 1);
 delay(50);
}

void EEPROMreadconf() {
 Serial_MODE = EEPROM.read(MODE_SHIFT);
 for (byte i = 0; i < 4; i++) {
   my_IP[i] = EEPROM.read(SELFIP_SHIFT+i);
 }
 for (byte i = 0; i < 4; i++) {
   send_IP[i] = EEPROM.read(SENDIP_SHIFT+i);
 }
 fromTerm = word(EEPROM.read(FROM_SHIFT), EEPROM.read(FROM_SHIFT+1));
 toTerm = word(EEPROM.read(TO_SHIFT), EEPROM.read(TO_SHIFT+1));
}

void statusProcess() {
  switch (main_state) {
    case WAITING_MSG: {
      if (serial_state == SERIAL_BUSY) {
        main_state = WARMUP;
        digitalWrite(POWER_UP_PIN, HIGH);
        break;
      }
      else break;
    }
    case WARMUP : {
      if 
      //Waiting for 10 min
    }
    case WAIT_FOR_TRANSMIT : {
      //Waiting for transmit
    }
    
   }    
}

void serialInit() {
  //Serial.begin(9600);
  switch (Serial_MODE) {
    case 0 : {
      Serial1.begin(9600);
      break;
    }
    case 1 : {
      Serial1.begin(19200);
      break;
    }
    case 2 : {
      Serial1.begin(38400);
      break;
    }
    case 3 : {
      Serial1.begin(57600);
      break;
    }
    case 4 : {
      Serial1.begin(115200);
      break;
    }
  }
  Serial1.setTimeout(200);
}

void GonetsHTTPsend(char *msg, int fromTerminal, int toTerminal,byte chSv) { //Функция для отправки сообщений в гонец
  EthernetClient client;
  //Подготовка строки к отправке
  byte ln = 0;
  byte tryN = 0;
  char sendbuff[256];
 // Serial.println(msg);
  ln = sprintf(sendbuff,"from=%d&to=%d&urgency=0&chSv=%d&subj=data&msg=%s",fromTerminal,toTerminal,chSv, msg); //Подготовка строки к отправке
   //Задаем 3 попытки для коннекта
  Serial.println(sendbuff);
  while (tryN<MAX_NUM_ATTEMPTS){
    if (client.connect(GonetsIP, GONETS_PORT)) {
        // HTTP-Reqest:
        Serial.println(F("Connected"));
        client.println(F("POST /sendmsg.htm HTTP/1.1"));
        client.print(F("Host: "));
        client.println(ipString(send_IP).c_str());
        client.println(F("Connection: keep-alive"));
        client.print(F("Content-Length: "));
        client.println(ln);
        client.println();
        client.print(sendbuff);
        delay(100);
        if (recieveAnswer(client)) Serial.println("Message sent");
        break;
       }
       else
       {
        Serial.println("Connection failed");
        delay(300);
        tryN++;
       }
  }
  client.stop(); //закрываем соединение
} //GonetsHTTPsend

void serialWorks() { //Serial recieving and sending messages
    String serialReq = "";
    if (Serial1.available()>0){
      serialReq = Serial1.readString();
    }
    if (serialReq != "" && serial_state == SERIAL_READY) {
    strcpy(sendBuff, serialReq.c_str());
    serial_state = SERIAL_BUSY;
    //GonetsHTTPsend(sendBuff,fromTerm,toTerm, 1);
    }
}

void setup() {
 Serial.begin(9600);  
 if (EEPROM.read(EEPROM_CHECK) != 0) {
   EEPROMreadconf();
 }
 serialInit();  
 serverInit();
 ADCSRA = 0;
 Serial.println("Ready...");
  
}

void loop() {
  statusProcess();
  serverWorks();
  serialWorks();
}
