#!/bin/bash

# Directorio donde está este script (root del repo)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Directorio donde está el binario de query_control
QC_DIR="$SCRIPT_DIR/query_control/bin"

# Nos movemos a query_control/bin
cd "$QC_DIR" || { echo "No pude hacer cd a $QC_DIR"; exit 1; }

# Y el ejecutable debe ser ./query_control
echo "Ejecutando AGING tests desde $(pwd)"

./query_control query AGING_1 4 &
./query_control query AGING_2 3 &
./query_control query AGING_3 5 &
./query_control query AGING_4 1 &

wait

echo "Finalizaron todas las queries."
