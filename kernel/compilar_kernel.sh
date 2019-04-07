#!/bin/bash
rm -rf kernel.out
COLOR_ROJO='\e[0;31m'
COLOR_NORMAL='\e[0m'
COLOR_VERDE='\e[1;32m'
archivosBibliotecaCompartida=$(cd ../shared_library; ls $PWD/*.c)
referencias="-lcommons -lpthread -lreadline"

echo "Compilando proyecto Kernel";
gcc *.c ${archivosBibliotecaCompartida} -o kernel.out ${referencias};

if [ $? != 0 ]; then
  echo -e "${COLOR_ROJO}FALLÓ LA COMPILACIÓN${COLOR_NORMAL}"
else
  echo -e "${COLOR_VERDE}COMPILOOOOOOO LPM${COLOR_NORMAL}"
fi