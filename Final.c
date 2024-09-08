#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define TLB_SIZE 5
#define PAGE_SIZE 4096 // 4 KiB

typedef struct {
    uint32_t virtual_address;
    uint32_t page_number;
    uint32_t offset;
    char *page_binary;
    char *offset_binary;
} TLBEntry;

// Global TLB to store up to 5 entries
TLBEntry *tlb[TLB_SIZE];
int tlb_start = 0; // FIFO index to replace

// Function to convert a decimal number to a binary string
char *decimal_to_binary(uint32_t num, int bits) {
    char *binary = (char *)malloc(bits + 1);
    binary[bits] = '\0';
    for (int i = bits - 1; i >= 0; i--) {
        binary[i] = (num & 1) ? '1' : '0';
        num >>= 1;
    }
    return binary;
}

// Function to convert a binary string to a decimal number
uint32_t binary_to_decimal(const char *binary) {
    uint32_t decimal = 0;
    int position = 0;
    for (int i = strlen(binary) - 1; i >= 0; i--, position++) {
        if (binary[i] == '1') {
            decimal += 1 << position;
        }
    }
    return decimal;
}

// Function to get the current time in seconds
double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

// Function to search for an address in the TLB
int search_tlb(uint32_t address) {
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i] != NULL && tlb[i]->virtual_address == address) {
            return i; // TLB Hit
        }
    }
    return -1; // TLB Miss
}

// Function to replace an entry in the TLB using FIFO policy
void replace_tlb(TLBEntry *entry) {
    // Check if the TLB is empty
    if (tlb[tlb_start] != NULL) {
        // Show the base address of the memory being replaced
        printf("Politica de reemplazo: 0x%08X\n", (unsigned int)tlb[tlb_start]->virtual_address);
        // Free the memory of the replaced entry
        free(tlb[tlb_start]->page_binary);
        free(tlb[tlb_start]->offset_binary);
        free(tlb[tlb_start]);
    } else {
        printf("Politica de reemplazo: 0x0\n");
    }

    // Replace the entry in TLB
    tlb[tlb_start] = entry;
    tlb_start = (tlb_start + 1) % TLB_SIZE; // Move the FIFO index
}

int main() {
    uint32_t address;
    char input[20];
    double start_time, end_time;

    // Show TLB memory range
    printf("TLB desde 0x%08X hasta 0x%08X\n", (unsigned int)tlb, (unsigned int)tlb + sizeof(tlb));

    while (1) {
        printf("Ingrese dirección virtual: ");
        fgets(input, 20, stdin);
        if (input[0] == 's' || input[0] == 'S') {
            printf("Good bye!\n");
            break;
        }

        address = strtoul(input, NULL, 10);
        uint32_t page_number = address / PAGE_SIZE;
        uint32_t offset = address % PAGE_SIZE;

        // Start the time measurement
        start_time = get_time();

        // Search in TLB
        int index = search_tlb(address);
        if (index != -1) {
            printf("TLB Hit\n");
        } else {
            printf("TLB Miss\n");

            // Create a new TLB entry
            TLBEntry *new_entry = (TLBEntry *)malloc(sizeof(TLBEntry));
            new_entry->virtual_address = address;
            new_entry->page_number = page_number;
            new_entry->offset = offset;
            new_entry->page_binary = decimal_to_binary(page_number, 20); // Page number binary representation
            new_entry->offset_binary = decimal_to_binary(offset, 12);   // Offset binary representation

            // Replace the entry in TLB
            replace_tlb(new_entry);
        }

        // Show page and offset in decimal and binary
        printf("Página: %u\n", page_number);
        printf("Desplazamiento: %u\n", offset);
        printf("Página en binario: %s\n", decimal_to_binary(page_number, 20));
        printf("Desplazamiento en binario: %s\n", decimal_to_binary(offset, 12));

        // End the time measurement
        end_time = get_time();

        // Show the time spent
        printf("Tiempo: %.6f segundos\n", end_time - start_time);
    }

    // Free all TLB entries
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i] != NULL) {
            free(tlb[i]->page_binary);
            free(tlb[i]->offset_binary);
            free(tlb[i]);
        }
    }

    return 0;
}
