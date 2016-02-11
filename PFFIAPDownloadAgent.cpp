//  IEEE1888 / FIAP Downloader Library for Arduino-1.0
//
//  2012/03/07 ver.1   H.Ochiai
//  2012/11/11 ver.1.1 H.Ochiai
//  2016/02/10 ver.1.2   Hiromasa Ihara(forked)

// --------- PFFIAPDownloadAgent.cpp (begin) ---------
#include <SPI.h>
#include <Ethernet.h>
#include <Client.h>
#include <avr/pgmspace.h>     // Retrieve Strings from the Program Memory
#include "PFFIAPDownloadAgent.h"

static char NULL_CHARACTER = '\0';

// void void FIAPDownloadAgent::begin( ... );
// Initialize the FIAPDownloadAgent instance
void FIAPDownloadAgent::begin(
    const char* server_host,
    const char* server_path,
    uint16_t server_port){

  this->server_host=server_host;
  this->server_path=server_path;
  this->server_port=server_port;
}

// int FIAPDownloadAgent::get(...);
//   const char pid[]
//   char value[]
//   int n, 
//   int* year
//   byte* month
//   byte* day,
//   byte* hour
//   byte* minute
//   byte* second,
//   char timezone[]
//   int n_tz
//
//   Return values
//    FIAP_DOWNLOAD_OK       0      // Succeeded
//    FIAP_DOWNLOAD_CONNFAIL 1  // Connection faild (Socket I/O error)
//    FIAP_DOWNLOAD_DNSERR   2  // DNS error
//    FIAP_DOWNLOAD_HTTPERR  3  // HTTP Server error (The response was not "200 OK")
//    FIAP_DOWNLOAD_FIAPERR  4  // FIAP Server error
//    FIAP_DOWNLOAD_INTERNAL_ERR 5  // INTERNAL ERROR (Client side) e.g., memory overflow

// Messages (Stored in the program memory) -- HTTP Header Part
PROGMEM const char FIAPDownloadAgent_Get_HTTPHEADER01[] =  "POST ";
PROGMEM const char FIAPDownloadAgent_Get_HTTPHEADER02[] =  " HTTP/1.1";
PROGMEM const char FIAPDownloadAgent_Get_HTTPHEADER03[] =  "Content-Type: text/xml charset=UTF-8";
PROGMEM const char FIAPDownloadAgent_Get_HTTPHEADER04[] =  "User-Agent: PFFIAPDownloadAgent (Arduino)";
PROGMEM const char FIAPDownloadAgent_Get_HTTPHEADER05[] =  "Host: ";
PROGMEM const char FIAPDownloadAgent_Get_HTTPHEADER06[] =  "SOAPAction: \"http://soap.fiap.org/query\"";
PROGMEM const char FIAPDownloadAgent_Get_HTTPHEADER07[] =  "Content-Length: ";

// Messages (Stored in the program memory) -- HTTP Body Part
PROGMEM const char FIAPDownloadAgent_Get_HTTPBODY01[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
PROGMEM const char FIAPDownloadAgent_Get_HTTPBODY02[] = "<soapenv:Envelope xmlns:soapenv=";
PROGMEM const char FIAPDownloadAgent_Get_HTTPBODY03[] = "\"http://schemas.xmlsoap.org/soap/envelope/\">";
PROGMEM const char FIAPDownloadAgent_Get_HTTPBODY04[] = "<soapenv:Body>";
PROGMEM const char FIAPDownloadAgent_Get_HTTPBODY05[] = "<ns2:queryRQ xmlns:ns2=\"http://soap.fiap.org/\">";
PROGMEM const char FIAPDownloadAgent_Get_HTTPBODY06[] = "<transport xmlns=\"http://gutp.jp/fiap/2009/11/\">";
PROGMEM const char FIAPDownloadAgent_Get_HTTPBODY07[] = "<header>";
PROGMEM const char FIAPDownloadAgent_Get_HTTPBODY08[] = "<query id=\"9eed9de4-1c48-4b08-a41d-dac067fc1c0d\" type=\"storage\">";
PROGMEM const char FIAPDownloadAgent_Get_HTTPBODY09[] = "<key id=\"";
PROGMEM const char FIAPDownloadAgent_Get_HTTPBODY10[] = "\" attrName=\"time\" select=\"maximum\" />";
PROGMEM const char FIAPDownloadAgent_Get_HTTPBODY11[] = "</query></header></transport></ns2:queryRQ>";
PROGMEM const char FIAPDownloadAgent_Get_HTTPBODY12[] = "</soapenv:Body></soapenv:Envelope>";

int FIAPDownloadAgent::get(const char pid[], char value[], int n, 
    int* year, byte* month, byte* day,
    byte* hour, byte* minute, byte* second,
    char timezone[], int n_tz){

  int ret_code = FIAP_DOWNLOAD_OK;
  int rescode = 0;  // HTTP response code
  int clen = 0;     // content length
  char count;
  unsigned char c;

  // point_id=pid;
  EthernetClient client;

  if (!client.connect(server_host, server_port)) {
    return(FIAP_DOWNLOAD_CONNFAIL);
  }

  clen = 430; // sum of literal strings
  clen += strlen(pid);
  // Serial.print("len="); Serial.println(clen);

  // sending message buffer
  char sbuf[80];

  // send HTTP header
  strcpy_P(sbuf,FIAPDownloadAgent_Get_HTTPHEADER01);
  client.print(sbuf);  // "POST "
  client.print(server_path);
  strcpy_P(sbuf,FIAPDownloadAgent_Get_HTTPHEADER02);
  client.println(sbuf);  // " HTTP/1.1"
  strcpy_P(sbuf,FIAPDownloadAgent_Get_HTTPHEADER03);
  client.println(sbuf);  // "Content-Type: text/xml; charset=UTF-8"
  strcpy_P(sbuf,FIAPDownloadAgent_Get_HTTPHEADER04);
  client.println(sbuf);  // "User-Agent: FIAPDownloadAgent (Arduino)"
  strcpy_P(sbuf,FIAPDownloadAgent_Get_HTTPHEADER05);
  client.print(sbuf);  // "Host: "
  client.println(server_host);
  // client.println("Content-Type: application/soap+xml; charset=UTF-8");
  strcpy_P(sbuf,FIAPDownloadAgent_Get_HTTPHEADER06);
  client.println(sbuf);  // "SOAPAction: \"http://soap.fiap.org/query\""
  strcpy_P(sbuf,FIAPDownloadAgent_Get_HTTPHEADER07);
  client.print(sbuf);  // "Content-Length: "
  client.println(clen);
  client.println();

  // send HTTP body
  strcpy_P(sbuf,FIAPDownloadAgent_Get_HTTPBODY01);
  client.println(sbuf);  // "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
  strcpy_P(sbuf,FIAPDownloadAgent_Get_HTTPBODY02);
  client.print(sbuf);  // "<soapenv:Envelope xmlns:soapenv="
  strcpy_P(sbuf,FIAPDownloadAgent_Get_HTTPBODY03);
  client.println(sbuf);  // "\"http://schemas.xmlsoap.org/soap/envelope/\">"
  strcpy_P(sbuf,FIAPDownloadAgent_Get_HTTPBODY04);
  client.print(sbuf);  // "<soapenv:Body>"
  strcpy_P(sbuf,FIAPDownloadAgent_Get_HTTPBODY05);
  client.print(sbuf);  // "<ns2:queryRQ xmlns:ns2=\"http://soap.fiap.org/\">"
  strcpy_P(sbuf,FIAPDownloadAgent_Get_HTTPBODY06);
  client.print(sbuf);  // "<transport xmlns=\"http://gutp.jp/fiap/2009/11/\">"
  strcpy_P(sbuf,FIAPDownloadAgent_Get_HTTPBODY07);
  client.print(sbuf);  // "<header>"
  strcpy_P(sbuf,FIAPDownloadAgent_Get_HTTPBODY08);
  client.print(sbuf);  // "<query id=\"9eed9de4-1c48-4b08-a41d-dac067fc1c0d\" type=\"storage\">"
  strcpy_P(sbuf,FIAPDownloadAgent_Get_HTTPBODY09);
  client.print(sbuf);  // "<key id=\""
  client.print(pid);
  strcpy_P(sbuf,FIAPDownloadAgent_Get_HTTPBODY10);
  client.println(sbuf);  // "\" attrName=\"time\" select=\"maximum\" />"
  strcpy_P(sbuf,FIAPDownloadAgent_Get_HTTPBODY11);
  client.print(sbuf);  // "</query></header></transport></ns2:queryRQ>"
  strcpy_P(sbuf,FIAPDownloadAgent_Get_HTTPBODY12);
  client.println(sbuf);  // "</soapenv:Body></soapenv:Envelope>"
  //  client.println("                                         ");
  client.println();

  // parse HTTP response
  count = 0;
  while (client.connected()) {
    if (client.available()) {
      c = client.read();  // Serial.print(c);
      if (count == 1 && (c >= '0' && c <= '9')) {  // parse HTTP response code
        rescode = rescode * 10 + (c - '0');
        continue;
      }
      if (c == ' ') {  
        count++;
      }
      if (count == 2 || c == '\n') {  // end of HTTP response code
        break;    
      }
    }
  }
  if (!client.connected()) {  // unexpected disconnect
    client.stop();
    return FIAP_DOWNLOAD_HTTPERR;
  }

  // disconnect HTTP
  if(client.connected()){
    ret_code=parse(&client,value,n,year,month,day,hour,minute,second,timezone,n_tz);
  }
  while (client.connected() && client.available()) {    
    c = client.read(); // Serial.print(c);  
  }
  client.stop();
  if (rescode == 200) {
    return ret_code;
  }
  return FIAP_DOWNLOAD_HTTPERR;
}

char* FIAPDownloadAgent::getAttrValue(EthernetClient* client){

  static char attr_value_buffer[50];

  attr_value_buffer[0]=NULL_CHARACTER;
  while(client->connected() && client->available()){
    char c=client->read();
    if(c==' ' || c=='='){
      continue;
    }else if(c=='"'){ 
      int i;
      char buf[50];
      for(i=0;i<50 && client->connected() && client->available();i++){
        c=client->read();
        buf[i]=c;
        if(c=='"'){
          break; 
        }
      }
      buf[i]=NULL_CHARACTER;
      if(i==50){
        return NULL; 
      }
      strncpy(attr_value_buffer,buf,50);
      return attr_value_buffer;
    }
  }
  return NULL;
}

char* FIAPDownloadAgent::getContent(EthernetClient* client){

  static char content_buffer[100];
  content_buffer[0]=NULL_CHARACTER;

  int i;
  char buf[100];
  for(i=0;i<100 && client->connected() && client->available();i++){
    char c=client->read();
    buf[i]=c;
    if(c=='<'){
      break; 
    }
  }
  if(i==100){
    return NULL;
  }
  buf[i]=NULL_CHARACTER;

  strncpy(content_buffer,buf,100);
  return content_buffer;
}

// Constant Texts (Stored in the program memory)
PROGMEM const char FIAPDownloadAgent_ParseValue01[] = "ime ";
PROGMEM const char FIAPDownloadAgent_ParseValue02[] = "ime=";
int FIAPDownloadAgent::parseValue(EthernetClient* client, char value[], int n, int* year, byte* month, byte* day, byte* hour, byte* minute, byte* second, char timezone[], int n_tz){

  while(client->connected() && client->available()){
    char c=client->read(); // Serial.print(c);
    if(c==' '){
      continue;
    }else if(c=='/'){
      return FIAP_DOWNLOAD_OK;
    }else if(c=='>'){
      char* str_value=getContent(client);
      if(str_value==NULL){
        return FIAP_DOWNLOAD_INTERNAL_ERR; 
      }
      strncpy(value,str_value,n-1);
      value[n-1]=NULL_CHARACTER;
      int i=strlen(value);
      if(i==n-1){
        return FIAP_DOWNLOAD_INTERNAL_ERR; 
      }
      return FIAP_DOWNLOAD_OK;
    }else if(c=='t'){
      int i;
      char buf[5];
      for(i=0;i<4 && client->connected() && client->available();i++){
        buf[i]=client->read(); 
      }
      buf[i]=NULL_CHARACTER;
      char abuf[5]; strcpy_P(abuf,FIAPDownloadAgent_ParseValue01); // "ime "
      char bbuf[5]; strcpy_P(bbuf,FIAPDownloadAgent_ParseValue02); // "ime="
      if(strcmp(buf,abuf)==0 || strcmp(buf,bbuf)==0){
        char* time=getAttrValue(client);
        if(time==NULL){
          return FIAP_DOWNLOAD_INTERNAL_ERR; 
        }
        if(strlen(time)<19){
          return FIAP_DOWNLOAD_INTERNAL_ERR;
        }
        strncpy(buf,&time[0],4); buf[4]=NULL_CHARACTER; *year=atoi(buf);
        strncpy(buf,&time[5],2); buf[2]=NULL_CHARACTER; *month=(byte)atoi(buf);
        strncpy(buf,&time[8],2);  buf[2]=NULL_CHARACTER; *day=(byte)atoi(buf);
        strncpy(buf,&time[11],2); buf[2]=NULL_CHARACTER; *hour=(byte)atoi(buf);
        strncpy(buf,&time[14],2); buf[2]=NULL_CHARACTER; *minute=(byte)atoi(buf);
        strncpy(buf,&time[17],2); buf[2]=NULL_CHARACTER; *second=(byte)atoi(buf);
      }
    }
  }
}

// Constant Texts (Stored in the program memory)
PROGMEM const char FIAPDownloadAgent_Parse01[] = "value ";
PROGMEM const char FIAPDownloadAgent_Parse02[] = "error ";
int FIAPDownloadAgent::parse(EthernetClient* client, char value[], int n, int* year, byte* month, byte* day, byte* hour, byte* minute, byte* second, char timezone[], int n_tz){

  value[0]=NULL_CHARACTER;
  timezone[0]=NULL_CHARACTER;
  *year=0; *month=0; *day=0; *hour=0; *minute=0; *second=0;
  while(client->connected() && client->available()){
    char c=client->read();
    if(c=='<'){
      int i;
      char buf[7];
      for(i=0;i<6 && client->connected() && client->available();i++){
        buf[i]=client->read(); 
        if(buf[i]==':'){ i=-1; } // restarts from i=0 (i=-1; i++ => i=0)
      }
      buf[i]=NULL_CHARACTER;

      char sbuf[7];
      strcpy_P(sbuf,FIAPDownloadAgent_Parse01); // "value "
      if(strcmp(buf,sbuf)==0){
        return parseValue(client, value, n, year, month, day, hour, minute, second, timezone, n_tz);
      }
      strcpy_P(sbuf,FIAPDownloadAgent_Parse02); // "error "
      if(strcmp(buf,sbuf)==0){
        return FIAP_DOWNLOAD_FIAPERR;
      }
    }
  }
  return FIAP_DOWNLOAD_OK;
}

// --------- PFFIAPDownloadAgent.cpp (end) ---------
