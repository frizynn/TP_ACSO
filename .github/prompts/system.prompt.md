
### Objetivo General
- Desarrollar en C un simulador de CPU que ejecute instrucciones del conjunto ARMv8.
- Simular la ejecución de instrucciones en lenguaje ensamblador a partir de su versión en hexadecimal.

### Estructura del Trabajo
- **División del Programa:**
  - **Shell:** Proporciona la interfaz de usuario con comandos para controlar la ejecución del simulador.
  - **Simulador:** Implementa la lógica de lectura, decodificación y ejecución de cada instrucción.

### Archivos y Directorios Importantes
- **/src:**
  - **shell.c y shell.h:** Implementan el shell (no deben modificarse).
  - **sim.c:** Archivo donde debes implementar la función `process_instruction()` y toda la lógica del simulador.
  - **Makefile:** Para compilar el proyecto (puedes modificarlo si agregas nuevos archivos .c).
- **/inputs:** Contiene programas en assembly ARMv8 para testear el simulador.
- **/ref:** Manual de referencia del ISA ARMv8 (útil para validar la implementación).
- **Repositorio:** Se debe entregar el enlace al repositorio con la estructura correcta (directorio src en la raíz).

### Funcionalidades del Shell
El shell provee los siguientes comandos:
- **go:** Ejecuta el programa completo hasta encontrar la instrucción de halt.
- **run <n>:** Ejecuta _n_ instrucciones.
- **mdump <low> <high>:** Muestra el contenido de la memoria entre las direcciones especificadas y lo guarda en un archivo de dump.
- **rdump:** Muestra el estado actual del CPU (registros X0-X31, flags N y Z, PC y cantidad de instrucciones ejecutadas).
- **input reg_num reg_val:** Permite ingresar un valor a un registro específico.
- **?:** Muestra la lista de comandos disponibles.
- **quit:** Sale del shell.

### Estado del CPU y Memoria
- **Estructura del CPU (definida en shell.h):**
  - **PC:** Contador de programa.
  - **REGS[32]:** Registro general.
  - **Flags:** Sólo se implementan N (negativo) y Z (cero); C (carry) y V (overflow) siempre serán 0.
- **Memoria:**
  - Inicia en la dirección `0x10000000` y tiene un tamaño de `0x00100000`.
  - Es **little-endian** (el byte menos significativo se almacena en la dirección más baja).
  - Para acceder a la memoria se deben usar las funciones:
    - `mem_read_32(uint64_t address)`
    - `mem_write_32(uint64_t address, uint32_t value)`

### Tareas Principales en la Implementación
- **Implementar `process_instruction()` en sim.c:**
  - **Decode Stage:** Leer y decodificar la instrucción (determinar el tipo de instrucción y extraer sus campos).
  - **Execute Stage:** Ejecutar la instrucción actualizando el estado del CPU (registros, flags, PC, memoria).
- **Organización del Código:**
  - Se recomienda definir estructuras y funciones auxiliares para organizar el decode y la ejecución de instrucciones.

### Instrucciones a Implementar (sólo la variante de 64 bits)
1. **ADDS:**
   - **Immediate:** Ej. `adds X0, X1, 3` (considerar shift: si es 01, desplazar imm12 12 bits a la izquierda).
   - **Extended Register:** Ej. `adds X0, X1, X2`.
2. **SUBS:**
   - **Immediate:** Ej. `subs X0, X1, 3` (manejo similar del shift).
   - **Extended Register:** Ej. `subs X0, X1, X2`.
3. **HLT:**  
   - Ejemplo: `hlt 0` para detener la simulación (simplemente setear la variable global `RUN_BIT` a 0).
4. **CMP:**
   - **Extended Register:** Ej. `cmp X13, X14` (calcular la resta, actualizar flags, sin almacenar el resultado).
   - **Immediate:** Ej. `cmp X13, 4`.
5. **Operaciones Lógicas y Aritméticas:**
   - **ANDS:** `ands X0, X1, X2` (bit a bit AND y actualización de flags).
   - **EOR:** `eor X0, X1, X2` (bit a bit XOR).
   - **ORR:** `orr X0, X1, X2` (bit a bit OR).
6. **Saltos:**
   - **B (Branch):** Salto incondicional calculado de forma relativa (usar el immediate de 28 bits).
   - **BR (Branch Register):** Salto a la dirección contenida en un registro.
   - **B.Cond (Branch Condicional):** Ej. `beq`, `bne`, `bgt`, `blt`, `bge`, `ble` que dependen de los flags N y Z.
7. **Shift:**
   - **LSL (Immediate):** Ej. `lsl X4, X3, 4` (shift lógico a la izquierda).
   - **LSR (Immediate):** Ej. `lsr X4, X3, 4` (shift lógico a la derecha).
8. **Operaciones de Memoria:**
   - **STUR:** Guardar un registro en memoria.
   - **STURB:** Guardar el byte menos significativo de un registro.
   - **STURH:** Guardar los 16 bits menos significativos.
   - **LDUR:** Cargar de memoria.
   - **LDURH:** Cargar 16 bits (completar con ceros a 64 bits).
   - **LDURB:** Cargar 8 bits (completar con ceros a 64 bits).
9. **Otras Instrucciones:**
   - **MOVZ:** Ej. `movz X1, 10` (solo implementar cuando el shift es cero).
   - **ADD:** 
     - **Immediate:** Ej. `add X0, X1, 3` (manejar shift 00 y 01).
     - **Extended Register:** Ej. `add X0, X1, X2`.
   - **MUL:** Ej. `mul X0, X1, X2` (multiplicación).
   - **CBZ / CBNZ:** Branch if zero / if not zero (condicionales basados en el valor de un registro).

### Consideraciones Importantes
- **Tests:** Es crucial escribir tests para cada instrucción. Usa los comandos `run 1`, `rdump` y `mdump` para verificar el estado del CPU y la memoria.
- **Simulador de Referencia:** Tu implementación debe coincidir con el comportamiento del `ref_sim` proporcionado.
- **Flags:** Solo se actualizan N y Z en operaciones aritméticas. C y V se asumen siempre en 0.
- **Registro X31:** Se trata como el registro XZR, que siempre tiene el valor 0 (especialmente relevante para instrucciones como CMP).
- **Memoria Little-Endian:** Presta atención al orden de los bytes en operaciones de carga y almacenamiento.

Con estos puntos tienes un panorama claro de qué hacer, qué funciones utilizar y cuáles son las instrucciones que debes implementar. ¿Necesitas profundizar en algún aspecto en particular?