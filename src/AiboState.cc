#include "AiboState.h"

AiboState::AiboState(){

}

AiboState::~AiboState(){
	delete aibolink;
}

int extract(char data, int i){
	return (data >> i ) & 0x01;
}

void extractChar(char data){
	int c[8];	
	int i;
	for ( i = 0; i < 8; i++){
		c[i] = extract(data, i);
	}

	for ( i = 7; i >= 0; i--){
		printf("%d", c[i]);
	}
	printf(" ");	
}

long AiboState::convertLong(char *buff)
{
    long retval = 0;
    retval += (buff[0] & 0xFF) <<  0;
    retval += (buff[1] & 0xFF) <<  8;
    retval += (buff[2] & 0xFF) << 16;
    retval += (buff[3] & 0xFF) << 24;
    return retval;
}



void AiboState::connect(const char *ip_addr, int port)
{

    aibolink = new AiboNet(ip_addr, port);

}

// Reads State Data from World State 
void AiboState::readData(){
	stateData.model = aibolink->readUntil((char)0);        // printf("model: %s\n", stateData.model);
	stateData.timeStamp = convertLong(aibolink->read(4));  // printf("ts: %ld\n", stateData.timeStamp);
	stateData.frame = convertLong(aibolink->read(4)); 	   // printf("frame: %ld\n", stateData.frame);

	long NumOutputs = *aibolink->read(4); 		           //printf("numOutputs: %ld\n", NumOutputs);
	for (int i = 0; i < NumOutputs; i++){
		stateData.positions[i] = *(float*)aibolink->read(4); //printf("pos[%d]: %ld\n", i, stateData.positions[i]);
	}
	
	long NumSensors = *aibolink->read(4); 		           //printf("NumSensors: %ld\n", NumSensors);
	for (int i = 0; i < NumSensors; i++){
		stateData.sensors[i] = *(float*)aibolink->read(4); //printf("sensor[%d] %f\n", i, stateData.sensors[i]);
	}
	
	long NumButtons = *aibolink->read(4); 		           //printf("NumButtons: %ld\n", NumButtons);
	for (int i = 0; i < NumButtons; i++){
		stateData.buttons[i] = *(float*)aibolink->read(4); //printf("button[%d] = %f\n", i, stateData.buttons[i]);
	}

	long NumPIDJoints = *aibolink->read(4); 	           //printf("NumPIDJoints: %ld\n", NumPIDJoints);
	for (int i = 0; i < NumPIDJoints; i++){
		stateData.duties[i] = *aibolink->read(4);          //printf("jointDC[%d] = %f\n", i, stateData.duties[i]);
	}
}

	
