#!/bin/bash
rm -rf otros/lfs/Bloques/*
rm otros/lfs/Metadata/Bitmap.bin
rm -rf otros/lfs/Tables/*

rm -rf kernel/kernel.log
rm -rf lissandra/lissandra.log
rm -rf memoria/memoria.log
printf "\n\n";
printf "No te olvides de editar:\n";
printf "kernel.config\n";
printf "memoria.config\n";
printf "lissandra.config\n";
printf "\n\n";
