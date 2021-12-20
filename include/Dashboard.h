#pragma once
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DHTesp.h>
#include <HumiditySensor.h>
#include <Reservoir.h>

ESP8266WebServer server(80);

class Dashboard {
public:
    DHTesp* dht;
    HumiditySensor* hSensor;
    Reservoir* reservoir;
    
    Dashboard ( DHTesp* dht, HumiditySensor* hSensor, Reservoir* reservoir ) {
        this->dht = dht;
        this->hSensor = hSensor;
        this->reservoir = reservoir;
    }

    void init ( ) {
        connect();
        
        if (MDNS.begin("esp8266")) {
            Serial.println("MDNS responder started");
        }
    }

    void connect ( ) {
        setUpRouting();
        server.onNotFound(handleNotFound);
        server.begin();
        Serial.println("HTTP server started");
    }

    void checkAndReconnect ( ) {
        if (WiFi.status() != WL_CONNECTED) connect();
    }

    void disconnect ( ) {
        server.close();
        server.stop();
    }

    static void handleNotFound() {
        String message = "File Not Found\n\n";
        message += "URI: ";
        message += server.uri();
        message += "\nMethod: ";
        message += (server.method() == HTTP_GET) ? "GET" : "POST";
        message += "\nArguments: ";
        message += server.args();
        message += "\n";
        for (uint8_t i = 0; i < server.args(); i++) {
            message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
        }
        server.send(404, "text/plain", message);
    }

    String getIp ( ) {
        return WiFi.localIP().toString();
    }

    void listen ( ) {
        checkAndReconnect();
        server.handleClient();
    }

private:

    void setUpRouting ( ) {
        server.on("/test", HTTP_GET, []() {
            server.send(200, F("text/html"),
                F("HAHA troxa"));
        });
    
        server.on(F("/"), HTTP_GET, getIndex);

        server.on(F("/data"), HTTP_GET, [this] ( ) { 
            TempAndHumidity dhtReadings = dht->getTempAndHumidity();
            String name = Configurations::PLANT_NAME;
            float temp = dhtReadings.temperature;
            float hum = dhtReadings.humidity;
            int lifeTime = Configurations::PLANT_TIME_DAYS;
            int soilHumidity = hSensor->getPercentage();
            int waterLevel = reservoir->getLevelPercentage();
            server.send(200, "application/json", "{\"name\":\""+name+"\",\"temperature\":"+String(temp)+",\"airHumidity\":"+String(hum)+",\"lifeTime\":"+String(lifeTime)+",\"soilHumidity\":"+String(soilHumidity)+",\"waterLevel\":"+String(waterLevel)+"}");
        } );
    }

    static void getIndex() {
        server.send(200, "text/html", "<!DOCTYPE html><html lang='pt-BR'> <head> <meta charset='utf-8'> <meta name='viewport' content='width=device-width,initial-scale=1'> <title>Plantinha</title> <link rel='stylesheet' type='text/css' href='css/style.css'> <link rel='shortcut icon' type='image/x-icon' href=''> <script src='https://ajax.googleapis.com/ajax/libs/jquery/3.5.1/jquery.min.js'></script> <link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/materialize/1.0.0/css/materialize.min.css'> <script src='https://cdnjs.cloudflare.com/ajax/libs/materialize/1.0.0/js/materialize.min.js'></script> <link href='https://fonts.googleapis.com/icon?family=Material+Icons' rel='stylesheet'> <link rel='preconnect' href='https://fonts.googleapis.com'> <link rel='preconnect' href='https://fonts.gstatic.com' crossorigin> <link href='https://fonts.googleapis.com/css2?family=Montserrat:wght@100;200;300;400;600&display=swap' rel='stylesheet'> </head> <body class='row align-center vh100 m0'> <style>body{background-color: #f1f9f5; font-family: 'Montserrat', sans-serif;}input{border: 0 !important; box-shadow: none !important;}input#name{font-size: 2rem; font-weight: 400; text-transform: uppercase; color: #7cd6c1;}.row{display: flex; flex-direction: row;}.column{display: flex; flex-direction: column;}.justify-center{justify-content: center;}.align-center{align-items: center;}.text-center{text-align: center;}.h100{height: 100%;}.vh100{height: 100vh;}.w100{width: 100%;}.w50{width: 50%;}.m0{margin: 0;}div{margin: 0 !important;}.my05{margin: 0.5rem 0 0.5rem 0 !important;}.mx05{margin: 0 0.5rem 0 0.5rem !important;}.mx025{margin: 0 0.25rem 0 0.25rem !important;}.stat{display: flex; flex-direction: column; border: 1px solid #d3d3d3; border-radius: 0.5rem; padding: 1.5rem; color: #aaaaaa; font-size: 1.5rem; align-items: center; justify-content: center; margin: 0 0.5rem 0 0.5rem;}.stat > i{font-size: 2.5rem;}</style> <section class='column justify-center h100' style='width: 50%;'> <img style='width: 100%;' src='https://i.pinimg.com/originals/eb/22/3f/eb223f5ce629d66ae9a02d5afeace013.gif'> <input class='text-center' type='text' id='name' placeholder='Nome' style='letter-spacing: 1rem; margin-top: -2rem; margin-left: 0.5rem;' value='cezinha'> </section> <section class='column justify-center align-center w50'> <div class='column w50'> <div class='row w100'> <div class='stat w50 mx025'> <i class='material-icons'>brightness_7</i> <div id='temp'> 21ºC </div></div><div class='stat w50 mx025'> <i class='material-icons'>filter_drama</i> <div id=''> 50% </div></div></div><div class='row w100 my05'> <div class='stat w100 mx025'> <i class='material-icons'>all_inclusive</i> <div id=''> 200 dias </div></div></div><div class='row w100'> <div class='stat w50 mx025'> <i class='material-icons'>local_drink</i> <div id=''> 50% </div></div><div class='stat w50 mx025'> <i class='material-icons'>opacity</i> <div id=''> 50% </div></div></div></div></section> </body></html>");
    }
};