# Checklist para Pruebas Finales - Master of Files

## ✅ Verificaciones Generales

### Sistema Completo
- [ ] El deploy se hace compilando los módulos en las máquinas del laboratorio en menos de 10 minutos
- [ ] Los procesos se ejecutan de forma simultánea y la cantidad de hilos y subprocesos es la adecuada
- [ ] Los procesos establecen conexiones TCP/IP
- [ ] El sistema no registra casos de Espera Activa ni Memory Leaks
- [ ] El log respeta los lineamientos de logs mínimos y obligatorios de cada módulo
- [ ] El sistema no requiere permisos de superuser (sudo/root) para ejecutar correctamente
- [ ] El sistema no requiere de Valgrind o algún proceso similar para ejecutar correctamente
- [ ] El sistema utiliza una sincronización determinística (no utiliza más sleeps de los solicitados)

## 📋 Módulo Master

### Funcionalidades
- [ ] Se permite la conexión y desconexión de los worker sin presentar errores
- [ ] Se permite la conexión y desconexión de las Queries sin presentar errores
- [ ] El planificador respeta las prioridades definidas
- [ ] El aging se aplica de manera correcta
- [ ] El master reenvía los mensajes desde el Worker a la query sin problemas

### Logs Obligatorios (verificar formato exacto con "##")
- [ ] "## Se conecta un Query Control para ejecutar la Query <PATH_QUERY> con prioridad <PRIORIDAD> - Id asignado: <QUERY_ID>. Nivel multiprocesamiento <CANTIDAD>"
- [ ] "## Se conecta el Worker <WORKER_ID> - Cantidad total de Workers: <CANTIDAD>"
- [ ] "## Se desconecta un Query Control. Se finaliza la Query <QUERY_ID> con prioridad <PRIORIDAD>. Nivel multiprocesamiento <CANTIDAD>"
- [ ] "## Se desconecta el Worker <WORKER_ID> - Se finaliza la Query <QUERY_ID> - Cantidad total de Workers: <CANTIDAD>"
- [ ] "## Se envía la Query <QUERY_ID> (<PRIORIDAD>) al Worker <WORKER_ID>"
- [ ] "## Se desaloja la Query <QUERY_ID> (<PRIORIDAD>) del Worker <WORKER_ID> - Motivo: <DESCONEXION / PRIORIDAD>"
- [ ] "##<QUERY_ID> Cambio de prioridad: <PRIORIDAD_ANTERIOR> - <PRIORIDAD_NUEVA>"
- [ ] "## Se terminó la Query <QUERY_ID> en el Worker <WORKER_ID>"
- [ ] "## Se envía un mensaje de lectura de la Query <QUERY_ID> en el Worker <WORKER_ID> al Query Control"

### Desconexión de Query Control
- [ ] Si la Query está en READY, debe enviarse a EXIT directamente
- [ ] Si la Query está en EXEC, debe notificarse al Worker que debe desalojar
- [ ] Una vez recibido el contexto, se envía la Query a EXIT

## 📋 Módulo Worker

### Funcionalidades
- [ ] Interpreta correctamente las instrucciones definidas
- [ ] Realiza las traducciones de direcciones lógicas a físicas sin presentar errores
- [ ] Se respetan los retardos de accesos a memoria
- [ ] Se respeta el algoritmo LRU al momento de reemplazar las páginas
- [ ] Se respeta el algoritmo CLOCK-M al momento de reemplazar las páginas
- [ ] Es capaz de recibir correctamente las interrupciones del Kernel

### Logs Obligatorios
- [ ] "## Query <QUERY_ID>: Se recibe la Query. El path de operaciones es: <PATH_QUERY>"
- [ ] "## Query <QUERY_ID>: FETCH - Program Counter: <PROGRAM_COUNTER> - <INSTRUCCIÓN>"
- [ ] "## Query <QUERY_ID>: - Instrucción realizada: <INSTRUCCIÓN>"
- [ ] "## Query <QUERY_ID>: Desalojada por pedido del Master"
- [ ] "Query <QUERY_ID>: Acción: <LEER / ESCRIBIR> - Dirección Física: <DIRECCION_FISICA> - Valor: <VALOR LEIDO / ESCRITO>"
- [ ] "Query <QUERY_ID>: Se asigna el Marco: <NUMERO_MARCO> a la Página: <NUMERO_PAGINA> perteneciente al - File: <FILE> - Tag: <TAG>"
- [ ] "Query <QUERY_ID>: Se libera el Marco: <NUMERO_MARCO> perteneciente al - File: <FILE> - Tag: <TAG>"
- [ ] "## Query <QUERY_ID>: Se reemplaza la página <File1:Tag1>/<NUM_PAG1> por la <File2:Tag2><NUM_PAG2>"
- [ ] "Query <QUERY_ID>: - Memoria Miss - File: <FILE> - Tag: <TAG> - Pagina: <NUMERO_PAGINA>"
- [ ] "Query <QUERY_ID>: - Memoria Add - File: <FILE> - Tag: <TAG> - Pagina: <NUMERO_PAGINA> - Marco: <NUMERO_MARCO>"

## 📋 Módulo Storage

### Funcionalidades
- [ ] Respeta la estructura definida
- [ ] Actualiza correctamente las estructuras administrativas
- [ ] Mantiene el estado del FS luego de un reinicio
- [ ] Aplica correctamente el FRESH_START
- [ ] Respeta los errores definidos y los informa de manera correcta
- [ ] Calcula correctamente los hash

### Logs Obligatorios
- [ ] "##Se conecta el Worker <WORKER_ID> - Cantidad de Workers: <CANTIDAD>"
- [ ] "##Se desconecta el Worker <WORKER_ID> - Cantidad de Workers: <CANTIDAD>"
- [ ] "##<QUERY_ID> - File Creado <NOMBRE_FILE>:<TAG>"
- [ ] "##<QUERY_ID> - File Truncado <NOMBRE_FILE>:<TAG> - Tamaño: <TAMAÑO>"
- [ ] "##<QUERY_ID> - Tag creado <NOMBRE_FILE>:<TAG>"
- [ ] "##<QUERY_ID> - Commit de File:Tag <NOMBRE_FILE>:<TAG>"
- [ ] "##<QUERY_ID> - Tag Eliminado <NOMBRE_FILE>:<TAG>"
- [ ] "##<QUERY_ID> - Bloque Lógico Leído <NOMBRE_FILE>:<TAG> - Número de Bloque: <BLOQUE>"
- [ ] "##<QUERY_ID> - Bloque Lógico Escrito <NOMBRE_FILE>:<TAG> - Número de Bloque: <BLOQUE>"
- [ ] "##<QUERY_ID> - Bloque Físico Reservado - Número de Bloque: <BLOQUE>"
- [ ] "##<QUERY_ID> - Bloque Físico Liberado - Número de Bloque: <BLOQUE>"
- [ ] "##<QUERY_ID> - <NOMBRE_FILE>:<TAG> Se agregó el hard link del bloque lógico <BLOQUE_LOGICO> al bloque físico <BLOQUE_FISICO>"
- [ ] "##<QUERY_ID> - <NOMBRE_FILE>:<TAG> Se eliminó el hard link del bloque lógico <BLOQUE_LOGICO> al bloque físico <BLOQUE_FISICO>"
- [ ] "##<QUERY_ID> - <NOMBRE_FILE>:<TAG> Bloque Lógico <BLOQUE> se reasigna de <BLOQUE_FISICO_ACTUAL> a <BLOQUE_FISICO_CONFIRMADO>"

## 📋 Módulo Query Control

### Logs Obligatorios
- [ ] "## Conexión al Master exitosa. IP: <ip>, Puerto: <puerto>"
- [ ] "## Solicitud de ejecución de Query: <archivo_query>, prioridad: <prioridad>"
- [ ] "## Lectura realizada: File <File:Tag>, contenido: <CONTENIDO>"
- [ ] "## Query Finalizada - <MOTIVO>"

## 🧪 Pruebas Específicas

### Prueba 1: Planificación
- [ ] FIFO funciona correctamente
- [ ] PRIORIDADES funciona correctamente
- [ ] Aging funciona y produce desalojos cuando corresponde
- [ ] Configuraciones correctas según documento

### Prueba 2: Memoria Worker
- [ ] LRU funciona correctamente
- [ ] CLOCK-M funciona correctamente
- [ ] Los reemplazos se dan de acuerdo al algoritmo elegido
- [ ] Configuraciones correctas según documento

### Prueba 3: Errores
- [ ] ESCRITURA_ARCHIVO_COMMITED: Error correcto
- [ ] FILE_EXISTENTE: Error correcto
- [ ] LECTURA_FUERA_DEL_LIMITE: Error correcto
- [ ] TAG_EXISTENTE: Error correcto
- [ ] Las queries finalizan con los errores correspondientes

### Prueba 4: Storage
- [ ] Se puede observar correctamente el uso de los bloques
- [ ] La deduplicación funciona al momento de realizar el commit
- [ ] Configuraciones correctas según documento

### Prueba 5: Estabilidad General
- [ ] No se observan esperas activas
- [ ] No hay memory leaks
- [ ] El sistema no finaliza de manera abrupta
- [ ] Funciona con múltiples workers conectándose/desconectándose
- [ ] Configuraciones correctas según documento

## 📝 Archivos de Configuración Necesarios

### Para Prueba Planificación
- [ ] master.config (FIFO, luego PRIORIDADES)
- [ ] worker1.config y worker2.config
- [ ] storage.config
- [ ] superblock.config

### Para Prueba Memoria Worker
- [ ] master.config
- [ ] worker.config (CLOCK-M, luego LRU)
- [ ] storage.config
- [ ] superblock.config

### Para Prueba Errores
- [ ] master.config
- [ ] worker.config
- [ ] storage.config
- [ ] superblock.config

### Para Prueba Storage
- [ ] master.config
- [ ] worker.config
- [ ] storage.config
- [ ] superblock.config

### Para Prueba Estabilidad General
- [ ] master.config
- [ ] worker1.config a worker6.config
- [ ] storage.config
- [ ] superblock.config

## ⚠️ Puntos Críticos a Verificar

1. **Desconexión de Query Control**: Debe cancelar la query inmediatamente
   - Si está en READY → EXIT
   - Si está en EXEC → Desalojar del Worker

2. **Aging**: Debe reducir la prioridad cada TIEMPO_AGING milisegundos
   - Solo para queries en READY
   - Debe producir desalojos cuando la prioridad aumenta

3. **Algoritmos de Reemplazo**:
   - LRU: Usa last_used_ms, elige la página menos usada recientemente
   - CLOCK-M: Busca (U=0, M=0), luego (U=0, M=1), luego vuelve a buscar (U=0, M=0), luego (U=0, M=1)

4. **Deduplicación en Storage**: Al hacer COMMIT, debe buscar bloques con mismo hash y reasignar

5. **Logs**: Todos deben tener el formato exacto con "##" al inicio según la consigna

