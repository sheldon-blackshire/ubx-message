#ifndef GPS_UBX_SERIALIZE_STATE_H_
#define GPS_UBX_SERIALIZE_STATE_H_

#include <stdint.h>

typedef enum {
	kHeader0 = 0,
	kHeader1,
	kClass,
	kId,
	kLength0,
	kLength1,
	kPayload,
	kChecksumA,
	kChecksumB
}UbxPacketField;

class UbxSerializationState {
private:
	UbxPacketField state_;
public:

	UbxSerializationState() : state_() {
		this->Reset();
	}
	~UbxSerializationState(){}

	void Reset(){
		this->state_ = this->First();
	}
	UbxPacketField Current() const {return state_;}
	UbxPacketField First() const {return UbxPacketField::kHeader0;}
	UbxPacketField Last() const {return UbxPacketField::kChecksumB;}
	void Increment(){
	    if (this->state_ == this->Last()) {
	        this->state_ = this->First();
	    } else {
	        const int16_t kIncState = static_cast<int16_t>(this->state_) + 1;
	        this->state_ = static_cast<UbxPacketField>(kIncState);
	    }
	}
};

#endif /* GPS_UBX_SERIALIZE_STATE_H_ */
