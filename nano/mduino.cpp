#include "mduino.h"

#include "HardwareSerial.h"

#define MDUINO_START_BYTE 0xA5

// start/length/msg/checksum bytes
#define MDUINO_MSG_OVERHEAD_LENGTH 4
// msg is always the third byte in packet
#define MDUINO_MSG_ID_INDEX 3
#define MDUINO_MSG_FRONT_OVERHEAD 3

IncomingMessage::IncomingMessage() {

}

uint8_t IncomingMessage::getMsgId() {
	return _msgId;
}

void IncomingMessage::setMsgId(uint8_t msgId) {
	_msgId = msgId;
}

uint8_t IncomingMessage::getLength() {
	return _length;
}

void IncomingMessage::setLength(uint8_t length) {
	_length = length;
}

uint8_t IncomingMessage::getChecksum() {
	return _checksum;
}

void IncomingMessage::setChecksum(uint8_t checksum) {
	_checksum = checksum;
}

uint8_t IncomingMessage::getPacketLength() {
	return _length - MDUINO_MSG_OVERHEAD_LENGTH;
}

bool IncomingMessage::isAvailable() {
	return _complete;
}

void IncomingMessage::setAvailable(bool complete) {
	_complete = complete;
}

bool IncomingMessage::isError() {
	return _errorCode > 0;
}

uint8_t IncomingMessage::getErrorCode() {
	return _errorCode;
}

void IncomingMessage::setErrorCode(uint8_t errorCode) {
	_errorCode = errorCode;
}

// copy common fields from xbee response to target response
void IncomingMessage::setCommon(IncomingMessage &target) {
	target.setMsgId(getMsgId());
	target.setAvailable(isAvailable());
	target.setChecksum(getChecksum());
	target.setErrorCode(getErrorCode());
	target.setLength(getLength());
}

uint8_t* IncomingMessage::getFrameData() {
	return _frameDataPtr;
}

void IncomingMessage::setFrameData(uint8_t* frameDataPtr) {
	_frameDataPtr = frameDataPtr;
}

void IncomingMessage::init() {
	_complete = false;
	_errorCode = NO_ERROR;
	_checksum = 0;
}

void IncomingMessage::reset() {
	init();
	_msgId = 0;
	_length = 0;
	_checksum = 0;

	_errorCode = NO_ERROR;
}

void mduino::resetResponse() {
	_pos = 0;
	_checksumTotal = 0;
	_response.reset();
}

mduino::mduino(): _response(IncomingMessage()) {
        _pos = 0;
        _checksumTotal = 0;

        _response.init();
        _response.setFrameData(_responseFrameData);
		// Contributed by Paul Stoffregen for Teensy support
        _serial = &Serial;
}

// Support for SoftwareSerial. Contributed by Paul Stoffregen
void mduino::begin(UARTClass &serial) {
	_serial = &serial;
}

void mduino::setSerial(UARTClass &serial) {
	_serial = &serial;
}

bool mduino::available() {
	return _serial->available();
}

uint8_t mduino::read() {
	return _serial->read();
}

void mduino::write(uint8_t val) {
	_serial->write(val);
}

IncomingMessage& mduino::getResponse() {
	return _response;
}

// TODO how to convert response to proper subclass?
void mduino::getResponse(IncomingMessage &response) {

	response.setLength(_response.getLength());
	response.setMsgId(_response.getMsgId());
	response.setFrameData(_response.getFrameData());
}

void mduino::readPacketUntilAvailable() {
	while (!(getResponse().isAvailable() || getResponse().isError())) {
		// read some more
		readPacket();
	}
}

bool mduino::readPacket(int timeout) {

	if (timeout < 0) {
		return false;
	}

	unsigned long start = millis();

    while (int((millis() - start)) < timeout) {

     	readPacket();

     	if (getResponse().isAvailable()) {
     		return true;
     	} else if (getResponse().isError()) {
     		return false;
     	}
    }

    // timed out
    return false;
}

void mduino::readPacket() {
	// reset previous response
	if (_response.isAvailable() || _response.isError()) {
		// discard previous packet and start over
		resetResponse();
	}

    while (available()) {

        b = read();

		// checksum includes all bytes including the sync
		_checksumTotal ^= b;

        switch(_pos) {
			case 0:
		        if (b == MDUINO_START_BYTE) {
		        	_pos++;
		        }

		        break;
			case 1:
				// length msb
				_response.setLength(b);
				_pos++;

				break;
			case 2:
				_response.setMsgId(b);
				_pos++;

				break;
			default:
				// starts at fourth byte

				if (_pos > MDUINO_MAX_MSG_DATA_SIZE) {
					// exceed max size.  should never occur
					_response.setErrorCode(PACKET_EXCEEDS_BYTE_ARRAY_LENGTH);
					return;
				}

				// check if we're at the end of the packet
				if (_pos == (_response.getLength() + 3)) {
					// verify checksum
					// if the last byte is the checksum
					// then XOR it with itself should be 0

					if (_checksumTotal == 0) {
						_response.setChecksum(b);
						_response.setAvailable(true);

						_response.setErrorCode(NO_ERROR);
					} else {
						// checksum failed
						_response.setErrorCode(CHECKSUM_FAILURE);
					}

					// reset state vars
					_pos = 0;

					return;
				} else {
					// add to packet array, starting with the fourth byte of the msgId
					_response.getFrameData()[_pos - MDUINO_MSG_FRONT_OVERHEAD] = b;
					_pos++;
				}
        }
    }
}

OutgoingMessage::OutgoingMessage() {

}

OutgoingMessage::OutgoingMessage(uint8_t msgId) {
	_msgId = msgId;
}

uint8_t OutgoingMessage::getMsgId() {
	return _msgId;
}

void OutgoingMessage::setMsgId(uint8_t msgId) {
	_msgId = msgId;
}

void OutgoingMessage::setData(uint8_t* ptr) {
	_dataPtr = ptr;
}

void OutgoingMessage::setDataLength(uint8_t length) {
	_length = length;
}

uint8_t OutgoingMessage::getDataLength() {
	return _length;
}

uint8_t OutgoingMessage::getData(uint8_t pos) {
	return _dataPtr[pos];
}

void mduino::send(OutgoingMessage &request) {
	// checksum is XOR of all bytes
	uint8_t checksum = 0;

	// the new new deal
	checksum ^= MDUINO_START_BYTE;
	write(MDUINO_START_BYTE);

	// send length
	uint8_t len = (request.getDataLength());
	checksum ^= len;
	write(len);

	// Msg id
	write(request.getMsgId());
	checksum ^= request.getMsgId();

	for (int i = 0; i < request.getDataLength(); i++) {
		write(request.getData(i));
		checksum ^= request.getData(i);
	}

	// send checksum
	write(checksum);
}
