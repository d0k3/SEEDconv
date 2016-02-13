#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 0xAC000

void showhelp_exit() {
    printf("usage: SEEDconv [00000000] ...\n");
    printf(" get [00000000] file on N3DS 9.6.0+ from:\n");
    printf(" nand:/data/<console-unique>/sysdata/0001000f/\n\n");
    exit(0);
}

int main( int argc, char** argv )
{
    FILE* fp;
    unsigned char* buffer;
    unsigned char* seeddb;
    
    int n_seeds = 0;
    
    printf("\nSEEDconv by d0k3\n");
    printf("----------------\n");
    
    if(argc < 2) showhelp_exit();
    
    buffer = (unsigned char*) malloc(BUFFER_SIZE);
    seeddb = (unsigned char*) malloc(BUFFER_SIZE);
    if((buffer == NULL) || (seeddb == NULL)) {
        printf("out of memory");
        return 0;
    }
    
    memset(seeddb, 0x00, BUFFER_SIZE);
    while ( --argc > 0 ) {
        size_t size;
        argv++;
        
        printf("\n\"%s\"...\n", *argv);
        fp = fopen(*argv, "rb");
        if (fp == NULL) {
            printf("could not open file!\n\n");
            continue;
        }
        size = fread(buffer, 1, BUFFER_SIZE, fp);
        fclose(fp);
    
        // collect seeds
        if ( size == BUFFER_SIZE ) { // most likely a 00000000 file
            for ( int n = 0; n < 2; n++ ) {
                static const char zeroes[16] = { 0x00 };
                static const int seed_offsets[2] = {0x7000, 0x5C000};
                unsigned char* seed_data = buffer + seed_offsets[n];
                for ( size_t i = 0; i < 2000; i++ ) {
                    static const char magic[4] = { 0x00, 0x00, 0x04, 0x00 };
                    unsigned char* titleId = seed_data + (i*8);
                    unsigned char* seed = seed_data + (2000 * 8) + (i*16);
                    int exid = 0;
                    if ( memcmp(titleId + 4, magic, 4) != 0 ) continue;
                    if (memcmp(seed, zeroes, 16) == 0) // Bravely Second demo seedworkaround
                        seed = buffer + seed_offsets[(n+1)%2] + (2000 * 8) + (i*16);
                    if (memcmp(seed, zeroes, 16) == 0) continue;
                    for (exid = 0; exid < n_seeds; exid++) // duplicate check
                        if ( memcmp(titleId, seeddb + 0x10 + (0x20 * exid), 8) == 0 ) break;
                    if (exid < n_seeds) continue;
                    memcpy(seeddb + 0x10 + (0x20 * n_seeds), titleId, 0x8);
                    memcpy(seeddb + 0x18 + (0x20 * n_seeds), seed, 0x10);
                    printf("SEED found (%i):\n%08X%08X\n%08X%08X\n%08X%08X\n", ++n_seeds,
                        *((unsigned int*) (titleId + 4)), *((unsigned int*) titleId),
                        *((unsigned int*) (seed + 12)), *((unsigned int*) (seed + 8)),
                        *((unsigned int*) (seed + 4)), *((unsigned int*) seed));
                }
            }
        } else { // maybe a seeddb.bin to merge - check first
            static const char zeroes[16] = { 0x00 };
            int n_entries = *(int*) buffer; // relies on your system being big endian
            if ((n_entries != (int) (size - 16) / 32) || (memcmp(buffer + 2, zeroes, 14) != 0)) {
                printf("file has unknown type!\n");
                continue;
            }
            for ( size_t i = 0x10; i < size; i += 0x20 ) {
                unsigned char* titleId = buffer + i + 0x00;
                unsigned char* seed = buffer + i + 0x08;
                int exid = 0;
                if ((memcmp(titleId, zeroes, 8) == 0) || (memcmp(seed, zeroes, 16) == 0))
                    continue; // don't merge zero seeds / titleIds
                for (exid = 0; exid < n_seeds; exid++) // duplicate check
                    if ( memcmp(buffer + i, seeddb + 0x10 + (0x20 * exid), 0x20) == 0 ) break;
                if (exid < n_seeds) continue;
                memcpy(seeddb + 0x10 + (0x20 * n_seeds), buffer + i, 0x20);
                printf("SEED found (%i):\n%08X%08X\n%08X%08X\n%08X%08X\n", ++n_seeds,
                    *((unsigned int*) (titleId + 4)), *((unsigned int*) titleId),
                    *((unsigned int*) (seed + 12)), *((unsigned int*) (seed + 8)),
                    *((unsigned int*) (seed + 4)), *((unsigned int*) seed));
            }
        }
    }
    
    printf("\n----------------\n");
    *((int*) seeddb) = n_seeds;
    if ( n_seeds > 0 ) {
        fp = fopen("seeddb.bin", "wb");
        if(fp == NULL) {
            printf("write seeddb.bin failed!\n\n");
            return 0;
        }
        fwrite(seeddb, 1, 0x10 + (0x20 * n_seeds), fp);
        fclose(fp);
        printf("%i SEEDs found!\n\n", n_seeds);
    } else {
        printf("No SEEDs found!\n\n");
    }    
    
    free(buffer);
    free(seeddb);
    
    return 1;
}
