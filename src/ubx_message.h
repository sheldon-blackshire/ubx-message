#ifndef GPS_UBX_MESSAGE_H_
#define GPS_UBX_MESSAGE_H_

#include <stdint.h>
#include "ubx_serialize_state.h"

class UbxMessage {
private:

	struct UbxPacket{
		uint8_t header_0_;
		uint8_t header_1_;
		uint8_t class_;
		uint8_t id_;
		uint8_t length_0_;
		uint8_t length_1_;
		uint8_t* payload_;
		uint8_t chk_a_;
		uint8_t chk_b_;
	};

	UbxPacket packet_;
	const uint16_t kMaxPayloadSize_;
	uint16_t payload_counter_;
	uint8_t payload_null_byte_;

	UbxSerializationState state_;

	/*
	 * Calculates the checksum of the packet using an
	 * 8-bit Fletcher algorithm. The checksum of a packet
	 * consists of the class, id , length, and payload. It doesn't
	 * include the header, or checksum itself. This function does not
	 * update the internal checksum, but rather stores the data inside
	 * input variables a and b.
	 */

	void Checksum(uint8_t& a, uint8_t& b);

public:

	UbxMessage(uint8_t* buffer, uint16_t size);
	virtual ~UbxMessage();

	void Length(uint16_t len);
	uint16_t Length() const;

	const uint8_t* Payload() const {return packet_.payload_;}

	void Class(uint8_t c) {packet_.class_ = c;}
	uint8_t Class() const {return packet_.class_;}

	void Id(uint8_t i) {packet_.id_ = i;}
	uint8_t Id() const {return packet_.id_;}

	uint16_t Size();
	static uint8_t OverheadSize() {return 8;}

	/*
	 * Access the message payload using array notation.
	 * If an out-of-bounds memory access is attempted, the last byte in
	 * the payload is returned. Array notation access is safe when an internal
	 * working buffer is not present, however the results are meaningless.
	 *
	 * Example Usage:
	 *
	 *		uint8_t kMemoryPool[128] = {0};
	 *		UbxMessage message(kMemoryPool, 128);
	 *		message.Length(1);  <- Assigns payload lenth
	 *		message[0] = 0x01;  <- Sets the payload byte
	 */

	uint8_t &operator[] (uint16_t index);

	/*
	 * Retrieves the 16-bit checksum using the fletcher algorithm.
	 * When this function is called, the interal checksum data is update.
	 */
	uint16_t Checksum();

	/*
	 * Initializes the internal packet structure to zeros and resets the
	 * Ingest() state machine. Note: If a working buffer for payload is specified
	 * in the device constructor, all values in the buffer will be zero'd. When
	 * set_headers is true, 0xb5 and 0x62 are populated for the packet header instead
	 * of zeros (This is useful for packet construction).
	 */
	void Init(bool set_headers, bool zero_memory = true);

	/*
	 * Updates the packet checksum. This function must be called after making any
	 * modifications to the packet. Failure to call update could result in failure
	 * when using Serialize() or Valid().
	 */

	void Update();

	/*
	 * Determines if the packet checksum is valid. This function should be called
	 * following Deserialize() returning true.
	 */
	bool Valid();

	/*
	 * Parses a serialized Ublox message one byte at a time. Returns true when a packet
	 * has been discovered, otherwise false. This function does not determine checksum
	 * validity, therefore once a packet is found, a call to Valid() should
	 * be made before working with packet data.
	 *
	 * Note: Packet is deserialized based on Little Endian formatting.
	 */
	bool Deserialize(uint8_t byte);

	/*
	 * Copies contents of this Ublox message into an array of bytes (Useful for sending
	 * over a communication interface). A buffer must be provided that will house the
	 * serialized message, as well as the max size of the buffer. The length of the serialized
	 * packet is returned on success. Zero is returned if the supplied input buffer size was
	 * not large enough. Endianness is handled internally for the packet overhead, but
	 * its the client responsibility to make the payload endianness consistent with Ublox
	 * documentation.
	 *
	 * Note: The input buffer size must be ATLEAST the payload length, plus 8 bytes of overhead.
	 */
	uint16_t Serialize(uint8_t* buffer, uint16_t size);

    /*
     *  Serializes contents of this Ublox message. Each time the function is called, the next
     *  byte in the message is retrieved. The function returns true when serialization is on-going
     *  and false when the last serialized byte is retrieved. This means that one byte must be
     *  managed after a false value is returned.
     *
     * Note: Endianness is handled internally for the packet overhead, but
	 * its the client responsibility to make the payload endianness consistent with Ublox
	 * documentation.
     */
	bool Serialize(uint8_t& byte);
};

#endif /* GPS_UBX_MESSAGE_H_ */
