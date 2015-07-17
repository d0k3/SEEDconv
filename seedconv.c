#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 0xAC000
#define TITLEID_OFFSET 0x7000
#define SEED_OFFSET (TITLEID_OFFSET + (2000*8))

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
        argv++;
        printf("\n\"%s\"...\n", *argv);
        fp = fopen(*argv, "rb");
        if (fp == NULL) {
            printf("could not open file!\n\n");
            continue;
        }
        if (fread(buffer, 1, BUFFER_SIZE, fp) != BUFFER_SIZE) {
            printf("file is too small!\n\n");
            fclose(fp);
            continue;
        };
        fclose(fp);
    
        // collect seeds
        for ( size_t i = 0; i < 2000; i++ ) {
            static const char magic[4] = { 0x00, 0x00, 0x04, 0x00 };
            unsigned char* titleId = buffer + TITLEID_OFFSET + (i*8);
            unsigned char* seed = buffer + SEED_OFFSET + (i*16);
            int exid = 0;
            if ( memcmp(titleId + 4, magic, 4) != 0 ) continue;
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
