#!/bin/bash
source compilar_lissandra.sh
if [[ $? == 0 ]]; then
  echo "RECORDA QUE PODES USAR: ./compilar_ejecutar {valgrind}Opcional {leaks}Opcional"
    if [[ $1 = "valgrind" ]]; then
      if [[ $2 = "leaks" ]]; then
        valgrind --leak-check=yes ./lissandra.out
      else
        valgrind ./lissandra.out
      fi
    else
      ./lissandra.out
    fi
fi
