//Function for sending HTM page if SD isn't available
void internalHTMLsend(EthernetClient cl){
  cl.println(F("<!DOCTYPE html>"));
  cl.println(F("<html>"));
  cl.println(F("<head>")); 
  cl.println(F("<meta charset="utf-8">"));
  cl.println(F("<title>DoubleA Gonets sender</title>"));
  cl.println(F("<body>"));
  cl.println(F("SD Open error"));
  cl.println(F("</body>"));
  cl.println(F("</html>"));
//  cl.println(F("<head>"));
//  cl.println(F("<head>"));
//  cl.println(F("<head>"));
//  cl.println(F("<head>"));
//  cl.println(F("<head>"));
//  cl.println(F("<head>"));
//  cl.println(F("<head>"));
//  cl.println(F("<head>"));
//  cl.println(F("<head>"));
//  cl.println(F("<head>"));     
}
