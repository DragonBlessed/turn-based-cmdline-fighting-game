#!/bin/bash

set -e
g++ -std=c++17 -Wall -Wextra -Wpedantic fighter.cpp tests/fighter_tests.cpp -o /tmp/fighter_tests
/tmp/fighter_tests
