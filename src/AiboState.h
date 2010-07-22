#ifndef AIBO_STATE_H
#define AIBO_STATE_H

#include "AiboNet.h"


/** Holds all the information provided by the World State
 *  serializer published by the Aibo.
 */
struct AiboState_t {
	char* model;
	long  timeStamp;
	long  frame;
	float positions[47];
	float duties[18];
	float sensors[11];
	float buttons[10];
};

/** 
 * Collects state information about Aibo including
 * model information, timestamp of the data, the frame number,
 * the positions of the joins, the duties of joints, sensors, and
 * buttons.  Full documentation can be read in the WorldStateSerialize.h
 * in Tekkotsu/Behaviors/Mon/.
 *
 *
 */
class AiboState{
public:

	AiboState();

	// Create socket and connect to Aibo
	void connect(const char *ip_addr, int port);

	// Disconnect from Aibo
	void disconnect();

	~AiboState();

	// Converts a char* buffer to a long
	long convertLong(char *buff);

	// Takes an unsigned int and returns a char string binary equivalent
	char* binary(unsigned int v);

	// Read from the World State socket
	void readData();

	// Holds the state information.  Obviously should be private.
	AiboState_t	stateData;

private:

	// Sockt connection to the Aibo
	AiboNet *aibolink;

	// Set to true to print debug statements
	bool debug;

	// Buffer used to temporarily hold information from the Aibo
	char* readBuffer;
};

#endif
