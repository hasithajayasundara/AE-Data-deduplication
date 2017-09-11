#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "headers/SHA1Hashing.h"

#define HASHSIZE 101
#define MAX_CHUNK_SIZE 65536
#define AVERAGE_CHUNK_SIZE 4096
#define E 2.718281828

//global variable list
static int windowSize = AVERAGE_CHUNK_SIZE/(E-1);

static struct nlist *hashtab[HASHSIZE]; // Pointer table

struct nlist {
    struct nlist *next;
    char *name;
    char *defn;
};

/*
 * strdup(): make a duplicate copy of s
 */

static char *mystrdup(char *s) {

    char *p = (char *)malloc(strlen(s) + 1);

    if (p == NULL) {
        printf("Error: ran out of memory");
        exit(EXIT_FAILURE);
    }

    strcpy(p, s);
    return p;
}

/*
 * hash(): Calculates a hash value for a given string
 */

static unsigned hash(char *s) {

    unsigned hashval;

    for (hashval = 0; *s != '\0'; s++)
        hashval = *s + 31 * hashval;

    return hashval % HASHSIZE;
}

/*
 * lookup(): Look for s in the hash table
 */

static struct nlist *lookup(char *s) {

    struct nlist *np;

    for (np = hashtab[hash(s)]; np != NULL; np = np->next)
        if (strcmp(s, np->name) == 0)
            return np;  // Found

    return NULL; // Not found
}

/*
 * install(): Put a value into the hash table
 */

static struct nlist *insert(char *name, char *defn) {

    struct nlist *np;

    if ((np = lookup(name)) == NULL) {

        unsigned hashval;

        if ((np = (struct nlist *)malloc(sizeof(*np))) == NULL) {
            printf("Error: ran out of memory");
            exit(EXIT_FAILURE);
        }
        hashval  = hash(name);
        np->name = mystrdup(name);
        np->next = hashtab[hashval];
        hashtab[hashval] = np;

    } else

        free((void *)np->defn);

    np->defn = mystrdup(defn);

    return np;
}

/*
 * undef(): remove a name and definition from the table
 */

static void undef(char *s) {

    unsigned h;
    struct nlist *prior;
    struct nlist *np;

    prior = NULL;
    h = hash(s);

    for (np = hashtab[h]; np != NULL; np = np->next) {
        if (strcmp(s, np->name) == 0)
            break;
        prior = np;
    }

    if (np != NULL) {
        if (prior == NULL)
            hashtab[h] = np->next;
        else
            prior->next = np->next;

        free((void *)np->name);
        free((void *)np->defn);
        free((void *)np);
    }

}

/**
 * this method chunks the given data stream
 */
int chunkData(unsigned char *buffer, int n) {

    unsigned char *copy;
    unsigned char *max = buffer, *end = buffer + n - 8;
    int i=0;
    for (copy = buffer +1; copy <= end; copy++) {
        int comp_res =comp(copy, max);
        if (comp_res < 0) {
            max = copy;
            continue;
        }
        if (copy == max + windowSize || copy == buffer + MAX_CHUNK_SIZE){ //chunk max size
            return copy - buffer;
        }
        i++;
    }
    return n;
}

/**
 * This method compares the given pointers
 */
int comp(unsigned char *i,unsigned char *max) {

    uint64_t a = __builtin_bswap64(*((uint64_t *) i));
    uint64_t b = __builtin_bswap64(*((uint64_t *) max));

    if (a > b) {
        return 1;
    }
    return -1;
}

unsigned char *file(char *name){
    FILE *fp = fopen ( name , "r" ); //TestDataset
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
    return buffer;
}

int main()
{
    //init sha1
    SHA1_CTX ctx;
    BYTE buf[SHA1_BLOCK_SIZE];

    //init hash map search
    struct nlist *np;

    //Get the file
    unsigned char *buffer1 = file("DARPA99Week1-1");//DARPA99Week1-1
    printf("Length and size of first data stream -%d %.2fMB\n",strlen(buffer1),strlen(buffer1)/1048576.0);
    unsigned char *buffer2 = file("DARPA99Week1-2");//DARPA99Week1-2
    printf("Length and size of second data stream -%d %.2fMB\n",strlen(buffer2),strlen(buffer2)/1048576.0);

    while(1){
        int length = strlen(buffer1);
        if(length < (windowSize + 8)){
            sha1Init(&ctx);
            sha1Update(&ctx,buffer1,length);
            sha1Final(&ctx, buf);
            (void)insert(buf,buffer1);
            break;
        }
        int boundary = chunkData(buffer1, length);
        sha1Init(&ctx);
        sha1Update(&ctx,buffer1,boundary+1);
        sha1Final(&ctx, buf);
        char subsStr[boundary+1];
        for(int j = 0; j < boundary + 1; j++){
            subsStr[j] = buffer1[j];
        }
        (void)insert(buf,subsStr);
        if(boundary == length){
            break;
        }
        buffer1 = buffer1 + boundary + 1;
    }
    int duplicateContent = 0;
    int count = 0;
    while(1){
        int length = strlen(buffer2);
        if(length < (windowSize + 8)){
            sha1Init(&ctx);
            sha1Update(&ctx,buffer2,length);
            sha1Final(&ctx, buf);
            np = lookup(buf);
            if(np != NULL){
                count++;
                duplicateContent += strlen(np->defn);
            }
            break;
        }
        int boundary = chunkData(buffer2, length);
        sha1Init(&ctx);
        sha1Update(&ctx,buffer2,boundary+1);
        sha1Final(&ctx, buf);
        char subsStr[boundary+1];
        for(int j = 0; j < boundary + 1; j++){
            subsStr[j] = buffer2[j];
        }
        np = lookup(buf);
        if(np != NULL){
            count++;
            duplicateContent += strlen(np->defn);
        }
        if(boundary == length){
            break;
        }
        buffer2 = buffer2 + boundary + 1;
    }
    printf("Found %d duplicate contents.\n",count);
    printf("Length of the duplicate content - %d.\n",duplicateContent);
    printf("Size of the duplicate content - %d bytes.\n",duplicateContent);
    printf("Total number of MB reduced %.2f.\n",(duplicateContent - count*20.0)/1048576.0);
    printf("Duplicates as a percentage %.2f.",(duplicateContent/strlen(buffer2))*100);
    return 0;
}