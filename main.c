#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "sha1.h"

//global variable list
static int window_size = 8192/1.718281828; //chunk_avg_size

/**
 * @brief this method chunks the given data stream
 * @param p the data stream to be chunked
 * @param n length of the data stream
 * @return chunk boundaries
 */
int chunk_data(unsigned char *buffer, int n) {

    unsigned char *copy;
    unsigned char *max = buffer, *end = buffer+n-8;

    for (copy = buffer +1; copy <= end; copy++) {
        int comp_res =comp(copy, max);
        if (comp_res < 0) {
            max = copy;
            continue;
        }
        if (copy == max + window_size || copy == buffer + 65536){
            return copy - buffer;
        }
    }
    return n;
}

/**
 * @brief This method compares the given pointers
 * @param i position of the current pointer
 * @param max position of the max pointer
 * @return -1/+1 after comparing the pointers
 */
int comp(unsigned char *i,unsigned char *max){
    uint64_t a = __builtin_bswap64(*((uint64_t *) i));
    uint64_t b = __builtin_bswap64(*((uint64_t *) max));
    if (a > b) {
        return 1;
    }
    return -1;
}

/**
 * @brief The main method
 * @return 0 for successful completion of programme
 */
int main()
{
    SHA1Context sha;
    int i;
    FILE *fp = fopen ( "TestDataset" , "r" ); //TestDataset
    fseek( fp , 0L , SEEK_END);
    long lSize = ftell( fp );
    rewind( fp );
    unsigned char *buffer = calloc( 1, lSize+1 );
    if( !buffer ) {
        fclose(fp),fputs("error allocating memory\n",stderr),exit(1);
    }
    if( 1!=fread( buffer , lSize, 1 , fp) ){
        fclose(fp),free(buffer),fputs("error\n",stderr),exit(1);
    }
    int length = 0;
    int boundary = 0;
    while(1){
        length = strlen(buffer);
        if(length < window_size + 8){
            SHA1Reset(&sha);
            SHA1Input(&sha,buffer,length);
            for(i = 0; i < 5 ; i++) {
                printf("%X ", sha.Message_Digest[i]);
            }
            printf("\n");
            break;
        }
        boundary = chunk_data(buffer,length);
        SHA1Reset(&sha);
        SHA1Input(&sha,buffer,boundary+1);
        for(i = 0; i < 5 ; i++) {
            printf("%X ", sha.Message_Digest[i]);
        }
        printf("\n");
        if(boundary == length){
            break;
        }
        buffer = buffer + boundary + 1;
    }
    fclose(fp);
    //free(buffer);
    return 0;
}