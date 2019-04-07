#!/bin/bash
source compilar_memoria.sh
if [ $? == 0 ]; then
  ./memoria.out
fi
