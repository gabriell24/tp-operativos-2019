

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_


#include <commons/bitarray.h>
#include <commons/string.h>
#include <commons/config.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include "consola.h"
#include "configuracion.h"

#define SIN_BLOQUES_LIBRES -1

typedef struct {
	int tamanio_bloques;
	int cantidad_bloques;
	char magic_number[10];
	t_bitarray *bitarray;
	int ultimo_bloque_retornado;
	char *path_raiz;
	//char *ruta_metadata_file;
	//char *ruta_bitmap_file;
} t_datos_fs;

typedef struct {
	char *consistency;
	uint16_t partitions;
	int compaction_time;
} t_metadata;

//t_bitarray *bitarray;
t_datos_fs datos_fs;
t_config *metadata;
t_config *datos_archivo;
t_config *datos_archivo_propio;

void iniciar_fs(char *path);
void escribir(int fd, char *texto);
void cargar_metadata(char *path, char *archivo);
void finalizar_estructuras_fs();
int tomar_bloque_libre();
void guardar_bitmap(char *path_bitmap);
void guardar_archivo_metadata(char *tabla, char *criterio, int particiones, int compaction_time);
void crear_carpeta_tabla(char *tabla);
void crear_archivo_bloque(int bloque, char *contenido);
void crear_archivo_particion(char*, int, int);
char *path_tablas();
char *path_bloques();
void check(int posicion, bool esCorrecto);
char bit_in_char(int bit, int mode);
bool crear_sub_rutas(char*);
bool existe_tabla(char*);
char* obtener_datos(char *path, uint16_t key);
bool matchea_key_en_linea(char *linea, uint16_t key);
t_metadata obtener_metadata(char *tabla);

#endif /* FILESYSTEM_H_ */
