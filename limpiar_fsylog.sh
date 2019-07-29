#!/bin/bash
rm -rf otros/lfs/Bloques/*
rm otros/lfs/Metadata/Bitmap.bin
rm -rf otros/lfs/Tables/*

rm -rf kernel/kernel.log
rm -rf lissandra/lissandra.log
rm -rf memoria/memoria.log
rm -rf otros/lfs/exportacion.xls
rm -rf otros/memoria2/memoria.log
rm -rf otros/memoria3/memoria.log
rm -rf otros/memoria4/memoria.log
printf "\n\n";
printf "No te olvides de editar:\n";
printf "kernel.config\n";
printf "memoria.config\n";
printf "lissandra.config\n";
printf "\n\n";
