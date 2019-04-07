#!/bin/bash

#Limpio la pantalla
printf "\033c"
pathActual=$(pwd)

#Compilar Kernel
echo "Empiezo a compilar Kernel"
cd $pathActual/kernel
source compilar_kernel.sh;
echo "Fin proceso de compilar Kernel"

#Compilar Memoria
echo "Empiezo a compilar Memoria"
cd $pathActual/memoria
source compilar_memoria.sh;
echo "Fin proceso de compilar Memoria"

#Compilar FS
echo "Empiezo a compilar FS"
cd $pathActual/lissandra
source compilar_lissandra.sh;
echo "Fin proceso de compilar FS"

#cd $pathActual
