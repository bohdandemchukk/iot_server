#!/bin/bash
g++ ConsoleApplication1.cpp -o server -lpaho-mqttpp3 -lpaho-mqtt3as && ./server
