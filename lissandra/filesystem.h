

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_


#include <commons/bitarray.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include "consola.h"
#include "configuracion.h"
#include "lissandra.h"
#include "../shared_library/estructuras_compartidas.h"

#define SIN_BLOQUES_LIBRES -1

typedef struct {
	int tamanio_bloques;
	int cantidad_bloques;
	char magic_number[10];
	t_bitarray *bitarray;
	int ultimo_bloque_retornado;
	char *path_raiz;
	char *_bitmap;
	//char *ruta_metadata_file;
	//char *ruta_bitmap_file;
} t_datos_fs;

typedef struct {
	char *consistency;
	uint16_t partitions;
	int compaction_time;
} t_metadata;

typedef struct {
	int timestamp;
	uint16_t key;
	char *value;
} t_registro;

typedef struct {
	char *tabla;
	t_list *t_registro;
} t_memtable;

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
t_timestamp_value *cargar_datos_timestamp_value(char *linea);
t_timestamp_value *obtener_datos_de_particion(char *path, uint16_t key);
bool matchea_key_en_linea(char *linea, uint16_t key);
t_metadata obtener_metadata(char *tabla);
t_memtable *obtener_tabla_en_memtable(char *tabla);
t_registro *obtener_registros_por_key(char *tabla, uint16_t key);
void printear_memtable();
void limpiar_registros_memtable(t_registro *unRegistro);
void limpiar_tablas_memtable(t_memtable *unaTabla);
t_timestamp_value *devolver_timestamp_mayor(t_timestamp_value *uno, t_timestamp_value *otro);
void limpiar_timestampvalue_si_corresponde(t_timestamp_value *registro);
t_response_describe *devolver_metadata(char *path_tabla, char *tabla);
#endif /* FILESYSTEM_H_ */
