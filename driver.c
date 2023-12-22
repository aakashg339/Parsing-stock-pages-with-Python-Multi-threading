#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MESSAGE_SIZE 1024
#define NUMBER_OF_FILES 100

#define PARSE_DATA_LOG_FILE "parsed_data.log"
#define OUTPUT_FILE "output.txt"

pthread_mutex_t mutex;

// Function to make the call to python file to get data
void makingTheCallToPythonFile(char *htmlFileName) {
    pid_t pid = fork();
    if (pid == 0) {
        execlp("python3", "python3", "parser.py", htmlFileName, OUTPUT_FILE, NULL);
    }
    wait(NULL);
}

// Function to read data from python output file
char* readDataFromPythonOutputFile() {
    FILE *fp = fopen(OUTPUT_FILE, "r");
    
    if(fp != NULL) {
        char data[MESSAGE_SIZE];
        fgets(data, sizeof(data), fp);
        fclose(fp);

        char *filedata;
        filedata = (char*)malloc(MESSAGE_SIZE);
        strcpy(filedata, data);

        return filedata;
    }
    else {
        return "No Data";
    }
}

void writeToFile(char *data) {

    FILE *fp = fopen("parsed_data.log", "a");
    if(fp != NULL && strlen(data) > 0) {
        strcat(data, "\n");
        fputs(data, fp);
    }
    fclose(fp);
}

void *callingThePythonParserToWriteToFile(void *arg) {
    
    pthread_mutex_lock(&mutex);
    char *htmlFileName = (char *)arg;

    makingTheCallToPythonFile(htmlFileName);

    char *fileData = readDataFromPythonOutputFile();
    
    writeToFile(fileData);
    pthread_mutex_unlock(&mutex);

    return NULL;
}

void main() {
    char buffer[MESSAGE_SIZE];
    char fileNames[NUMBER_OF_FILES][MESSAGE_SIZE];
    int i, numberOfFilesFromDrive = 0;

    pthread_mutex_init(&mutex, NULL);

    for(i=0; i<100; i++) {
        bzero(fileNames[i], MESSAGE_SIZE);
    }

    
    // Getting the list of files
    chdir("http_dump/dump_http/files");
    FILE *fp = popen("ls", "r");

    bzero(buffer, MESSAGE_SIZE);

    // Read output of ls command using fgets and send to client
    numberOfFilesFromDrive = 0;
    while(fgets(buffer, MESSAGE_SIZE, fp) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0;
        strcpy(fileNames[numberOfFilesFromDrive], buffer);
        bzero(buffer, MESSAGE_SIZE);
        numberOfFilesFromDrive++;
    }
    fclose(fp);

    chdir("..");
    chdir("..");
    chdir("..");

    pthread_t threads[numberOfFilesFromDrive];

    for (int i = 0; i < numberOfFilesFromDrive; i++) {
        char *fileName;
        fileName = (char*)malloc(MESSAGE_SIZE);
        strcpy(fileName, fileNames[i]);
        pthread_create(&threads[i], NULL, callingThePythonParserToWriteToFile, fileName);
    }

    for (int i = 0; i < numberOfFilesFromDrive; i++) {
        pthread_join(threads[i], NULL);
    }

    // Read parsed_data.log and find the highest stock price
    fp = fopen("parsed_data.log", "r");
    if(fp == NULL) {
        printf("\nError opening parsed_data.log. Exiting.");
    }
    else {
        char maxCompany[MESSAGE_SIZE];
        float maxinumPrice = 0.0;
        
        for(i=0; i<numberOfFilesFromDrive; i++) {
        	char companyName[MESSAGE_SIZE];
            float priceShare;
            char *token;
            bzero(buffer, MESSAGE_SIZE);
            fgets(buffer, MESSAGE_SIZE, fp);
            token = strtok(buffer, "=");
            strcpy(companyName, token);
            token = strtok(NULL, " ");
            priceShare = atof(token);
            if (priceShare > maxinumPrice) {
                maxinumPrice = priceShare;
                strcpy(maxCompany, companyName);
            }
        }

        printf("\nHighest performing stock: %s with price: %f\n", maxCompany, maxinumPrice);
    }
	
	fclose(fp);

    pthread_mutex_destroy(&mutex);

}