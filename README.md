# ubx-message
Ublox Ubx-protocol message Serialize/Deserialize c++ class intended for small embedded applications.

## Dependencies

 - string.h
 - stdint.h
 
## Serialize

    const uint16_t kMemorySize = 32;
	uint8_t kMemoryPool[kMemorySize] = {0};
	UbxMessage message(kMemoryPool, kMemorySize);
 
    message.Init(true);
	message.Class(0x13);
	message.Id(0x21);
	message.Length(6);
	message[0] = 3;
	message[1] = 0;
	message[2] = 0;
	message[3] = 0;
	message[4] = 0;
	message[5] = 0;

	message.Update();
	const uint8_t kSerializedBufferSize = 32;
	uint8_t serialized_buffer[kSerializedBufferSize] = {0};
	const uint16_t kMessageLength = message.Serialize(serialized_buffer, kSerializedBufferSize);

## Deserialize

    const uint16_t kMemorySize = 32;
	uint8_t kMemoryPool[kMemorySize] = {0};
	UbxMessage message(kMemoryPool, kMemorySize);
	
	const uint8_t serial_data[14] = {
			0xB5, 0x62, 0x13, 0x21, 0x06,
			0x00, 0x03, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x3D, 0x29
	};
	
	const uint16_t kSize = sizeof(serial_data)/sizeof(serial_data[0]);
	for(uint16_t idx = 0; idx < kSize; idx++){
		if(message.Deserialize(serial_data[idx])){
			// Got payload
			printf("Class:  %02x\r\n", message.Class());
			printf("Id:     %02x\r\n", message.Id());
			printf("Length: %02x\r\n", message.Length());
			printf("ChkSum: %04x\r\n", message.Checksum());
			printf("Payload: ");
			for(uint16_t jdx = 0; jdx < message.Length(); jdx++){
				printf("%02x ", message[jdx]);
			}
			printf("\r\n");
			printf("Valid:%u", message.Valid());
		}
	}

