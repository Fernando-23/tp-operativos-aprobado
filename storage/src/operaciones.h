#ifndef OPERACIONES_H_
#define OPERACIONES_H_

#include <sys/stat.h>
#include <stdlib.h>
#include "../../utils/src/utils/helpers.h"
#include "../../utils/src/utils/conexiones.h"

#include "utils_storage.h"
#include "errores.h"

typedef enum{
    CREATE,
    TRUNCATE,
    TAG,
    COMMIT,
    DELETE,
    LEER_BLOQUE,
    ESCRIBIR_BLOQUE,
    CARTA_DOCUMENTO
}CodOperacionStorage;


void* recursosHumanos(void*);//atenderClientes

void* atenderLaburanteDisconforme(void*);//atenderCliente
void pedidoDeLaburante(int mail_laburante);


void actualizarTamanioMetadata(char *nombre_file, Tag *tag, int tamanio_a_truncar);

void unlinkearBloquesLogicos(int cant_a_unlinkear, t_list *bloques_logicos);

void agrandarEnTruncate(Tag *tag, int tamanio_acutal, int nuevo_tamanio);

ErrorStorageEnum realizarCREATE(int query_id, char *nombre_file, char *nombre_tag);
ErrorStorageEnum realizarTRUNCATE(int query_id,char* nombre_file, char* nombre_tag, int tamanio_a_truncar);
ErrorStorageEnum realizarTAG(int query_id, char *nombre_file_origen,
char* nombre_tag_origen, char* nombre_file_destino,char* nombre_tag_destino);
ErrorStorageEnum realizarELIMINAR_UN_TAG(char* query_id,char *nombre_file,char *nombre_tag);
ErrorStorageEnum realizarCOMMIT(int query_id,char *nombre_file,char *nombre_tag);


Mensaje* mensajitoResultadoStorage(ErrorStorageEnum);

t_config* crearMetadata(char* path_tag);
File* crearFile(char* nombre_file);
Tag* crearTag(char* nombre_tag, char* nombre_file_asociado);
void crearDirectorio(char* path_directorio);

void asignarFileTagAChars(char* nombre_file,char* tag,char* file_a_cortar);
File* buscarFilePorNombre(char* nombre);
Tag* buscarTagPorNombre(t_list* tags,char* nombre_tag);
BloqueLogico* buscarBloqueLogicoPorId(int id_logico);
BloqueFisico* buscarBloqueFisicoPorNombre(char *nombre);
BloqueFisico* buscarBloqueFisicoPorId(int id_fisico);
BloqueLogico* crearBloqueLogico(int nro_bloque_logico, BloqueFisico *bloque_fisico_a_asignar, char *path_tag);

bool crearHLink(char *ruta_hl_del_bloque_logico, char *bloque_fisico_a_hardlinkear);

void asignarBloquesFisicosATagCopiado(Tag *tag_destino);

void liberarBloqueLogico(BloqueLogico* bloque_a_liberar);
void liberarBloqueDeBitmap(int nro_bloque, int query_id);

void unlinkearBloquesLogicosParaELIMINAR_UN_TAG(int query_id,int cant_a_unlinkear,Tag* tag);

void gestionarTruncateSegunTamanio(Tag* tag_concreto, int tamanio_a_truncar);

void asignarBloquesFisicosATagEnTruncate(Tag *tag_a_asignar_hardlinks, int cant_bloques_necesarios);

void crearArchBloqueLogico(int nro_bloque,char* path_directorio_logico);

void eliminarRespuestaConsultaBitmap(RespuestaConsultaBitmap* response_a_limpiar);
void limpiarBitsPorStringArray(char** bloques_a_limpiar);
char* obtenerNombreBloqueConCeros(int numero);

bool tieneHLinks(char* ruta_abs_a_consultar);
int contadorHLinks(char *ruta_abs_a_consultar);
//////////////////////////////////////////////////////////////////////////
// FER 
// y
// la 
//mexicana 

void ferConLaMexicana(Tag* tag, int tamanio_actual,int nuevo_tamanio);
//esto fue pusheado en un liveshare
// fer no se hace responsable del bullying que le hacen
//////////////////////////////////////////////////////////////////////////

char* stringArrayConfigAString(char** array_a_pasar_a_string);

BloqueFisico *obtenerBloqueFisico(int nro_bloque_a_buscar);

void escribirEnHashIndex(Tag *tag);
DatosParaHash *obtenerDatosParaHash(BloqueLogico *bloque_logico);

ErrorStorageEnum realizarTRUNCATE(int query_id, char *file_completo, int tamanio_a_truncar);




//--------RETARDOS----------
void hacerRetardoOperacion();

#endif //OPERACIONES_H_