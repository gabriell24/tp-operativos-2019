#!/bin/bash
rm -rf memoria.out
COLOR_ROJO='\e[0;31m'
COLOR_NORMAL='\e[0m'
COLOR_VERDE='\e[1;32m'
archivosBibliotecaCompartida=$(cd ../shared_library; ls $PWD/*.c)
referencias="-lcommons -lpthread -lreadline"

echo "Compilando proyecto Memoria";
gcc *.c ${archivosBibliotecaCompartida} -ggdb3 -o memoria.out ${referencias};

if [ $? != 0 ]; then
  echo -e "${COLOR_ROJO}FALLO LA COMPILACIÓN${COLOR_NORMAL}"
else
  echo -e "${COLOR_VERDE}COMPILOOOOOOO LPM${COLOR_NORMAL}"
fi
