cmake_minimum_required(VERSION 3.28)
project(Test_version1 C)

set(CMAKE_C_STANDARD 23)

add_executable(Test_version Src/main.c Src/evaluate.c Src/forbidden_hand.c Src/game.c Src/hash.c Src/minimax.c)
