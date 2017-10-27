//Function for sending HTM page if SD isn't available
void internalHTMLsend(EthernetClient cl){
  cl.println(F("<!DOCTYPE html>"));
  cl.println(F("<html>"));
  cl.println(F("<head>")); 
  cl.println(F("<meta charset=\"utf-8\">"));
  cl.println(F("<title>DoubleA Gonets sender</title>"));
  cl.println(F("</head>"))
  cl.println(F("<body>"));
  cl.println(F("<h2>DoubleA configuration</h2>"));
  cl.println(F("<table border='0'>"));
  cl.println(F("<tr><td>Свой IP адрес</td><td><input id=\"textField1\" type=\"text\" value=\""));
  cl.print(my_IP[0]); cl.print('.'); cl.print(my_IP[1]); cl.print('.'); cl.print(my_IP[2]); cl.print('.'); cl.print(my_IP[3]); cl.println(F("\"></input></td></tr>"));
  cl.println(F("<tr><td>IP адрес терминала ГОНЕЦ</td><td><input id=\"textField2\" type=\"text\" value=\"")
  cl.print(send_IP[0]); cl.print('.'); cl.print(send_IP[1]); cl.print('.'); cl.print(send_IP[2]); cl.print('.'); cl.print(send_IP[3]); cl.println(F("\"></input></td></tr>"));           
  cl.print(F("<tr><td>Номер отправляющего терминала ГОНЕЦ</td><td><input id=\"textField3\" type=\"text\" value=\"")); cl.print(fromTerm); cl.println(F("\"></input></td></tr>"));
  cl.println(F("<tr><td>Номер принимающего термиенала ГОНЕЦ</td><td><input id=\"textField4\" type=\"text\" value=\"")); cl.print(toTerm); cl.println(F("\"></input></td></tr>"));
  cl.println(F("<tr><td>Скорость передачи данных (Бод)</td><td><select id=\"sel\"><option value=\"0\">9600</option><option value=\"1\">19200</option><option value=\"2\">38400</option><option value=\"3\" selected>57600</option><option value=\"4\">115200</option></td></tr>"));
  cl.println(F("</table>"));
  cl.println(F("<button type=\"button\" onclick=\"loadXMLDoc()\">Change settings</button>"));
  cl.println(F("<div id=\"console\"></div>"));
  cl.println(F("<script>"));
  cl.println(F("function loadXMLDoc() {"));
  cl.println(F("var v1 = textField2.value;"));     
  cl.println(F("var v2 = textField2.value;"));     
  cl.println(F("var v3 = textField3.value;"));     
  cl.println(F("var v4 = textField4.value;"));     
  cl.println(F("var v5 = sel.value;"));     
  cl.println(F("var xhttp = new XMLHttpRequest();"));     
  cl.println(F("if (this.readyState == 4 && this.status == 200) {"));     
  cl.println(F("document.getElementById(\"console\").innerHTML = this.responseText;"));     
  cl.println('}');     
  cl.println(F("};"));     
  cl.println(F("xhttp.open(\"GET\", \"/?ip=\" + v1 + \"&ip_dest=\" + v2 + \"&from=\" + v3 + \"&to=\" + v4 + \"&mode=\" + v5, true);"));     
  cl.println(F("xhttp.send();"));     
  cl.println('}');     
  cl.println(F("</script>"));
  cl.println(F("</body>"));     
  cl.println(F("</html>"));     
}
