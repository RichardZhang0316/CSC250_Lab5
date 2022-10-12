/**
 * The program recovers text (ASCI text) from a WAV file. The program will be executed with command line arguments
 * for the number of LSb (-b), a WAV audio file name, and a text filename (where the extracted text will be 
 * written). If correct command line arguments are provided, then the program will recover the text in the WAV 
 * file. The text from the WAV file will then be written to the plaintext file (overwriting any previous 
 * contents). The program will also print the number of characters encrypted or decrypted and the name of the 
 * resulting file to the screen.
 * 
 * @author Richard Zhang {zhank20@wfu.edu}
 * @date Oct. 11, 2022
 * @assignment Lab 5
 * @course CSC 250
 **/
#include <math.h>  
#include <stdio.h>
#include <stdlib.h>  
#include <string.h> 
#include "getWavArgs.h" /* source files for parsing the command line arguments */

#define MAX_STRING 256 /* define the maximum length of a file name */

/* function reads the RIFF, fmt, and start of the data chunk */
int readWavHeader(FILE* inFile, short *sampleSizePtr, int *numSamplesPtr, int *sampleRatePtr, short *numChannelsPtr);
/* function reads the WAV audio data (last part of the data chunk) and gets the encrypted message */
int readWavData(FILE* inFile, short sampleSize, int numSamples, short numChannels, int numLSB, char textFileName[]);

/**
 * The main function first checks whether the command line entered meets the requirments and then checks if 
 * the WAV file exist or not. Then, it first reads the RIFF and format chunks and also reads and
 * prints the start of the data chunk by calling readWavHeader() function. Then, it calls readWavData() function.
 **/
int main(int argc, char *argv[]) {
    FILE *inFile;      /* WAV file */
    short sampleSize = 0;  /* size of an audio sample (bits) */
    int sampleRate = 0;    /* sample rate (samples/second) */
    int numSamples = 0;    /* number of audio samples */ 
    int wavOK = 0;     /* 1 if the WAV file si ok, 0 otherwise */
    short numChannels = 0; /* number of channels */
    int numLSB;           /* number of LSBs to use for recovering text */
    int argsOK = 0;    /* shows if the input recieved is legit */
    char wavFileName[MAX_STRING];   /* WAV file name */
    char textFileName[MAX_STRING];  /* plaintext file name */
    
    /* check if the input recieved is legit */
    argsOK = getWavArgs(argc, argv, &numLSB, wavFileName, textFileName);
    if (argsOK == 0) {
        printf("Illegal input found. Please enter the correct format of input \n");
        return 1;
    }

    /* check if we can open the WAV file */
    inFile = fopen(wavFileName, "rbe"); 
    if(!inFile) {
        printf("could not open wav file %s \n", wavFileName);
        return 2;
    }

    /* checks if the WAV file has compatible format and reads the RIFF, fmt, and start of the data chunk if so */
    wavOK = readWavHeader(inFile, &sampleSize, &numSamples, &sampleRate, &numChannels);
    if(!wavOK) {
       printf("wav file %s has incompatible format \n", argv[1]);   
       return 3;
    }
    else {
        /* reads the WAV audio data (last part of the data chunk) and gets the encrypted message */
        readWavData(inFile, sampleSize, numSamples, numChannels, numLSB, textFileName);
    }

    if(inFile) {
        /* close the WAV file */
        fclose(inFile);
    }
    return 0;
}


/**
 *  function reads the RIFF, fmt, and start of the data chunk. 
 */
int readWavHeader(FILE* inFile, short *sampleSizePtr, int *numSamplesPtr, int *sampleRatePtr, short *numChannelsPtr) {
    char chunkId[] = "    ";  /* chunk id, note initialize as a C-string */
    char data[] = "    ";      /* chunk data */
    int chunkSize = 0;        /* number of bytes remaining in chunk */
    short audioFormat = 0;    /* audio format type, PCM = 1 */
    short numChannels = 0;    /* number of audio channels */ 
    int sampleRate = 0;       /* Audio samples per second */
    short bitsPerSample = 0; /* Number of bits used for an audio sample. */

    /* first chunk is the RIFF chunk, let's read that info */  
    fread(chunkId, 1, 4, inFile);
    fread(&chunkSize, 1, 4, inFile);
    fread(data, 1, 4, inFile);

    /* let's try to read the next chunk, it always starts with an id */
    fread(chunkId, 1, 4, inFile);
    /* if the next chunk is not "fmt " then let's skip over it */  
    while(strcmp(chunkId, "fmt ") != 0) {
        fread(&chunkSize, 1, 4, inFile);
        /* skip to the end of this chunk */  
        fseek(inFile, chunkSize, SEEK_CUR);
        /* read the id of the next chuck */  
        fread(chunkId, 1, 4, inFile);
    }  

    /* if we are here, then we must have the fmt chunk, now read that data */  
    fread(&chunkSize, 1, 4, inFile);
    fread(&audioFormat, 1,  sizeof(audioFormat), inFile);
    fread(&numChannels, 1,  sizeof(numChannels), inFile);
    /* you'll need more reads here, hear? */ 
    fread(&sampleRate, 1,  sizeof(sampleRate), inFile);
    /* skip byte rate and block align to get to position of bits per sample*/
    fseek(inFile, 4, SEEK_CUR);
    fseek(inFile, 2, SEEK_CUR);
    fread(&bitsPerSample, 1, sizeof(bitsPerSample), inFile); 

    /* read the data chunk next, use another while loop (like above) */
    /* visit http://goo.gl/rxnHB1 for helpful advice */
    /* let's try to read the data chunk to get the date chunk id and data size (num samples) */
    fread(chunkId, 1, 4, inFile);
    /* if the next chunk is not "data" then let's skip over it */  
    while(strcmp(chunkId, "data") != 0) {
        fread(&chunkSize, 1, 4, inFile);
        /* skip to the end of this chunk */  
        fseek(inFile, chunkSize, SEEK_CUR);
        /* read the id of the next chuck */  
        fread(chunkId, 1, 4, inFile);
    }

    /* if we are here, then we must have the fmt chunk, now read that data */  
    fread(&chunkSize, 1, 4, inFile);

    *sampleSizePtr = bitsPerSample;
    if (numChannels == 2){
        *numSamplesPtr = (chunkSize * 8 / bitsPerSample) / 2;
    }
    else{
        *numSamplesPtr = chunkSize * 8 / bitsPerSample;
    }
    *sampleRatePtr = sampleRate;
    *numChannelsPtr = numChannels;
    return (audioFormat == 1);
}


/**
 *  function reads the WAV audio data (last part of the data chunk) and gets the encrypted message
 */
int readWavData(FILE* inFile, short sampleSize, int numSamples, short numChannels, int numLSB, char textFileName[]) {
    FILE* outputFilePtr;            /* pointer to the output plaintext file */
    int bytesPerSample = sampleSize / 8;
    char ch = ' ';
    char previousChar = ' ';        /* used to detect the end of file */
    int sample;            
    int mask = 0;                   /* bit mask used to to decrypt message */
    int readSample = 0;             /* the total number of samples that are read */
    int numChars = 0;               /* the total number of characters that are deciphered */
    int i = 0;

    /* determine the type of bit mask used */
    if (numLSB == 1) {
        mask = 1;
    }
    else if (numLSB == 2) {
        mask = 3;
    }
    else {
        mask = 15;
    }

    /* if the number of channels is 2, we need to double numSamples */
    if (numChannels == 2) {
        numSamples *= 2;
    }

    /* read the encrypted message */
    outputFilePtr = fopen(textFileName, "we");
    while (readSample <= numSamples) {
        for (i = 0; i < (8 / numLSB); i++) {
            fread(&sample, 1, bytesPerSample, inFile);
            if (i == 0) {
                ch = sample & mask;
            } else {
                ch = (ch << numLSB) | (sample & mask);
            }
            readSample++;
        }
        /* shows if we reach the end of the encrypted message */
        fprintf(outputFilePtr, "%c", ch);
        numChars++;
        if (previousChar == ':' && ch == ')') {
            goto print;
        }
        previousChar = ch;
        ch = ' ';
    }

    print:
    printf("%d samples are read \n", readSample);
    printf("%d characters are deciphered \n", numChars);
    fclose(outputFilePtr);
    return 1;
}