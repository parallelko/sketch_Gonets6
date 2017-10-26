#include <SPI.h>   //подключаем библиотеку для работы с SPI
#include <Ethernet.h>  //Библиотека для работы в сети
#include <string.h> //Библиотека для работ со строками
#include <stdio.h> //Библиотека для преобразования
#include <EEPROM.h>
//Regexp Features
#include <Regexp.h>
#define MAXCAPTURES 4

//Server Features
#define SERVER_PORT 80
EthernetServer server(SERVER_PORT);

//my defines
#define MAX_NUM_ATTEMPTS 3
#define GONETS_PORT 80
#define NORMAL_STATE 0
#define SD_INIT_FAIL 1

// HTTP request
#define REQ_BUF_SIZE 128
char HTTP_req[REQ_BUF_SIZE] = {0}; // null terminated string
int reqIndex = 0;

// String request
#define MAX_LEN_STRING  128
#define MAX_LEN_REQUEST 256 // 512
String request = "";

#define GET           "GET /"
#define INDEX_STR     "index"
#define HTM_EXT       ".HTM"

uint16_t BRate_serial1 = 57600;

/* Authorization
#define AUTH_OFF 0
#define AUTH_ON  1
byte authMode = AUTH_OFF;
// Online encode to Base64: base64encode.org
String AUTH_HASH = "Authorization: Basic YWRtaW46YW1z"; // admin:ams
*/

void StrClear(char *str, char length) {
  for (int i = 0; i < length; i++) {
    str[i] = 0;
  }
}

char StrContains(char *str, char *sfind) {
  char found = 0;
  char index = 0;
  char len;

  len = strlen(str);
  if (strlen(sfind) > len) {return 0;}
  
  while (index < len) {
    if (str[index] == sfind[found]) {
      found++;
      if (strlen(sfind) == found) {return 1;}
    } else {
        found = 0;
      }
    index++;
  }
  return 0;
}

// server answers
String makeAnswer(String content) {
  String s = "";
  s += F("HTTP/1.1 200 OK\n");
  s += F("Content-Type: ");
  s += content;
  s += F("\n");
  s += F("Connnection: close\n"); // "Connection: keep-alive\n"
  return s;
}

void sendHtmlAnswer(EthernetClient cl) {
  cl.println(makeAnswer(F("text/html")));
}

void sendErrorAnswer(char mess[], EthernetClient cl) {
  cl.print(mess);
  cl.println(F(" ERROR"));
  cl.println(F("Connnection: close"));
  cl.println();
}
 
 void parseRequest(EthernetClient cl) {
  // index request
  if (StrContains(HTTP_req, "GET / ") || StrContains(HTTP_req, "GET /index.htm")) {
    
       sendHtmlAnswer(cl);
       internalHTMLsend(cl);
    }
    else if (openIndexFile()) {
      sendHtmlAnswer(cl);
      sendBodyAnswerHTML(cl);
      
    } else {
      webFile = SD.open(F("404.htm"));
      sendBodyAnswer(cl);
    }
  }
  else if (StrContains(HTTP_req, GET)) {
    // files requests
    if      (StrContains(HTTP_req, ".htm")) {
      if (openWebFile()) {
        sendHtmlAnswer(cl);
        sendBodyAnswerHTML(cl);
      } else {
        webFile = SD.open(F("404.htm"));
        sendBodyAnswer(cl);
      }
    }
  }
} // parseRequest ( )

//________________________________________________________________________________________________________________________________________________________________

String ipString(byte ip[]) {
  String s = "";
  for (byte i = 0; i < 4; i++) {
    s += ip[i];
    if (i < 3)
      s += '.';
  }
  Serial.println(s);
  return s;
}

//Всё что касается передачи данных по Ethernet
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; //Указываем MAC адресс
byte my_IP[] = {192,168,1,127};
//Задание переменных для передачи в Гонец
byte send_IP[] = {192,168,1,55};
char fromTerm[5]= "1025";
char toTerm[5]= "7";
IPAddress ip(my_IP);//Свой IP
IPAddress GonetsIP(send_IP);//IP устройства назначения

void EEPROMreadconf() {
 byte chread = EEPROM.read(1);
 char ch;
 string readbuff = "";
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
 for (i = 0; i<5; i++){
   if ((ch=EEPROM.read(FROM_SHIFT+i)!='\0') {
   readbuff += ch;
 }
 else break;
 fromTerm = readbuff.c_str();
 readbuff = "";
 for (i = 0; i<5; i++){
   if ((ch=EEPROM.read(TO_SHIFT+i)!='\0') {
    readbuff += ch;
   }
   else break;
 toTerm = readbuff.c_str();         
}

//Инициализируем настройки портов вводы вывода
void serverInit() {
  ip(my_IP);
  GonetsIP(send_IP);
  Ethernet.begin(mac, ip);
  server.begin();
  delay(200);
}
void serialInit() {
  Serial.begin(9600);
  Serial1.begin(BRate_serial1);
  Serial1.setTimeout(200);
}

/*
  _____________________________________________________________________________________________________________________________________________
  Server proccesing
*/
void serverWorks2(EthernetClient sclient) {
  String strRequest = "";
  if (sclient) {
    boolean currentLineIsBlank = true;
    while (sclient.connected()) {      
      if (sclient.available()) {   // client data available to read
        char c = sclient.read();   // read 1 byte (character) from client

        /* limit the size of the stored received HTTP request
           buffer first part of HTTP request in HTTP_req array (string)
           leave last element in array as 0 to null terminate string (REQ_BUF_SZ - 1) */

        // make request (array)
        if (reqIndex < (REQ_BUF_SIZE - 1)) {
          HTTP_req[reqIndex] = c; // save HTTP request character
          reqIndex++;
        }
        
        // make request (String)
        /*
        if (request.length() < MAX_LEN_REQUEST) {
          if (c == '\n') {row++;}
          if (row != 2 && row != 3 && row != 4 && row != 5 && row != 6) {
            request += c;
          }
        }
        */

        if (strRequest.length() < MAX_LEN_STRING) {
          strRequest += c;
        }
        if (c == '\n' || strRequest.length() >= MAX_LEN_STRING) {
          if (strRequest.indexOf(F("GET")) == 0) {
            request += strRequest;
            strRequest = "";
          }
          else if (strRequest.indexOf(F("Authorization")) == 0) {
            request += strRequest;
            strRequest = "";
          }
          else {
            strRequest = "";
          }
        }

        /* last line of client request is blank and ends with \n
           respond to client only after last line received */

        if (c == '\n' && currentLineIsBlank) {
          //if (authMode == AUTH_OFF || (authMode == AUTH_ON && request.lastIndexOf(AUTH_HASH) > -1)) {     Authorization
            parseRequest(sclient); 
           //responseSend(sclient);
           // Reset buffer index and all buffer elements to 0
            reqIndex = 0;
            StrClear(HTTP_req, REQ_BUF_SIZE);
            request = "";
     
         /* Authorization 
          } 
          else { // if (authMode == AUTH_OFF || (authMode == AUTH_ON && request.lastIndexOf(AUTH_HASH) > -1))
              request = "";
              sclient.println(F("HTTP/1.0 401 Unauthorized"));
              sclient.println(F("WWW-Authenticate: Basic realm=\"Arduino Mega Server\""));
            }
          */ 
          break;
        }
        
        // every line of text received from the client ends with \r\n
        if (c == '\n') {
          
          /* last character on line of received text starting new line with next character read */
          currentLineIsBlank = true;
        } else if (c != '\r') {
            // a text character was received from client
            currentLineIsBlank = false;
          }
      } // if (client.available())
    } // while (client.connected())  
    delay(5); // give the web browser time to receive the data
    sclient.stop(); // close the connection
  } // if (client)  
} // serverWorks2( )

void serverWorks() { //Обработка сообщений Ethernet
  for (int sock = 0; sock < MAX_SOCK_NUM - 1; sock++) {
    EthernetClient sclient = server.available_(sock);
    serverWorks2(sclient);
  }
  /*
  EthernetClient sclient = server.available();
  serverWorks2(sclient);
  */
}
boolean recieveAnswer(EthernetClient cl){ //Проверка ответа от терминала
  String strRequest = "";
  byte tryN = 0;
  if (cl) {
    while (cl.connected() && tryN<MAX_NUM_ATTEMPTS) {      
      while (cl.available()) {   // client data available to read
        char c = cl.read();   // read 1 byte (character) from client
        if (reqIndex < (REQ_BUF_SIZE - 1)) {
          HTTP_req[reqIndex] = c; // save HTTP request character
          reqIndex++;
        }
        if (strRequest.length() < MAX_LEN_STRING) {
          strRequest += c;
        }
        if (c == '\n' || strRequest.length() >= MAX_LEN_STRING) {
          //Parsing answers
          if (strRequest.indexOf(F("OK")) > 0) {
            //Clear global buffers
            reqIndex = 0;
            StrClear(HTTP_req, REQ_BUF_SIZE);
            request = "";
            return true;
          }
        }
       } // while (client.available())
     Serial.println("No answer");
     delay(200);
     tryN++;  
    } // while (client.connected())
    cl.stop();
    return false;  
  } // if (client)  
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
    String serialReq;
    char *sendBuff;
    if (Serial1.available()>0){
      serialReq = Serial1.readString();
    }
    if (serialReq != ""){
    sendBuff = serialReq.c_str();
    GonetsHTTPsend(sendBuff,fromTerm,toTerm, 1);
    serialReq = "";
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
