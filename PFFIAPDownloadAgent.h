//  IEEE1888 / FIAP Downloader Library for Arduino-1.0
//
//  2012/03/07 ver.1   H.Ochiai
//  2012/11/11 ver.1.1 H.Ochiai

// --------- FIAPDownloadAgent.h (begin) ---------
#ifndef __FIAPDownloadAgent__
#define __FIAPDownloadAgent__

#include <Client.h>

// return code of get method
#define FIAP_DOWNLOAD_OK       0      // Succeeded
#define FIAP_DOWNLOAD_CONNFAIL 1      // Connection faild (Socket I/O error)
#define FIAP_DOWNLOAD_DNSERR   2      // DNS error
#define FIAP_DOWNLOAD_HTTPERR  3      // HTTP Server error (The response was not "200 OK")
#define FIAP_DOWNLOAD_FIAPERR  4      // FIAP Server error
#define FIAP_DOWNLOAD_INTERNAL_ERR 5  // INTERNAL ERROR (Client side) e.g., memory overflow

// class definition
class FIAPDownloadAgent {
public: 
  void begin(const char* server_host,
             const char* server_path,
             uint16_t server_port);
             
  int get(const char pid[], char value[], int n, 
         int* year, byte* month, byte* day,
         byte* hour, byte* minute, byte* second,
         char timezone[], int n_tz);

private:
  char* getAttrValue(EthernetClient* client);
  char* getContent(EthernetClient* client);
  int parseValue(EthernetClient* client, char value[], int n, int* year, byte* month, byte* day, byte* hour, byte* minute, byte* second, char timezone[], int n_tz);
  int parse(EthernetClient* client, char value[], int n, int* year, byte* month, byte* day, byte* hour, byte* minute, byte* second, char timezone[], int n_tz);
  
private: 
  const uint8_t* server_ip4;
  const char* server_host;
  const char* server_path;
  uint16_t server_port;
  // const char* point_id;
};

#endif  // #ifndef FIAPDownloadAgent
// --------- FIAPDownloadAgent.h (end) ---------
