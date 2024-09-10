#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

// Definición de constantes sin usar #define
const uint32_t NUM_PAGES = (1 << 20); // 2^20 páginas para 32 bits con 4 KiB por página
const int TLB_SIZE = 4; // Tamaño máximo de la TLB (ajustable)

// Declaración de la tabla de páginas sin usar arreglos ni estructuras
uint32_t *page_table_ppn; // Simulación de la tabla de páginas usando punteros para PPNs
int *page_table_valid;    // Simulación de bits de validez de la tabla de páginas

// Punteros para manejar la lista enlazada de la TLB
void *tlb_head = NULL; // Puntero a la cabeza de la TLB
void *tlb_tail = NULL; // Puntero a la cola de la TLB
int tlb_count = 0;     // Contador de entradas en la TLB

// Función para imprimir un número en formato binario
void print_binary(uint32_t number) {
    for (int i = 31; i >= 0; i--) {
        printf("%d", (number >> i) & 1);
        if (i % 4 == 0) {
            printf(" "); // Espacio cada 4 bits para mayor claridad
        }
    }
}

// Función para buscar en la TLB
uint32_t search_tlb(uint32_t vpn) {
    void *current = tlb_head;
    while (current != NULL) {
        uint32_t *current_vpn = (uint32_t *)current;         // VPN almacenado en el nodo
        uint32_t *current_ppn = (uint32_t *)(current + 4);   // PPN almacenado en el nodo
        void **next = (void **)(current + 8);                // Puntero al siguiente nodo

        if (*current_vpn == vpn) {
            return *current_ppn; // Retornar PPN si se encuentra la entrada
        }
        current = *next;
    }
    return 0xFFFFFFFF; // Retornar error si no se encuentra la entrada
}

// Función para agregar una entrada a la TLB (FIFO)
void add_to_tlb(uint32_t vpn, uint32_t ppn) {
    // Crear un nuevo nodo en el heap (12 bytes: VPN, PPN, puntero al siguiente)
    void *new_node = malloc(12);
    *(uint32_t *)new_node = vpn;             // Almacenar VPN en el nodo
    *(uint32_t *)(new_node + 4) = ppn;       // Almacenar PPN en el nodo
    *(void **)(new_node + 8) = NULL;         // Siguiente nodo es NULL

    // Si la TLB está vacía
    if (tlb_head == NULL) {
        tlb_head = new_node;
        tlb_tail = new_node;
    } else {
        // Agregar el nuevo nodo al final
        *(void **)(tlb_tail + 8) = new_node;
        tlb_tail = new_node;
    }

    tlb_count++;

    // Si la TLB está llena, eliminar la entrada más antigua (FIFO)
    if (tlb_count > TLB_SIZE) {
        void *old_node = tlb_head;
        tlb_head = *(void **)(tlb_head + 8);
        free(old_node);
        tlb_count--;
    }
}

// Función para traducir una dirección virtual a física usando TLB
uint32_t translate_address(uint32_t virtual_address) {
    uint32_t vpn = virtual_address >> 12;  // Los primeros 20 bits (VPN)
    uint32_t offset = virtual_address & 0xFFF; // Los últimos 12 bits (desplazamiento)

    // Buscar en la TLB primero
    uint32_t ppn = search_tlb(vpn);
    if (ppn != 0xFFFFFFFF) {
        // Entrada encontrada en la TLB
        printf("TLB hit: Página virtual (dec: %u, bin: ", vpn);
        print_binary(vpn);
        printf(") -> Página física %u\n", ppn);
        return (ppn << 12) | offset;
    }

    // Entrada no encontrada en la TLB, buscar en la tabla de páginas
    if (page_table_valid[vpn]) {
        ppn = page_table_ppn[vpn];
        add_to_tlb(vpn, ppn); // Agregar la nueva entrada a la TLB
        printf("TLB miss: Página virtual (dec: %u, bin: ", vpn);
        print_binary(vpn);
        printf(") -> Página física %u\n", ppn);
        return (ppn << 12) | offset;
    } else {
        printf("Error: Dirección virtual no válida en la página (dec: %u, bin: ", vpn);
        print_binary(vpn);
        printf(")\n");
        return 0xFFFFFFFF; // Retorna un valor de error
    }
}

int main() {
    // Inicialización de la tabla de páginas
    page_table_ppn = (uint32_t *)malloc(NUM_PAGES * sizeof(uint32_t));
    page_table_valid = (int *)malloc(NUM_PAGES * sizeof(int));

    // Configurar algunos valores en la tabla de páginas para la simulación
    page_table_ppn[0] = 0x12345;    // Ejemplo de PPN
    page_table_valid[0] = 1;        // Entrada válida

    char input[20];  // Buffer para leer las direcciones y el carácter de salida
    uint32_t virtual_address;

    // Bucle que sigue hasta que el usuario ingrese 's'
    while (1) {
        printf("Ingrese una dirección virtual en decimal (o 's' para salir): ");
        scanf("%s", input);  // Leer la entrada como string

        // Verificar si la entrada es 's' (case insensitive)
        if (tolower(input[0]) == 's' && input[1] == '\0') {
            printf("Good bye!");
            break;  // Salir del bucle si se ingresa 's'
        }

        // Convertir la entrada a un número decimal
        virtual_address = (uint32_t)strtoul(input, NULL, 10);

        // Traducir la dirección y mostrar el resultado
        uint32_t physical_address = translate_address(virtual_address);
        if (physical_address != 0xFFFFFFFF) {
            printf("Dirección virtual: %u -> Dirección física: 0x%08X\n", virtual_address, physical_address);
        }
    }

    // Liberar memoria asignada
    free(page_table_ppn);
    free(page_table_valid);

    // Liberar nodos de la TLB
    void *current = tlb_head;
    while (current != NULL) {
        void *next = *(void **)(current + 8);
        free(current);
        current = next;
    }

    printf("Programa terminado.\n");
    return 0;
}
