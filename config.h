// config.h
#ifndef CONFIG_H
#define CONFIG_H

// Include necessary standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/stat.h>
#include <errno.h>

// Define constant paths
#define LOGFILE "/tmp/data"
#define DEVICES "/proc/bus/input/devices"
#define INPUTSTREAM "/dev/input/"

// Constant limits
#define BUFFER_SIZE 64
#define BUFFER_SOFT_CAP 25
#define BUFFER_INCREASE 2
#define BUFFER_GOAL 10


// Function prototypes
int checkFileExists(FILE *inputFile);
char* getEvent();
int interpretCharacter(char* outputBuffer, struct input_event *eventInput, int *bufferIndex);
char* inputBuffer(int argc, char **argv, char* eventName, int *temporaryBufferCount);

#endif // CONFIG_H
