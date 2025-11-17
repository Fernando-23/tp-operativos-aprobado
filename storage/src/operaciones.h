#ifndef OPERACIONES_H_
#define OPERACIONES_H_
#include "utils_storage.h"
#include <sys/stat.h>
#include "../../utils/src/utils/helpers.h"

typedef enum{
    CREATE,
    TRUNCATE,
    TAG,
    COMMIT,
    FLUSH,
    DELETE,
    LEER_BLOQUE,
    ACTUALIZAR_FRAME_MODIFICADO,
    ERROR
}CodOperacionStorage;

void* recursosHumanos(void*);//atenderClientes

void* atenderLaburanteDisconforme(void*);//atenderCliente
void pedidoDeLaburante(int mail_laburante);
bool realizarCREATE(char* query_id,char* nombre_file, char* nombre_tag);
t_config* crearMetadata(char* path_tag);
File* crearFile(char* nombre_file,char* path_file);
Tag* crearTag(char* nombre_tag,char* path_tag);
void crearDirectorio(char* path_directorio);

void asignarFileTagAChars(char* nombre_file,char* tag,char* file_a_cortar);
File* buscarFilePorNombre(char* nombre);
Tag* buscarTagPorNombre(t_list* tags,char* nombre_tag);

BloqueLogico* crearBloqueLogico(BloqueFisico* block0, int nro_bloque,char* id_bloque_fisico_hlink);
void liberarBloqueLogico(BloqueLogico* bloque_a_liberar);

void gestionarTruncateSegunTamanio(Tag* tag_concreto, int tamanio_a_truncar);

void asignarBloquesFisicosATag(Tag* tag_a_asignar_hardlinks,char** bloques_fisicos_asignados);

bool crearArchBloqueLogico(int nro_bloque,char* path_directorio_logico,char* path_bloque_fisico_hlink);

void eliminarRespuestaConsultaBitmap(RespuestaConsultaBitmap* response_a_limpiar);
void limpiarBitsPorStringArray(char** bloques_a_limpiar);
char* obtenerNombreBloqueConCeros(int numero);

bool tieneHLinks(char* ruta_abs_a_consultar);
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

//--------RETARDOS----------
void hacerRetardoOperacion();

#endif //OPERACIONES_H_