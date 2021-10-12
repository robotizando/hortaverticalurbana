/*
  FSBrowser - A web-based FileSystem Browser for ESP8266 filesystems

  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the ESP8266WebServer library for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; eit
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  See readme.md for more information.
*/

////////////////////////////////
// Select the FileSystem by uncommenting one of the lines below

#define USE_SPIFFS
//#define USE_LITTLEFS
//#define USE_SDFS

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Ticker.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>


struct Config {
  char deviceUUID[64];
  int intervalPulse1;
  int schedulePulse1;
  int intervalPulse2;
  int schedulePulse2;
  int logCloudInterval;
};

const uint8_t fingerprint[20] = {0x94,0x57,0xC8,0xFE,0x33,0xB5,0x54,0xCE,0x29,0x84,0x14,0xDD,0x32,0xC1,0x48,0xF1,0x51,0x21,0xC5,0x51};


// ---------------
// NTP
// ---------------

const long utcOffsetInSeconds = -3 * 3600; //America_São Paulo

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

unsigned long currentTime;


// ---------------
// Timers
// ---------------

#define DATALOG_UPDATE_INTERVAL 60
Ticker dataLogTimer;

#define CHECK_ALARM_INTERVAL 1
Ticker checkAlarmTimer;


// ---------------
// GENERAL IO
// ---------------

#define IRRIGA 0
#define ILUMINA 1

int relesPins[] = {D0,D3};
#define relesQtd  2

const int batteryPin = A0;
int batteryValue = 0;  // value read from the pot

void output(int index, int value){
    digitalWrite(relesPins[index], !value);
}

double voltsPerUnit = 0.04;
int adoffset = 8;


int irrigaStatus = LOW;
int iluminaStatus = LOW;

// ---------------
// BME280
// ---------------

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;

float temperature, humidity, pressure, altitude;

void updateBME(){
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;
  altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
}

// ---------------
// FILE SYSTEM
// ---------------

#if defined USE_SPIFFS
  #include <FS.h>
  const char* fsName = "SPIFFS";
  FS* fileSystem = &SPIFFS;
  SPIFFSConfig fileSystemConfig = SPIFFSConfig();
#elif defined USE_LITTLEFS
  #include <LittleFS.h>
  const char* fsName = "LittleFS";
  FS* fileSystem = &LittleFS;
  LittleFSConfig fileSystemConfig = LittleFSConfig();
#elif defined USE_SDFS
  #include <SDFS.h>
  const char* fsName = "SDFS";
  FS* fileSystem = &SDFS;
  SDFSConfig fileSystemConfig = SDFSConfig();
  //fileSystemConfig.setCSPin(chipSelectPin);
#else
#error Please select a filesystem first by uncommenting one of the "#define USE_xxx" lines at the beginning of the sketch.
#endif


#define DBG_OUTPUT_PORT Serial



// ---------------
// SYSTEM CONFIGURATION FILE
// ---------------

const char *configFile = "/config.json";
Config config;

// Loads the configuration from a file
void loadConfiguration(const char *filename, Config &config) {

  Serial.println(F("Loading configuration File"));

  File file = fileSystem->open(filename, "r");

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<512> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error){
    Serial.println(F("Failed to read file, using default configuration"));
    strlcpy(config.deviceUUID,"", sizeof(config.deviceUUID));
    config.intervalPulse1 = 0;
    config.schedulePulse1 = 0;
    config.intervalPulse2 = 0;
    config.schedulePulse2 = 0;
    config.logCloudInterval = 0;
  } else {
    // Copy values from the JsonDocument to the Config
    strlcpy(config.deviceUUID, doc["deviceUUID"], sizeof(config.deviceUUID));
    config.intervalPulse1 = doc["intervalPulse1"];
    config.schedulePulse1 = doc["schedulePulse1"];
    config.intervalPulse2 = doc["intervalPulse2"];
    config.schedulePulse2 = doc["schedulePulse2"];
    config.logCloudInterval = doc["logCloudInterval"];
  }

  file.close();
  Serial.println(F("Configuration file loaded"));
}



#ifndef STASSID
#define STASSID "FamiliaTeo"
#define STAPSK  "aguadoce"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;
const char* host = "XControl";

ESP8266WebServer server(80);

static bool fsOK;
String unsupportedFiles = String();

File uploadFile;

static const char TEXT_PLAIN[] PROGMEM = "text/plain";
static const char FS_INIT_ERROR[] PROGMEM = "FS INIT ERROR";
static const char FILE_NOT_FOUND[] PROGMEM = "FileNotFound";

////////////////////////////////
// Utils to return HTTP codes, and determine content-type

void replyOK() {
  server.send(200, FPSTR(TEXT_PLAIN), "");
}

void replyOKWithMsg(String msg) {
  server.send(200, FPSTR(TEXT_PLAIN), msg);
}

void replyNotFound(String msg) {
  server.send(404, FPSTR(TEXT_PLAIN), msg);
}

void replyBadRequest(String msg) {
  DBG_OUTPUT_PORT.println(msg);
  server.send(400, FPSTR(TEXT_PLAIN), msg + "\r\n");
}

void replyServerError(String msg) {
  DBG_OUTPUT_PORT.println(msg);
  server.send(500, FPSTR(TEXT_PLAIN), msg + "\r\n");
}

#ifdef USE_SPIFFS
/*
   Checks filename for character combinations that are not supported by FSBrowser (alhtough valid on SPIFFS).
   Returns an empty String if supported, or detail of error(s) if unsupported
*/
String checkForUnsupportedPath(String filename) {
  String error = String();
  if (!filename.startsWith("/")) {
    error += F("!NO_LEADING_SLASH! ");
  }
  if (filename.indexOf("//") != -1) {
    error += F("!DOUBLE_SLASH! ");
  }
  if (filename.endsWith("/")) {
    error += F("!TRAILING_SLASH! ");
  }
  return error;
}
#endif


////////////////////////////////
// Request handlers

/*
   Return the FS type, status and size info
*/
void handleStatus() {
  DBG_OUTPUT_PORT.println("handleStatus");
  FSInfo fs_info;
  String json;
  json.reserve(128);

  json = "{\"type\":\"";
  json += fsName;
  json += "\", \"isOk\":";
  if (fsOK) {
    fileSystem->info(fs_info);
    json += F("\"true\", \"totalBytes\":\"");
    json += fs_info.totalBytes;
    json += F("\", \"usedBytes\":\"");
    json += fs_info.usedBytes;
    json += "\"";
  } else {
    json += "\"false\"";
  }
  json += F(",\"unsupportedFiles\":\"");
  json += unsupportedFiles;
  json += "\"}";

  server.send(200, "application/json", json);
}


/*
   Return the list of files in the directory specified by the "dir" query string parameter.
   Also demonstrates the use of chuncked responses.
*/
void handleFileList() {
  if (!fsOK) {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }

  if (!server.hasArg("dir")) {
    return replyBadRequest(F("DIR ARG MISSING"));
  }

  String path = server.arg("dir");
  if (path != "/" && !fileSystem->exists(path)) {
    return replyBadRequest("BAD PATH");
  }

  DBG_OUTPUT_PORT.println(String("handleFileList: ") + path);
  Dir dir = fileSystem->openDir(path);
  path.clear();

  // use HTTP/1.1 Chunked response to avoid building a huge temporary string
  if (!server.chunkedResponseModeStart(200, "text/json")) {
    server.send(505, F("text/html"), F("HTTP1.1 required"));
    return;
  }

  // use the same string for every line
  String output;
  output.reserve(64);
  while (dir.next()) {
#ifdef USE_SPIFFS
    String error = checkForUnsupportedPath(dir.fileName());
    if (error.length() > 0) {
      DBG_OUTPUT_PORT.println(String("Ignoring ") + error + dir.fileName());
      continue;
    }
#endif
    if (output.length()) {
      // send string from previous iteration
      // as an HTTP chunk
      server.sendContent(output);
      output = ',';
    } else {
      output = '[';
    }

    output += "{\"type\":\"";
    if (dir.isDirectory()) {
      output += "dir";
    } else {
      output += F("file\",\"size\":\"");
      output += dir.fileSize();
    }

    output += F("\",\"name\":\"");
    // Always return names without leading "/"
    if (dir.fileName()[0] == '/') {
      output += &(dir.fileName()[1]);
    } else {
      output += dir.fileName();
    }

    output += "\"}";
  }

  // send last string
  output += "]";
  server.sendContent(output);
  server.chunkedResponseFinalize();
}


/*
   Read the given file from the filesystem and stream it back to the client
*/
bool handleFileRead(String path) {
  DBG_OUTPUT_PORT.println(String("handleFileRead: ") + path);
  if (!fsOK) {
    replyServerError(FPSTR(FS_INIT_ERROR));
    return true;
  }

  if (path.endsWith("/")) {
    path += "index.htm";
  }

  String contentType;
  if (server.hasArg("download")) {
    contentType = F("application/octet-stream");
  } else {
    contentType = mime::getContentType(path);
  }

  if (!fileSystem->exists(path)) {
    // File not found, try gzip version
    path = path + ".gz";
  }
  if (fileSystem->exists(path)) {
    File file = fileSystem->open(path, "r");
    if (server.streamFile(file, contentType) != file.size()) {
      DBG_OUTPUT_PORT.println("Sent less data than expected!");
    }
    file.close();
    return true;
  }

  return false;
}


/*
   As some FS (e.g. LittleFS) delete the parent folder when the last child has been removed,
   return the path of the closest parent still existing
*/
String lastExistingParent(String path) {
  while (!path.isEmpty() && !fileSystem->exists(path)) {
    if (path.lastIndexOf('/') > 0) {
      path = path.substring(0, path.lastIndexOf('/'));
    } else {
      path = String();  // No slash => the top folder does not exist
    }
  }
  DBG_OUTPUT_PORT.println(String("Last existing parent: ") + path);
  return path;
}

/*
   Handle the creation/rename of a new file
   Operation      | req.responseText
   ---------------+--------------------------------------------------------------
   Create file    | parent of created file
   Create folder  | parent of created folder
   Rename file    | parent of source file
   Move file      | parent of source file, or remaining ancestor
   Rename folder  | parent of source folder
   Move folder    | parent of source folder, or remaining ancestor
*/
void handleFileCreate() {
  if (!fsOK) {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }

  String path = server.arg("path");
  if (path.isEmpty()) {
    return replyBadRequest(F("PATH ARG MISSING"));
  }

#ifdef USE_SPIFFS
  if (checkForUnsupportedPath(path).length() > 0) {
    return replyServerError(F("INVALID FILENAME"));
  }
#endif

  if (path == "/") {
    return replyBadRequest("BAD PATH");
  }
  if (fileSystem->exists(path)) {
    return replyBadRequest(F("PATH FILE EXISTS"));
  }

  String src = server.arg("src");
  if (src.isEmpty()) {
    // No source specified: creation
    DBG_OUTPUT_PORT.println(String("handleFileCreate: ") + path);
    if (path.endsWith("/")) {
      // Create a folder
      path.remove(path.length() - 1);
      if (!fileSystem->mkdir(path)) {
        return replyServerError(F("MKDIR FAILED"));
      }
    } else {
      // Create a file
      File file = fileSystem->open(path, "w");
      if (file) {
        file.write((const char *)0);
        file.close();
      } else {
        return replyServerError(F("CREATE FAILED"));
      }
    }
    if (path.lastIndexOf('/') > -1) {
      path = path.substring(0, path.lastIndexOf('/'));
    }
    replyOKWithMsg(path);
  } else {
    // Source specified: rename
    if (src == "/") {
      return replyBadRequest("BAD SRC");
    }
    if (!fileSystem->exists(src)) {
      return replyBadRequest(F("SRC FILE NOT FOUND"));
    }

    DBG_OUTPUT_PORT.println(String("handleFileCreate: ") + path + " from " + src);

    if (path.endsWith("/")) {
      path.remove(path.length() - 1);
    }
    if (src.endsWith("/")) {
      src.remove(src.length() - 1);
    }
    if (!fileSystem->rename(src, path)) {
      return replyServerError(F("RENAME FAILED"));
    }
    replyOKWithMsg(lastExistingParent(src));
  }
}


/*
   Delete the file or folder designed by the given path.
   If it's a file, delete it.
   If it's a folder, delete all nested contents first then the folder itself

   IMPORTANT NOTE: using recursion is generally not recommended on embedded devices and can lead to crashes (stack overflow errors).
   This use is just for demonstration purpose, and FSBrowser might crash in case of deeply nested filesystems.
   Please don't do this on a production system.
*/
void deleteRecursive(String path) {
  File file = fileSystem->open(path, "r");
  bool isDir = file.isDirectory();
  file.close();

  // If it's a plain file, delete it
  if (!isDir) {
    fileSystem->remove(path);
    return;
  }

  // Otherwise delete its contents first
  Dir dir = fileSystem->openDir(path);

  while (dir.next()) {
    deleteRecursive(path + '/' + dir.fileName());
  }

  // Then delete the folder itself
  fileSystem->rmdir(path);
}


/*
   Handle a file deletion request
   Operation      | req.responseText
   ---------------+--------------------------------------------------------------
   Delete file    | parent of deleted file, or remaining ancestor
   Delete folder  | parent of deleted folder, or remaining ancestor
*/
void handleFileDelete() {
  if (!fsOK) {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }

  String path = server.arg(0);
  if (path.isEmpty() || path == "/") {
    return replyBadRequest("BAD PATH");
  }

  DBG_OUTPUT_PORT.println(String("handleFileDelete: ") + path);
  if (!fileSystem->exists(path)) {
    return replyNotFound(FPSTR(FILE_NOT_FOUND));
  }
  deleteRecursive(path);

  replyOKWithMsg(lastExistingParent(path));
}

/*
   Handle a file upload request
*/
void handleFileUpload() {
  if (!fsOK) {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }
  if (server.uri() != "/edit") {
    return;
  }
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    // Make sure paths always start with "/"
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    DBG_OUTPUT_PORT.println(String("handleFileUpload Name: ") + filename);
    uploadFile = fileSystem->open(filename, "w");
    if (!uploadFile) {
      return replyServerError(F("CREATE FAILED"));
    }
    DBG_OUTPUT_PORT.println(String("Upload: START, filename: ") + filename);
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) {
      size_t bytesWritten = uploadFile.write(upload.buf, upload.currentSize);
      if (bytesWritten != upload.currentSize) {
        return replyServerError(F("WRITE FAILED"));
      }
    }
    DBG_OUTPUT_PORT.println(String("Upload: WRITE, Bytes: ") + upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (uploadFile) {
      uploadFile.close();
    }
    DBG_OUTPUT_PORT.println(String("Upload: END, Size: ") + upload.totalSize);
  }
}


void handleFile(String file) {
  if (!fsOK) {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }

  if (handleFileRead(file)) {
    return;
  }
    // Dump debug data
  String message;
  message.reserve(100);
  message = F("Error: File not found\n\nURI: ");
  message += file;
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += server.args();
  message += '\n';
  for (uint8_t i = 0; i < server.args(); i++) {
    message += F(" NAME:");
    message += server.argName(i);
    message += F("\n VALUE:");
    message += server.arg(i);
    message += '\n';
  }
  message += "path=";
  message += server.arg("path");
  message += '\n';
  DBG_OUTPUT_PORT.print(message);

  return replyNotFound(message);
  
}

/*
   The "Not Found" handler catches all URI not explicitely declared in code
   First try to find and return the requested file from the filesystem,
   and if it fails, return a 404 page with debug information
*/
void handleNotFound() {
  if (!fsOK) {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }

  String uri = ESP8266WebServer::urlDecode(server.uri()); // required to read paths with blanks

  handleFile(uri);
}

void handleIndex(){
  handleFile("/index.html");
}



/*
   This specific handler returns the index.htm (or a gzipped version) from the /edit folder.
   If the file is not present but the flag INCLUDE_FALLBACK_INDEX_HTM has been set, falls back to the version
   embedded in the program code.
   Otherwise, fails with a 404 page with debug information
*/
void handleGetEdit() {
  if (handleFileRead(F("/edit/index.htm"))) {
    return;
  }

#ifdef INCLUDE_FALLBACK_INDEX_HTM
  server.sendHeader(F("Content-Encoding"), "gzip");
  server.send(200, "text/html", index_htm_gz, index_htm_gz_len);
#else
  replyNotFound(FPSTR(FILE_NOT_FOUND));
#endif

}

ESP8266WiFiMulti wiFiMulti;

void setup(void) {
  ////////////////////////////////
  // SERIAL INIT
  DBG_OUTPUT_PORT.begin(115200);
  DBG_OUTPUT_PORT.setDebugOutput(true);
  DBG_OUTPUT_PORT.print('\n');

  ////////////////////////////////
  // FILESYSTEM INIT

  fileSystemConfig.setAutoFormat(false);
  fileSystem->setConfig(fileSystemConfig);
  fsOK = fileSystem->begin();
  DBG_OUTPUT_PORT.println(fsOK ? F("Filesystem initialized.") : F("Filesystem init failed!"));

#ifdef USE_SPIFFS
  // Debug: dump on console contents of filessytem with no filter and check filenames validity
  Dir dir = fileSystem->openDir("");
  DBG_OUTPUT_PORT.println(F("List of files at root of filesystem:"));
  while (dir.next()) {
    String error = checkForUnsupportedPath(dir.fileName());
    String fileInfo = dir.fileName() + (dir.isDirectory() ? " [DIR]" : String(" (") + dir.fileSize() + "b)");
    DBG_OUTPUT_PORT.println(error + fileInfo);
    if (error.length() > 0) {
      unsupportedFiles += error + fileInfo + '\n';
    }
  }
  DBG_OUTPUT_PORT.println();

  // Keep the "unsupportedFiles" variable to show it, but clean it up
  unsupportedFiles.replace("\n", "<br/>");
  unsupportedFiles = unsupportedFiles.substring(0, unsupportedFiles.length() - 5);
#endif


  ////////////////////////////////
  // CONFIG INIT
  loadConfiguration(configFile, config);


  ////////////////////////////////
  // Sensor Init
  bme.begin(0x76);

  ////////////////////////////////
  // OUTPUT INIT
  for( int x=0; x < relesQtd; x++ ) {
    pinMode(relesPins[x], OUTPUT);  
    output(x, LOW);
  }

  ////////////////////////////////
  // WI-FI INIT
  DBG_OUTPUT_PORT.printf("Connecting to %s\n", ssid);
  WiFi.mode(WIFI_STA);
  wiFiMulti.addAP(ssid, password);
  // Wait for connection
  while (wiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    DBG_OUTPUT_PORT.print(".");
  }

  ////////////////////////////////
  // WEB SERVER INIT
  
  // Root file
  server.on("/",  HTTP_GET, handleIndex);

  // Filesystem status
  server.on("/status", HTTP_GET, handleStatus);

  // List directory
  server.on("/list", HTTP_GET, handleFileList);

  // Load editor
  server.on("/edit", HTTP_GET, handleGetEdit);

  // Create file
  server.on("/edit",  HTTP_PUT, handleFileCreate);

  // Delete file
  server.on("/edit",  HTTP_DELETE, handleFileDelete);

  // Upload file
  // - first callback is called after the request has ended with all parsed arguments
  // - second callback handles file upload at that location
  server.on("/edit",  HTTP_POST, replyOK, handleFileUpload);

  //reload config file
  server.on("/reloadConfig.do",  HTTP_GET, handleReloadConfig);

  //Muda o estado da Irrigação
  server.on("/irrigaChange.do",  HTTP_GET, handleIrrigaChange);

  //Muda o estado da Iluminação
  server.on("/iluminaChange.do",  HTTP_GET, handleIluminaChange);

    //Pulsa irrigação
  server.on("/irrigaPulse.do",  HTTP_GET, handleIrrigaPulse);

  server.on("/readInfo.do",  HTTP_GET, handleReadInfo);

  // Testes
  server.on("/teste", HTTP_GET, handleTeste);


  // Default handler for all URIs not defined above
  // Use it to read files from filesystem
  server.onNotFound(handleNotFound);

  // Start server
  server.begin();
  DBG_OUTPUT_PORT.println("HTTP server started");
 

  //Start Time Client
  timeClient.begin();


  //-----------------------
  // TIMERS SETUP
  //-----------------------
  if( config.logCloudInterval > 0 ){
    Serial.print("Config log to cloud interval to ");
    Serial.print(config.logCloudInterval);
    Serial.println(" seconds");
    dataLogTimer.attach(config.logCloudInterval, dataLogPostToCloud );
  } else {
    Serial.print("Log to cloud disabled");
  }
    

  checkAlarmTimer.attach(CHECK_ALARM_INTERVAL, checkAlarm );
 
}

boolean postToCloud = false;

void loop(void) {
  server.handleClient();

  //It must be in the main loop :-P
  if( postToCloud ){
     
     updateBME();
     batteryValue = analogRead(batteryPin);

    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    client->setFingerprint(fingerprint);
    HTTPClient https;

    String path;
    path.reserve(256);
    path = "https://robotizando.com.br/xcontrol/log.php?did=";
    path += config.deviceUUID;
    path += "&t=";
    path += temperature;
    path += "&u=";
    path += humidity;
    path += "&p=";
    path += pressure;
    path += "&b=";
    path += (batteryValue-adoffset)*voltsPerUnit;

    if (https.begin(*client, path.c_str())) {  // HTTPS
        Serial.print("[HTTPS] GET...\n");
        // start connection and send HTTP header
        int httpCode = https.GET();
  
        // httpCode will be negative on error
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
  
          // file found at server
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = https.getString();
            if( payload.equals("rega") ){
                output( IRRIGA, HIGH );
                irrigaStatus = HIGH;
                delay(config.intervalPulse1*1000);
                output( IRRIGA, LOW );
                irrigaStatus = LOW;
            }
            
            Serial.println(payload);
          }
        } else {
          Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
        }
        https.end();
     } else {
        Serial.printf("[HTTPS] Unable to connect\n");
     }

    postToCloud = false;

  }
  
}


//-------------------------------------------------------------
// Meus handlers
//-------------------------------------------------------------



void handleIrrigaInfo(){
  handleReadInfo();
}

void handleIrrigaChange(){
  String json;
    
  if( irrigaStatus == LOW ){
    irrigaStatus = HIGH;
  } else {
    irrigaStatus = LOW;
  }

  output( IRRIGA, irrigaStatus );
  handleReadInfo();
}

void handleIluminaChange(){
  String json;
    
  if( iluminaStatus == LOW ){
    iluminaStatus = HIGH;
  } else {
    iluminaStatus = LOW;
  }

  output( ILUMINA, iluminaStatus );
  handleReadInfo();
}

void handleIrrigaPulse(){
  handleReadInfo();
  output( IRRIGA, HIGH );
  irrigaStatus = HIGH;
  delay(config.intervalPulse1*1000);
  output( IRRIGA, LOW );
  irrigaStatus = LOW;
}



void handleReloadConfig(){
    loadConfiguration(configFile, config);
    server.send(200, "text/plain", "ok");
}




/*
 * Signal data order
 * - battery level
 * - temperature
 * - humidity
 * - pressure
 * - altitude
 * - actuator 1 status
 * - actuator 2 status 
 * - next actuator 1 schedule in seconds
 * - next actuator 2 schedule in seconds
 */


void handleReadInfo(){
   updateBME();
   batteryValue = analogRead(batteryPin);

  String response;
  response.reserve(128);
  response = (batteryValue-adoffset)*voltsPerUnit;
  response += ";";
  response += temperature;
  response += ";";
  response += humidity;
  response += ";";
  response += pressure;
  response += ";";
  response += altitude;
  response += ";";
  response += irrigaStatus;
  response += ";";
  response += iluminaStatus;
  response += ";";
  server.send(200, "text/plain", response);
}




void dataLogPostToCloud(){
  postToCloud = true;
}



void handleTeste(){

  String response;
  response.reserve(128);
  
  response = "Current time in millis : ";
  response += timeUpdate(); 
  response += " - ";
  response += getFormattedTime( currentTime );
  Serial.println( response );
  server.send(200, "text/plain", response);
}



unsigned long timeUpdate(){
  String response;
  response.reserve(128);
  timeClient.update();
  currentTime = timeClient.getEpochTime();
  return currentTime;
}


String getFormattedTime( unsigned long rawTime ) {
  unsigned long hours = (rawTime % 86400L) / 3600;
  String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);
  unsigned long minutes = (rawTime % 3600) / 60;
  String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);
  unsigned long seconds = rawTime % 60;
  String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);
  return hoursStr + ":" + minuteStr + ":" + secondStr;
}


String dumpConfiguration(){

  String response;
  response.reserve(128);
  response = F("Device UUID : ");
  response += config.deviceUUID;
  response += F("\nPort 1 pulse schedule (in seconds) :");
  response += config.schedulePulse1;
  response += F("\nPort 1 pulse interval (in seconds) :");
  response += config.intervalPulse1;
  response += F("\nPort 2 pulse schedule (in seconds) :");
  response += config.schedulePulse2;
  response += F("\nPort 2 pulse interval (in seconds) :");
  response += config.intervalPulse2;

  Serial.println( response );

  return response;  

}



void checkAlarm(){
  timeUpdate();

  //Serial.println(  getFormattedTime( currentTime ) );
}
