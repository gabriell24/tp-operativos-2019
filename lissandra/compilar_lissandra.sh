#!/bin/bash
rm -rf lissandra.out
COLOR_ROJO='\e[0;31m'
COLOR_NORMAL='\e[0m'
COLOR_VERDE='\e[1;32m'
archivosBibliotecaCompartida=$(cd ../shared_library; ls $PWD/*.c)
referencias="-lcommons -lpthread -lreadline"

echo "Compilando proyecto Lissandra";
gcc *.c ${archivosBibliotecaCompartida} -ggdb3 -o lissandra.out ${referencias};

if [ $? != 0 ]; then
  echo -e "${COLOR_ROJO}FALLO LA COMPILACIÓN${COLOR_NORMAL}"
else
  echo -e "${COLOR_VERDE}COMPILOOOOOOO LPM${COLOR_NORMAL}"
fi
