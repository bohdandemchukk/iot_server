#!/bin/bash
sed -i 's/\r//' src/*.cpp include/*.h
g++ -std=c++23 -g -fsanitize=address src/*.cpp -I include -o server -lpaho-mqttpp3 -lpaho-mqtt3as && ./server


