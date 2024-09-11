#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define TLB_SIZE 5
#define PAGE_SIZE 4096 // 4 KiB
#define BINARY_PAGE_BITS 20
#define BINARY_OFFSET_BITS 12
#define MAX_TLB_BYTES 285

// Punteros para el TLB
uint32_t *tlb_virtual_address;
uint32_t *tlb_page_number;
uint32_t *tlb_offset;
char **tlb_page_binary;
char **tlb_offset_binary;

int tlb_start = 0; // FIFO index to replace


unsigned int decimal_a_binario(int decimal) {
    unsigned int binario = 0;
    int posicion = 0;

    // Procesar cada bit desde el menos significativo hasta el más significativo
    while (decimal > 0) {
        // Obtener el bit menos significativo y colocarlo en la posición correspondiente
        binario |= (decimal & 1) << posicion;
        // Desplazar a la derecha el decimal para procesar el siguiente bit
        decimal >>= 1;
        posicion++;
    }

    return binario;
}


int binario_a_decimal(unsigned int binario) {
    int decimal = 0;
    int posicion = 0;

    // Procesar cada bit desde el menos significativo hasta el más significativo
    while (binario > 0) {
        // Si el bit en la posición actual es 1, sumar 2^posicion al resultado
        if (binario & 1) {
            decimal += (1 << posicion);
        }
        // Desplazar a la derecha el binario para procesar el siguiente bit
        binario >>= 1;
        posicion++;
    }

    return decimal;
}

void gestionar_TLB(uint32_t address) {
    uint32_t page_number = address >> BINARY_OFFSET_BITS;
    uint32_t offset = address & (PAGE_SIZE - 1);
    unsigned int page_binary = decimal_a_binario(page_number);
    unsigned int offset_binary = decimal_a_binario(offset);

    // Comprobar si ya existe la entrada (TLB Hit)
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb_virtual_address[i] == address) {
            printf("TLB Hit\n");
            return;
        }
    }

    // Si no existe, reemplazar la entrada más antigua (FIFO) (TLB Miss)
    printf("TLB Miss\n");
    printf("Reemplazando entrada en %p\n", (void *)&tlb_virtual_address[tlb_start]);

    tlb_virtual_address[tlb_start] = address;
    tlb_page_number[tlb_start] = page_number;
    tlb_offset[tlb_start] = offset;
    snprintf(tlb_page_binary[tlb_start], BINARY_PAGE_BITS + 1, "%020u", page_binary);
    snprintf(tlb_offset_binary[tlb_start], BINARY_OFFSET_BITS + 1, "%012u", offset_binary);

    // Incrementar el índice FIFO (Ciclo en 0 si llega al final del buffer)
    tlb_start = (tlb_start + 1) % TLB_SIZE;
}

int main(int argc, char *argv[]){
    uint32_t address;
    char input[20];
    double start_time, end_time;

    //tan pronto empieza ejecucion, inicializamos el TLB
    // Inicialización de memoria para TLB
    tlb_virtual_address = (uint32_t *)malloc(TLB_SIZE * sizeof(uint32_t));
    tlb_page_number = (uint32_t *)malloc(TLB_SIZE * sizeof(uint32_t));
    tlb_offset = (uint32_t *)malloc(TLB_SIZE * sizeof(uint32_t));
    tlb_page_binary = (char **)malloc(TLB_SIZE * sizeof(char *));
    tlb_offset_binary = (char **)malloc(TLB_SIZE * sizeof(char *));

    for (int i = 0; i < TLB_SIZE; i++) {
        tlb_page_binary[i] = (char *)malloc((BINARY_PAGE_BITS + 1) * sizeof(char));
        tlb_offset_binary[i] = (char *)malloc((BINARY_OFFSET_BITS + 1) * sizeof(char));
    }



    while (1) {
        printf("Ingrese dirección virtual: ");
        fgets(input, 20, stdin);
        if (input[0] == 's' || input[0] == 'S') {
            printf("Good bye!\n");
            break;
        }

    }
    //liberacion de recursos
    for (int i = 0; i < TLB_SIZE; i++) {
        free(tlb_page_binary[i]);
        free(tlb_offset_binary[i]);
    }
    free(tlb_virtual_address);
    free(tlb_page_number);
    free(tlb_offset);
    free(tlb_page_binary);
    free(tlb_offset_binary);

    return 0;
}