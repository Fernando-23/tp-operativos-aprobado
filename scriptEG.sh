#!/bin/bash

# Directorio donde está este script (root del repo)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Directorio donde está el binario de query_control
QC_DIR="$SCRIPT_DIR/query_control/bin"

# Ejecutable query_control
QC="$QC_DIR/query_control"

# Verificación rápida
if [ ! -x "$QC" ]; then
    echo "Error: no encuentro ejecutable query_control en: $QC"
    exit 1
fi

echo "Usando query_control en: $QC"
echo "Directorio actual al inicio: $(pwd)"

# Nos movemos a query_control/bin
cd "$QC_DIR" || { echo "No pude hacer cd a $QC_DIR"; exit 1; }

AGING_NAMES=(AGING_1 AGING_2 AGING_3 AGING_4)
PRIORIDAD=20
INSTANCIAS=25

for NAME in "${AGING_NAMES[@]}"; do
    echo ">>> Lanzando $INSTANCIAS instancias de $NAME (prio $PRIORIDAD)"
    
    for ((i=1; i<=INSTANCIAS; i++)); do
        ./query_control query "${NAME}" "$PRIORIDAD" &
    done
done

wait

echo "Finalizaron todas las instancias."
