#include <SPI.h>   //подключаем библиотеку для работы с SPI
#include <Ethernet.h>  //Библиотека для работы в сети
#include <string.h> //Библиотека для работ со строками
#include <stdio.h> //Библиотека для преобразования
#include <SD.h> //Библиотека для работы с SD

//Regexp Features
#include <Regexp.h>
#define MAXCAPTURES 4

//Server Features
#define SERVER_PORT 80
EthernetServer server(SERVER_PORT);

//my defines
#define IN_BODY 0
#define IN_TEMPLATE 1
#define MAX_NUM_ATTEMPTS 3
#define MAX_TEMPLATE_LENGTH 10

// HTTP request
#define REQ_BUF_SIZE 128
char HTTP_req[REQ_BUF_SIZE] = {0}; // null terminated string
int reqIndex = 0;

// String request
#define MAX_LEN_STRING  128
#define MAX_LEN_REQUEST 256 // 512
String request = "";

// webFile
File webFile;
#define MAX_BUFFER_SIZE 256
#define GET           "GET /"
#define INDEX_STR     "index"
#define HTM_EXT       ".HTM"
uint16_t rsize;
char buff[MAX_BUFFER_SIZE];

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
void sendCssAnswer (EthernetClient cl) {
  cl.println(makeAnswer(F("text/css")));
}
void sendJsAnswer  (EthernetClient cl) {
  cl.println(makeAnswer(F("application/javascript")));
}
void sendPngAnswer (EthernetClient cl) {
  cl.println(makeAnswer(F("image/png")));
}
void sendJpgAnswer (EthernetClient cl) {
  cl.println(makeAnswer(F("image/jpeg")));
}
void sendGifAnswer (EthernetClient cl) {
  cl.println(makeAnswer(F("image/gif")));
}
void sendXmlAnswer (EthernetClient cl) {
  cl.println(makeAnswer(F("text/xml")));
}
void sendIcoAnswer (EthernetClient cl) {
  cl.println(makeAnswer(F("image/x-icon")));
}

//void sendDownAnswer(EthernetClient cl) {cl.println(makeAnswer(F("application/octet-stream")));}
//void sendPdfAnswer (EthernetClient cl) {cl.println(makeAnswer(F("application/x-pdf")));}
//void sendZipAnswer (EthernetClient cl) {cl.println(makeAnswer(F("application/x-zip")));}
//void sendGzAnswer  (EthernetClient cl) {cl.println(makeAnswer(F("application/x-gzip")));}
//void sendElseAnswer(EthernetClient cl) {cl.println(makeAnswer(F("text/plain")));}

void sendErrorAnswer(char mess[], EthernetClient cl) {
  cl.print(mess);
  cl.println(F(" ERROR"));
  cl.println(F("Connnection: close"));
  cl.println();
}

String makeTag(String tagBase, String tagCount, String value) {
  String s = "";
  s += "<"; s += tagBase; s += tagCount; s += ">";
  s += value;
  s += "</"; s += tagBase; s += tagCount; s += ">\n";
  return s;
}

boolean openWebFile() {
  char *fileName;
  fileName = strtok(HTTP_req, GET);
  webFile = SD.open(fileName);
  if (webFile) {
    return true;
  }
  else {
    return false;
  }
}

boolean openIndexFile() {
  webFile = SD.open("index.htm");
  if (webFile) {
    return true;
  }
  else {
    return false;
  }
}
 
 void parseRequest(EthernetClient cl) {
  // index request
  if (StrContains(HTTP_req, "GET / ") || StrContains(HTTP_req, "GET /index.htm")) {
    if (openIndexFile()) {
      sendHtmlAnswer(cl);
    } else {
      webFile = SD.open(F("404.htm"));
    }
  }
  else if (StrContains(HTTP_req, GET)) {
    // files requests
    if      (StrContains(HTTP_req, ".htm")) {
      if (openWebFile()) {
        sendHtmlAnswer(cl);
      } else {
        webFile = SD.open(F("404.htm"));
      }
    }
    else if (StrContains(HTTP_req, ".css"))  {
      if (openWebFile()) {
        sendCssAnswer(cl);
      }  else {
        sendErrorAnswer("", cl);
      }
    }
    else if (StrContains(HTTP_req, ".js"))   {
      if (openWebFile()) {
        sendJsAnswer(cl);
      }   else {
        sendErrorAnswer("", cl);
      }
    }
    else if (StrContains(HTTP_req, ".pde"))  {
      if (openWebFile()) {
        sendJsAnswer(cl);
      }   else {
        sendErrorAnswer("", cl);
      }
    }
    else if (StrContains(HTTP_req, ".png"))  {
      if (openWebFile()) {
        sendPngAnswer(cl);
      }  else {
        sendErrorAnswer("", cl);
      }
    }
    else if (StrContains(HTTP_req, ".jpg"))  {
      if (openWebFile()) {
        sendJpgAnswer(cl);
      }  else {
        sendErrorAnswer("", cl);
      }
    }
    else if (StrContains(HTTP_req, ".gif"))  {
      if (openWebFile()) {
        sendGifAnswer(cl);
      }  else {
        sendErrorAnswer("", cl);
      }
    }
    else if (StrContains(HTTP_req, ".ico"))  {
      if (openWebFile()) {
        sendIcoAnswer(cl);
      }  else {
        sendErrorAnswer("", cl);
      }
    }
    // Ajax requests
    else if (StrContains(HTTP_req, "request_settings")) {
      sendXmlAnswer(cl);
    }// else if (StrContains(HTTP_req, GET))
   }
   
} // parseRequest ( )

void responseSend(Ethernetclient sclient) {
  char ch;
  char buffTemplate[MAX_TEMPLATE_LENGTH];
  rsize = 0;
  uint8_t read_state = IN_BODY; 
  while (webFile) {
    if (!webFile.available()) breake;
    if (ch = webFile.read())>0) { //if file ended 
    sclient.write(buff,rsize); //Send last part
    rsize = 0; //reset pointer
    brake;
    }
    if (rsize >= MAX_BUFFER_SIZE) { //if buffer is full
     sclient.write(buff, rsize);
     rsize = 0; //Send buffer and reset pointer
    }    
    switch (read_state) {
       case IN_BODY {
         if (ch == '%'){
         sclient.write(buff,rsize); //Send buffer before symbol
         rsize = 0; //Clear buffer length
         read_state = IN_TEMPLATE;
         brake;
         }
         buff[rsize++] = ch; //Add new element to main buffer
         brake;
       }
       case IN_TEMPLATE {
         if (ch == '%') {
           sclient.write(sendVar(buffTemplate)); //Send string returned after replace by Variable
           read_state = IN_BODY;
           brake;
         if (rsize > MAX_TEMPLATE_LENGTH); //do not recieve close "%"
           sclient.write('%'); //Send missing %
           sclient.write(buffTemplate);
           read_state = IN_BODY;
           brake;
         buffTemplate[rsize++] = ch; //Add new element to TemplateBuffer
         brake;
         }
    }
  /* Old Version
            if (webFile) {
              while(webFile.available()) {
                rsize = webFile.read(buff, MAX_BUFFER_SIZE);//Считывание буфера
                sclient.write(buff, rsize);
              }
              webFile.close();
            } // if (webFile)
  old version */
  webFile.close();   
}
String makeHttpReq() {
    String s = "";
    for (int i = 0; i < reqIndex; i++) {
      if (HTTP_req[i] == '&') {
        s += ' ';
      }
      else {
        s += HTTP_req[i];
      }
    }
    return makeTag("httpReq", "", s);
  }

//________________________________________________________________________________________________________________________________________________________________

String ipString(byte ip[]) {
  String s = "";
  for (byte i = 0; i < 4; i++) {
    s += ip[i];
    if (i == 3) {return s;}
    s += '.';
  }
}

//Всё что касается передачи данных по Ethernet
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; //Указываем MAC адресс
byte my_IP[] = {192,168,1,127};
//Задание переменных для передачи в Гонец
byte send_IP[] = {192,168,1,55};
char fromTerm[5]= "1030";
char toTerm[5]= "7";

File myFile;
IPAddress ip(my_IP);// ip(192,168,1,155);//Свой IP
IPAddress GonetsIP(send_IP);//IP устройства назначения

void setup() {
    mainInit();
    sdreadconf();
}
void sdreadconf(){ //Чтение конфигурации из файла сonfig.ini
String str;
char lastChar;
  myFile = SD.open("conf.ini", FILE_READ);
  while (myFile.available()) {
      lastChar = myFile.read();
      if (lastChar != '\n' && lastChar != '\0'  ){
        str += lastChar;
      }
      else
      {
       setConf(str);
       str = "";
      }
  }
  myFile.close();
}
//Parcer and set config
char *sendVar(const char *Template) {
  
}
void setConf(const String &strBuffer){
  MatchState ms;
  ms.Target (strBuffer.c_str());  // what to search
  char result = ms.Match ("(%w+)=(.+)", 0);
  if (result != REGEXP_MATCHED){
    Serial.println("Config missmatches!");  
    return;
  }
  char captBuffer[64];  
  String key = ms.GetCapture(captBuffer,0);
  char* value = ms.GetCapture(captBuffer,1);
  if (key == "ip") {
    ms.Target (value);  // what to search
    result = ms.Match("(%d+).(%d+).(%d+).(%d+)", 0);
    if (result != REGEXP_MATCHED) {
      Serial.print("IP mismatch!");
      return;
    }  
    for (byte i=0; i<4; i++) {
       my_IP[i] = atoi(ms.GetCapture(captBuffer,i));
     }
     //Назначаем новый IP адрес
    IPAddress ip(my_IP);
    Ethernet.begin(mac, ip);
    Serial.println(Ethernet.localIP());

  }// key = ip
  else if (key == "ipdest") {
    ms.Target (value);  // what to search
    result = ms.Match("(%d+).(%d+).(%d+).(%d+)", 0);
    if (result != REGEXP_MATCHED) {
      Serial.print("SendIP mismatch!");
      return;
    }  
    byte new_ip[4];
    for (byte i=0; i<4; i++) {
       new_ip[i] = atoi(ms.GetCapture(captBuffer,i));
     }
    IPAddress GonetsIP(new_ip);
  }
}
void Logthis(const char *LOG){
  File LogFile;
  LogFile = SD.open("Log.txt", FILE_WRITE);
  LogFile.write(LOG);
  LogFile.write("\r\n");
  LogFile.close();
}
//Инициализируем настройки портов вводы вывода
void mainInit() //Инициализация параметров
{
  Serial.begin(9600);
  Serial.setTimeout(200);
  Ethernet.begin(mac, ip);
  if (!SD.begin(4)) {
    Serial.println("SD initialization failed!");
    server.begin();
    delay(200);
  }
  Serial.println(F("Ready..."));
  Serial1.begin(9600);
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
            responseSend(sclient);
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
  char sendIP = ipString(send_IP).c_str();
  ln = sprintf(sendbuff,"from=%s&to=%s&urgency=0&chSv=%d&subj=data&msg=%s",fromTerminal,toTerminal,chSv, msg); //Подготовка строки к отправке
   //Задаем 3 попытки для коннекта
  while (tryN<MAX_NUM_ATTEMPTS){
    if (client.connect(GonetsIP, 4001)) {
        // HTTP-Reqest:
        Serial.println(F("Connected"));
        client.println(F("POST /sendmsg.htm HTTP/1.1"));
        client.print(F("Host: "));
        client.println(sendIP);
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
    GonetsHTTPsend(sendBuff,fromTerm,toTerm,1);
    serialReq = "";
    }
}

void loop() {
  serverWorks();
  serialWorks();
}
