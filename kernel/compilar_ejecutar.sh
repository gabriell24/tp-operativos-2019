#!/bin/bash
source compilar_kernel.sh
if [ $? == 0 ]; then
  echo "./compilar_ejecutar {valgrind}Opcional {leaks}Opcional"
  if [ $1 = "valgrind" ]; then
    if [ $2 = "leaks" ]; then
      valgrind --leak-check=yes ./kernel.out
    else
      valgrind ./kernel.out
    fi
  else
    ./kernel.out
  fi
fi
