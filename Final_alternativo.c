#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h> 

#define PAGE_SIZE 4096            // Tamaño de página en bytes (4 KiB)
#define TLB_ENTRIES 5             // Número máximo de entradas en el TLB
#define VIRTUAL_ADDRESS_SPACE 32  // Espacio de direcciones virtuales (32 bits)
#define BINARY_SIZE 20        // Tamaño máximo para el binario de la página
#define OFFSET_BINARY_SIZE 12   // Tamaño máximo para el binario del desplazamiento

// Función para convertir un número a una cadena binaria usando operadores bitwise
uint32_t convert_to_binary_and_int(uint32_t num, int bits, char *output) {
    // Inicializa la cadena de salida con ceros
    for (int i = 0; i < bits; i++) {
        output[i] = (num & (1 << (bits - 1 - i))) ? '1' : '0';
    }
    output[bits] = '\0'; // Agrega el terminador de cadena
}

// Función para convertir una cadena binaria a decimal
uint32_t binary_to_decimal(const char *binary_str) {
    uint32_t decimal_value = 0;
    int length = strlen(binary_str);

    for (int i = 0; i < length; i++) {
        if (binary_str[length - 1 - i] == '1') {
            decimal_value += (1 << i);
        }
    }

    return decimal_value;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

// Función para inicializar el TLB en el heap
    uint32_t* init_tlb() {
    uint32_t *tlb = ( uint32_t*)malloc(TLB_ENTRIES * 4 * sizeof(uint32_t));
    if (tlb == NULL) {
        perror("Error al asignar memoria para el TLB");
        exit(EXIT_FAILURE); // Salir si no se puede asignar memoria
    }

    return tlb;
}

// Función para acceder a la entrada del TLB
uint32_t* get_tlb_entry(uint32_t *tlb, int index) {
    return &tlb[index * 4]; // Cada entrada tiene 4 valores (memory_address, page_number, offset, valid)
}

// Función para buscar en el TLB (simulación de TLB Hit/Miss)
bool search_tlb(uint32_t *tlb, uint32_t memory_address) {
    for (int i = 0; i < TLB_ENTRIES; i++) {
        uint32_t *entry = get_tlb_entry(tlb, i);
        if (entry[3] == 1 && entry[0] == memory_address) { // Valid and address match
            return true; // TLB hit
        }
    }
    return false; // TLB miss
}

static int next_entry = 0; // Índice para el reemplazo FIFO

// Función para agregar una página al TLB (reemplazo FIFO simple)
void add_to_tlb(uint32_t *tlb, uint32_t memory_address, uint32_t page_number, uint32_t offset) {

    uint32_t *current_entry = get_tlb_entry(tlb, next_entry);
    // Llenar la entrada del TLB
    current_entry[0] = memory_address;
    current_entry[1] = page_number;
    current_entry[2] = offset;
    current_entry[3] = 1; // Marca la entrada como válida
}

// Mostrar dirección base de la entrada que será reemplazada (si es válida)
unsigned int direccion_reemplazo(uint32_t *tlb,int next_entry){
    uint32_t *current_entry = get_tlb_entry(tlb, next_entry);
    if (current_entry[3] == 1) {
       return (unsigned int)((uintptr_t)tlb + 4 * next_entry);
    } else {
        return 0x0;
    }

}

// Función para mostrar los datos binarios almacenados en el TLB
void display_tlb_entry(uint32_t *entry) {

    printf("Página: %d\n", entry[1]);
    printf("Desplazamiento: %d\n", entry[2]);
    //printf("Valid: %d\n", entry[3]);

}

// Función para imprimir el TLB completo
void print_tlb(uint32_t *tlb) {
    printf("Contenido del TLB:\n");
    for (int i = 0; i < TLB_ENTRIES; i++) {
        printf("Entrada %d:\n", i);
        display_tlb_entry(get_tlb_entry(tlb, i));
        printf("\n");
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
int main() {
    uint32_t virtual_address;
    char input[20];
    struct timespec start, end;

    // Inicializar el TLB en el heap
    uint32_t *tlb = init_tlb();

    while (1) {
        // Leer dirección virtual del usuario
        printf("Ingrese dirección virtual: ");
        if (fgets(input, 20, stdin) == NULL) {
            perror("Error al leer entrada");
        }

        // Salir del ciclo si el usuario presiona 's'
        if (input[0] == 's'){ 
            printf("Good bye!\n");
            printf("\n");
            break;
        }

        virtual_address = atoi(input); // Convertir la entrada a número entero

        // Iniciar medición de tiempo
        clock_gettime(CLOCK_MONOTONIC, &start);

        printf("TLB desde %p hasta %p\n",(void*)tlb,(void*)(tlb + TLB_ENTRIES * 4));

        // Calcular el número de página y el desplazamiento dentro de la página
        uint32_t page_number = virtual_address / PAGE_SIZE;
        uint32_t offset = virtual_address % PAGE_SIZE;

        // Convertir número de página y desplazamiento a binario
        char page_number_binary[BINARY_SIZE];
        char offset_binary[OFFSET_BINARY_SIZE];

        convert_to_binary_and_int(page_number, BINARY_SIZE, page_number_binary);
        convert_to_binary_and_int(offset, OFFSET_BINARY_SIZE, offset_binary);

        unsigned int politica = direccion_reemplazo(tlb,next_entry);

        // Buscar en el TLB si hay un hit o un miss
        bool hit = search_tlb(tlb, virtual_address);
        if (hit) {
            printf("TLB Hit\n");
        } else {
            printf("TLB Miss.\n");
            add_to_tlb(tlb, virtual_address, page_number, offset);// Agregar al TLB en el heap
        }

        // Mostrar la entrada del TLB (hit o miss)
        bool found = false;
        for (int i = 0; i < TLB_ENTRIES; i++) {
            uint32_t *entry = get_tlb_entry(tlb, i);
            if (entry[0] == virtual_address) {
                display_tlb_entry(entry);
                found = true;
                break;
            }
        }
        // Reemplazo FIFO: mover el índice a la siguiente posición
        next_entry = (next_entry + 1) % TLB_ENTRIES;

        printf("Página en binario: %s\n", page_number_binary);
        printf("Desplazamiento en binario: %s\n", offset_binary);

        printf("Politica de reemplazo: 0x%x\n", politica);

        // Finalizar medición de tiempo
        clock_gettime(CLOCK_MONOTONIC, &end);
        double time_spent = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        printf("Tiempo: %f segundos\n", time_spent);

        printf("\n");

    }

    // Liberar la memoria dinámica del TLB (en el heap)
    free(tlb);

    return 0;
}
