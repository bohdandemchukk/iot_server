#!/bin/bash
sed -i 's/\r//' *.cpp *.h
g++ ConsoleApplication1.cpp Callback.cpp InfluxWriter.cpp HttpServer.cpp InfluxReader.cpp WeatherCache.cpp -o server -lpaho-mqttpp3 -lpaho-mqtt3as && ./server


