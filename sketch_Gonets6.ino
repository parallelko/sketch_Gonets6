#include <SPI.h>   //подключаем библиотеку для работы с SPI
#include <Ethernet.h>  //Библиотека для работы в сети
#include <string.h> //Библиотека для работ со строками
#include <stdio.h> //Библиотека для преобразования
#include <EEPROM.h>
//Regexp Features
#include <Regexp.h>
#define MAXCAPTURES 4
#define REQ_BUF_SIZE 128

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
#define SELFIP_SHIFT 1
#define SENDIP_SHIFT 5
#define FROM_SHIFT 10
#define TO_SHIFT 20
//#define 

byte my_IP[] = {192,168,1,127};
byte send_IP[] = {192,168,1,55};
char fromTerm[5]= "1025";
char toTerm[5]= "7";
IPAddress GonetsIP(send_IP);//IP устройства назначения
uint32_t BRate_serial1 = 57600;

boolean parseconfig(const char *reqbuffer) {
  byte read_state = OUT_BODY;
  String strBuff = "";
  byte count = 0;
  byte ch;
  int num;
  for (byte i = 0; i < REQ_BUF_SIZE; i++ ) {
       switch (read_state) {
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
              my_IP[count] = num;
              count = 0;
              strBuff = "";
              read_state = IN_BODY;
              break;
            }
            if (reqbuffer[i] == '.') {
              num = atoi(strBuff.c_str());
              if (num > 255) return false;
              my_IP[count++] = num;
              if (count > 3) return false;
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
              my_IP[count] = num;
              count = 0;
              strBuff = "";
              read_state = IN_BODY;
              break;
            }
            if (reqbuffer[i] == '.') {
              num = atoi(strBuff.c_str());
              if (num > 255) return false;
              send_IP[count++] = num;
              if (count > 3) return false;
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
             if (sprintf(toTerm, "%s", strBuff.c_str()) > 0){
             strBuff = "";
             read_state = IN_BODY;
             break;
             }
             else return false;
          }
          else if (isdigit(reqbuffer[i])){
                  strBuff += reqbuffer[i];
                  break;
               }
               else return false;
         }
         case IN_FROM : {
             if (reqbuffer[i] == '&') {
              if(sprintf(fromTerm, "%s", strBuff.c_str()) > 0) {
              strBuff = "";
              read_state = IN_BODY;
              break;
              }
              else return false;
             }
             else if (isdigit(reqbuffer[i])){
                      strBuff += reqbuffer[i];
                      break;
                   }
                   else return false;
         }
         case IN_MODE : {
           if (isdigit(reqbuffer[i])){ 
            switch (atoi(reqbuffer[i])) {
             case 0 : {
               BRate_serial1 = 9600;
               break;
            }
            case 1 : {
               BRate_serial1 = 19200;
               break;
            }
            case 2 : {
               BRate_serial1 = 38400;
               break;
            }
            case 3 : {
               BRate_serial1 = 57600;
               break;
            }
            case 4 : {
               BRate_serial1 = 115200;
               break;
            }
           }
          }
          else return false;
         }
      }            
  }
  EEPRROMsetconfig();
  return true;
}

void EEPRROMsetconfig() {
  
}

void EEPROMreadconf() {
 byte chread = EEPROM.read(1);
 char ch;
 String readbuff = "";
 switch (chread) {
   case 0: {
   BRate_serial1 = 9600;
   break;
   }
   case 1: {
   BRate_serial1 = 19200;
   break;
   }
   case 2: {
   BRate_serial1 = 38400;
   break;
   }
   break;
   case 3: {
   BRate_serial1 = 57600;
   break;
   }
   case 4: {
   BRate_serial1 = 115200;
   break;
   }
   }
 for (byte i = 0; i < 4; i++) {
   my_IP[i] = EEPROM.read(SELFIP_SHIFT+i);
   send_IP[i] = EEPROM.read(SENDIP_SHIFT+i);
 }
 for (byte i = 0; i<5; i++){
   if ((ch=EEPROM.read(FROM_SHIFT+i)) !='\0') {
   readbuff += ch;
   }
  else break;
 }
 sprintf(fromTerm, "%s", readbuff.c_str());
 readbuff = "";
 for (byte i = 0; i<5; i++){
   if ((ch=EEPROM.read(TO_SHIFT+i)) != '\0') {
    readbuff += ch;
   }
   else break;
 }
   sprintf(toTerm, "%s", readbuff.c_str());         
}

void serialInit() {
  Serial.begin(9600);
  Serial1.begin(BRate_serial1);
  Serial1.setTimeout(200);
}

void GonetsHTTPsend(char *msg, char *fromTerminal, char *toTerminal,byte chSv) { //Функция для отправки сообщений в гонец
  EthernetClient client;
  //Подготовка строки к отправке
  byte ln = 0;
  byte tryN = 0;
  char sendbuff[256];
  ln = sprintf(sendbuff,"from=%s&to=%s&urgency=0&chSv=%d&subj=data&msg=%s",fromTerminal,toTerminal,chSv, msg); //Подготовка строки к отправке
   //Задаем 3 попытки для коннекта
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
        //Logthis("Connection failed");
        Serial.println("Connection failed");
        delay(300);
        tryN++;
       }
  }
  client.stop(); //закрываем соединение
} //GonetsHTTPsend

void serialWorks() { //Serial recieving and sending messages
    String serialReq = "";
    char *sendBuff;
    if (Serial1.available()>0){
      serialReq = Serial1.readString();
    }
    if (serialReq != ""){
    sendBuff = serialReq.c_str();
    GonetsHTTPsend(sendBuff,fromTerm,toTerm, 1);
    }
}

void setup() {
  if (EEPROM.read(0) != 0) {
    EEPROMreadconf();
  }
  serialInit();  
  serverInit();
}

void loop() {
  serverWorks();
  serialWorks();
}
