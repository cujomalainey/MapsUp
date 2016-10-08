#ifndef MDUINO_h
#define MDUINO_h

#include "Arduino.h"

#include <inttypes.h>

// This value is determined by the largest message size available
#define MDUINO_MAX_MSG_DATA_SIZE 100
/**
 * Driver Error Codes
 */
#define NO_ERROR                          0
#define CHECKSUM_FAILURE                  1
#define PACKET_EXCEEDS_BYTE_ARRAY_LENGTH  2
#define UNEXPECTED_START_BYTE             3

/**
 * Framework Defines
 */
#define MESSAGE_SIZE                    0x08
#define INVALID_REQUEST                 0xFF
#define BITS_IN_BYTE                    0x08

class IncomingMessage {
public:
	IncomingMessage();
	uint8_t getMsgId();
	void setMsgId(uint8_t msgId);
	uint8_t getLength();
	void setLength(uint8_t length);
	uint8_t getChecksum();
	void setChecksum(uint8_t checksum);
	void setFrameData(uint8_t* frameDataPtr);
	uint8_t* getFrameData();

	void setFrameLength(uint8_t frameLength);

	uint8_t getFrameLength();

	void reset();

	void init();

	bool isAvailable();
	void setAvailable(bool complete);

	bool isError();

	uint8_t getErrorCode();
	void setErrorCode(uint8_t errorCode);
protected:
	// pointer to frameData
	uint8_t* _frameDataPtr;
private:
	void setCommon(IncomingMessage &target);
	uint8_t _msgId;
	uint8_t _length;
	uint8_t _checksum;
	bool _complete;
	uint8_t _errorCode;
};

class OutgoingMessage {
public:
	OutgoingMessage();
	OutgoingMessage(uint8_t msgId);

	uint8_t getMsgId();
	void setMsgId();

	void setData(uint8_t* ptr);
	uint8_t getData(uint8_t pos);

	uint8_t getDataLength();
	void setDataLength(uint8_t length);
	//void reset();
	void setMsgId(uint8_t msgId);
private:
	uint8_t _msgId;
	uint8_t* _dataPtr;
	uint8_t _length;
};

class mduino {
public:
	mduino();

	void readPacket();
	bool readPacket(int timeout);
	void readPacketUntilAvailable();
	void begin(UARTClass &serial);
	void getResponse(IncomingMessage &response);
	IncomingMessage& getResponse();
	void send(OutgoingMessage &request);
	//uint8_t sendAndWaitForResponse(OutgoingMessage &request, int timeout);
	void setSerial(UARTClass &serial);
private:
	bool available();
	uint8_t read();
	void flush();
	void write(uint8_t val);
	void resetResponse();
	IncomingMessage _response;
	// current packet position for response.  just a state variable for packet parsing and has no relevance for the response otherwise
	uint8_t _pos;
	// last byte read
	uint8_t b;
	uint8_t _checksumTotal;
	// buffer for incoming RX packets.  holds only the msg specific frame data, starting after the msg id byte and prior to checksum
	uint8_t _responseFrameData[MDUINO_MAX_MSG_DATA_SIZE];
	UARTClass* _serial;
};


#endif //MDUINO_h
