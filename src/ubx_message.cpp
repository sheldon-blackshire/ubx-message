
#include "ubx_message.h"

#include <string.h>

UbxMessage::UbxMessage(uint8_t* buffer, uint16_t size) : state_(DeserializationState::kHeader0),
                                                         packet_(),
                                                         payload_counter_(),
                                                         kMaxPayloadSize_(size),
                                                         payload_null_byte_() {
    this->packet_.payload_ = buffer;
}

UbxMessage::~UbxMessage() {
}

uint8_t& UbxMessage::operator[](uint16_t index) {
    if (this->packet_.payload_ == nullptr) {
        return this->payload_null_byte_;
    }

    if (index < this->kMaxPayloadSize_) {
        return this->packet_.payload_[index];
    } else {
        return this->packet_.payload_[this->kMaxPayloadSize_ - 1];
    }
}

void UbxMessage::Init(bool set_headers) {
    if (set_headers) {
        this->packet_.header_0_ = 0xb5;
        this->packet_.header_1_ = 0x62;
    } else {
        this->packet_.header_0_ = 0;
        this->packet_.header_1_ = 0;
    }

    this->packet_.class_ = 0;
    this->packet_.id_ = 0;
    this->packet_.length_0_ = 0;
    this->packet_.length_1_ = 0;
    this->packet_.chk_a_ = 0;
    this->packet_.chk_b_ = 0;

    if (this->packet_.payload_) {
        memset(this->packet_.payload_, 0, this->kMaxPayloadSize_);
    }

    this->ResetState();
    this->payload_counter_ = 0;
}

void UbxMessage::Update() {
    this->Checksum(this->packet_.chk_a_, this->packet_.chk_b_);
}

uint16_t UbxMessage::Checksum() {
    uint8_t chk_a = 0;
    uint8_t chk_b = 0;

    this->Checksum(chk_a, chk_b);

    uint16_t checksum = static_cast<uint16_t>(chk_b) << 8;
    checksum |= static_cast<uint16_t>(chk_a);
    return checksum;
}

void UbxMessage::Checksum(uint8_t& a, uint8_t& b) {
    uint8_t chk_a = 0;
    uint8_t chk_b = 0;

    const uint8_t buffer[4] = {
        this->packet_.class_,
        this->packet_.id_,
        this->packet_.length_0_,
        this->packet_.length_1_};

    for (uint8_t idx = 0; idx < 4; idx++) {
        chk_a += buffer[idx];
        chk_b += chk_a;
    }
    for (uint16_t idx = 0; idx < this->Length(); idx++) {
        chk_a += this->packet_.payload_[idx];
        chk_b += chk_a;
    }

    a = chk_a;
    b = chk_b;
}

bool UbxMessage::Valid() {
    uint8_t a = 0;
    uint8_t b = 0;
    this->Checksum(a, b);

    if (a != this->packet_.chk_a_) {
        return false;
    } else if (b != this->packet_.chk_b_) {
        return false;
    }

    return true;
}

void UbxMessage::Length(uint16_t len) {
    this->packet_.length_1_ = static_cast<uint8_t>((len & 0xff00) >> 8);
    this->packet_.length_0_ = static_cast<uint8_t>((len & 0x00ff));
}

uint16_t UbxMessage::Length() const {
    return static_cast<uint16_t>(this->packet_.length_0_) |
           static_cast<uint16_t>(this->packet_.length_1_) << 8;
}

bool UbxMessage::Deserialize(uint8_t byte) {
    switch (this->state_) {
        case DeserializationState::kHeader0:
            if (byte == 0xb5) {
                this->packet_.header_0_ = byte;
                this->IncrementState();
            }
            break;
        case DeserializationState::kHeader1:
            if (byte == 0x62) {
                this->packet_.header_1_ = byte;
                this->IncrementState();
            } else {
                this->ResetState();
            }
            break;
        case DeserializationState::kClass:
            this->packet_.class_ = byte;
            this->IncrementState();
            break;
        case DeserializationState::kId:
            this->packet_.id_ = byte;
            this->IncrementState();
            break;
        case DeserializationState::kLength0:
            this->packet_.length_0_ = byte;
            this->IncrementState();
            break;
        case DeserializationState::kLength1:
            this->packet_.length_1_ = byte;
            this->payload_counter_ = 0;
            this->IncrementState();
            break;
        case DeserializationState::kPayload:
            if (this->payload_counter_ < this->Length()) {
                if (this->packet_.payload_) {
                    if (this->payload_counter_ < this->kMaxPayloadSize_) {
                        this->packet_.payload_[this->payload_counter_++] = byte;
                    }
                }
                break;
            }

            this->IncrementState();

        case DeserializationState::kChecksumA:
            this->packet_.chk_a_ = byte;
            this->IncrementState();
            break;
        case DeserializationState::kChecksumB:
            this->packet_.chk_b_ = byte;
            this->ResetState();
            return true;
        default:
            break;
    }
    return false;
}

uint16_t UbxMessage::Serialize(uint8_t* buffer, uint16_t size) {
    const uint16_t kPayloadLength = this->Length();
    const uint16_t kPacketSize = kPayloadLength + 8;
    if (size < kPacketSize) {
        return 0;
    }

    uint16_t write_index = 0;

    buffer[write_index++] = this->packet_.header_0_;
    buffer[write_index++] = this->packet_.header_1_;
    buffer[write_index++] = this->packet_.class_;
    buffer[write_index++] = this->packet_.id_;
    buffer[write_index++] = this->packet_.length_0_;
    buffer[write_index++] = this->packet_.length_1_;

    if (this->packet_.payload_) {
        for (uint16_t idx = 0; idx < kPayloadLength; idx++) {
            buffer[write_index++] = this->packet_.payload_[idx];
        }
    } else {
        memset(&buffer[write_index], 0, kPayloadLength);
        write_index += kPayloadLength;
    }

    buffer[write_index++] = this->packet_.chk_a_;
    buffer[write_index++] = this->packet_.chk_b_;

    return write_index;
}

UbxMessage::DeserializationState UbxMessage::FirstState() const {
    return DeserializationState::kHeader0;
}
UbxMessage::DeserializationState UbxMessage::LastState() const {
    return DeserializationState::kChecksumB;
}

void UbxMessage::IncrementState() {
    if (this->state_ == this->LastState()) {
        this->state_ = this->FirstState();
    } else {
        const int16_t kIncState = static_cast<int16_t>(this->state_) + 1;
        this->state_ = static_cast<DeserializationState>(kIncState);
    }
}
void UbxMessage::ResetState() {
    this->state_ = this->FirstState();
}
