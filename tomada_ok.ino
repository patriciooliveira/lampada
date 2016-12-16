#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>


/*   variaveis globais usadas
*/
#define ssid      "Robotica-IMD"
#define password  "roboticawifi"
#define saida D5

int situacao;

WiFiServer server(80);
WiFiClient client;


/* variaveis de debug
*/

bool controlAllow = true;
bool header = false;
bool httpCompleto = false;
bool htmlCompleto = false;
bool contentTypeHtml = true;


void conecta(char* rede, char* senha);
void esperaCliente();
void trataRequisicao();
void statos();
void liga() ;
void desliga() ;
void resposta();

void setup() {
  pinMode(saida, OUTPUT);
  Serial.begin(115200);
  conecta(ssid, password);
  // inicia o server TCP (HTTP)
  server.begin();
}

void loop() {
  client = server.available();
  if (!client) {
    return;
  }
  esperaCliente();
  trataRequisicao();
}



/*  funcao conecta e exibe o ip pela porta serialquando conectado
    parametros:
    char* nome_da_rede
    char* senha_da_rede
*/
void conecta(char* rede, char* senha) {
  WiFi.begin(rede, senha);//
  Serial.println("conectando");
  while (WiFi.status() != WL_CONNECTED) {  // imprime ponto enquanto não conecta
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}


/*  aguarda a requisicao completa do cliente
*/
void esperaCliente() {
  Serial.println("\n\t New client");
  while (client.connected() && !client.available()) {
    delay(1);
  }
}

/* trata as requisicoes montando a resposta html
*/

void trataRequisicao() {
  String req = client.readStringUntil('\r');
  //  First line of HTTP request looks like "GET /path HTTP/1.1"
  // Retrieve the "/path" part by finding the spaces
  Serial.println(req);
  int addr_start = req.indexOf(' ');
  int addr_end = req.indexOf(' ', addr_start + 1);
  if (addr_start == -1 || addr_end == -1) {
    Serial.print("Invalid request: ");
    Serial.println(req);
    return;
  }


  req = req.substring(addr_start + 1, addr_end);
  /*
     deletar
  */
  Serial.println(req);

  if (req == "/status") {
    statos();
  }

  else if (req == "/config") {
    //aqui vai chamar a função de configuração
  }

  else if (req == "/0") { ///testar atribuir uma variavel aqui depois
    desliga();
  }
  else if (req == "/1") { ///testar atribuir uma variavel aqui depois
    liga();
  }

  else {
    String s = "HTTP/1.1 404 Not Found/r/n/r/n";
    s += "Nao encontrado";
    client.print(s);
    Serial.println("Sending 404");
    Serial.print("Request: ");
    Serial.println(req);
    client.flush();
  }
}


/*  status de toda a configuração do sistema  */
void statos() {
  //IPAddress ip = WiFi.localIP();
  //String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
  resposta();
}

void liga() {
  situacao = 1;
  Serial.println("liga");
  digitalWrite(saida, HIGH);
  resposta();
}

void desliga() {
  situacao = 0;
  Serial.println("desliga");
  digitalWrite(saida, situacao);
  resposta();
}


void resposta() {
  String s;
  //versão_do_protocolo código_de_estado descrição_do_estato
  s =  "HTTP/1.1 200 OK\r\n";
  if (controlAllow) {
    s += "Access-Control-Allow-Origin: *\r\n";
  }
  if (httpCompleto) {
    //Data
    s += "Date: Dia, xx Mmm AAAA hh:mm:ss GMT\r\n";
    //Servidor
    s += "Server ESP\r\n";
    //tipo_de_conteudo
  }
  if (contentTypeHtml) {
    s += "Content-Type: text/html\r\n\n";
  }
  else {
    s += "Content-Type: application/json\r\n\n";
  }
  if (htmlCompleto) {
    //Documento html
    s += "<!DOCTYPE HTML>\r\n";
    s += "<html>";
    s += "<head><title>LAMPADA</title></head><body>";
  }
  s += "{\"e\":";
  s += situacao;
  s += "}";
  if (htmlCompleto) {
    s += "</body>";
    s += "</html>\r\n\r\n";
  }
  //envia para o client
  client.print(s);
  client.flush();
  //imprime resultado na Serial
  Serial.println("Sending 200");
  Serial.print("situação: ");
  Serial.println(situacao);
  Serial.print("Done with client\n");
  Serial.println(s);
  return;
}
