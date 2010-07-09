#ifndef AIBO_STATE_H
#define AIBO_STATE_H

#include "AiboNet.h"

struct AiboState_t {
	char* model;
	long  timeStamp;
	long  frame;
	float positions[47];
	float duties[18];
	float sensors[11];
	float buttons[10];
};

//! \brief Collects state information about Aibo
class AiboState{
public:
	AiboState();
	void connect(const char *ip_addr, int port);
	void disconnect();
	~AiboState();

	long convertLong(char *buff);
	char* binary(unsigned int v);
	void readData();
	AiboState_t	stateData;
private:
	AiboNet *aibolink;
	bool debug;
	char* readBuffer;
};

#endif
