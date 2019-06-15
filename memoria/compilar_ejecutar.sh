#!/bin/bash
source compilar_memoria.sh
if [ $? == 0 ]; then
  echo "./compilar_ejecutar {valgrind}Opcional {leaks}Opcional"
  if [ $1 = "valgrind" ]; then
    if [ $2 = "leaks" ]; then
      valgrind --leak-check=yes ./memoria.out
    else
      valgrind ./memoria.out
    fi
  else
    ./memoria.out
  fi
fi
