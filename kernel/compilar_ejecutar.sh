#!/bin/bash
source compilar_kernel.sh
if [ $? == 0 ]; then
  ./kernel.out
fi
