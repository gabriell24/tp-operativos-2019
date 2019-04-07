#!/bin/bash
source compilar_lissandra.sh
if [ $? == 0 ]; then
  ./lissandra.out
fi
