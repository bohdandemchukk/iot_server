#!/bin/bash
g++ -std=c++23 -g \
    tests.cpp \
    WeatherCache.cpp \
    -o tests_results \
    -lgtest -lgtest_main -pthread \
    && ./tests_results --gtest_output=xml:tests/results.xml
