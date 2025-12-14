#!/bin/bash

# Directorio donde está este script (root del repo)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Directorio donde está el binario de query_control
QC_DIR="$SCRIPT_DIR/query_control/bin"

# Nos movemos a query_control/bin
cd "$QC_DIR" || { echo "No pude hacer cd a $QC_DIR"; exit 1; }

# Y el ejecutable debe ser ./query_control
echo "Ejecutando AGING tests desde $(pwd)"
echo "Asegúrate de que Master, Storage y Workers estén corriendo antes de ejecutar este script"

# Ejecutar queries con un pequeño delay entre cada una para evitar problemas de sincronización
# El delay de 0.1 segundos es suficiente para que el Master procese cada conexión
./query_control query AGING_1 4 &
sleep 0.1

./query_control query AGING_2 3 &
sleep 0.1

./query_control query AGING_3 5 &
sleep 0.1

./query_control query AGING_4 1 &

# Esperar a que todas las queries terminen
wait

echo "Finalizaron todas las queries."
