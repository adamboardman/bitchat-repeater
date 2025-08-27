#include "Message.h"

Message::Message() : PacketBase(type_message), sender_peer(nullptr) {
}

Message::Message(const uint8_t ttl, const uint64_t timestamp, const uint8_t packet_flags, const uint64_t sender)
    : PacketBase(type_message, ttl, timestamp, packet_flags, sender), sender_peer(nullptr) {
}

Message::Message(const uint8_t ttl, const uint64_t timestamp, const uint8_t packet_flags, const uint64_t sender,
                 const uint64_t recipient, const std::string &signature)
    : PacketBase(type_message, ttl, timestamp, packet_flags, sender, recipient, signature),
      sender_peer(nullptr) {
}

void Message::setMessageFlags(const uint8_t flags) {
    message_flags = flags;
}

void Message::setMessageTimestamp(uint64_t value) {
    message_timestamp = value;
}

void Message::setMessageId(const std::string &string) {
    message_id = string;
}

void Message::setSenderNickname(const std::string &string) {
    sender_nickname = string;
}

void Message::setContent(const std::string &string) {
    content = string;
}

void Message::setEncryptedContent(const std::string &string) {
    encrypted_content = string;
}

void Message::setOriginalSenderNickname(const std::string &string) {
    original_sender_nickname = string;
}

void Message::setRecipientNickname(const std::string &string) {
    recipient_nickname = string;
}

void Message::addMention(const std::string &string) {
    mentions.push_back(string);
}

void Message::setChannel(const std::string &string) {
    channel = string;
}

uint8_t Message::getMessageFlags() const {
    return message_flags;
}

uint64_t Message::getMessageTimestamp() const {
    return message_timestamp;
}

const std::string &Message::getMessageId() const {
    return message_id;
}

const std::string &Message::getSenderNickname() const {
    return sender_nickname;
}

const std::string &Message::getContent() const {
    return content;
}

const std::string &Message::getOriginalSenderNickname() const {
    return original_sender_nickname;
}

const std::string &Message::getRecipientNickname() const {
    return recipient_nickname;
}

void Message::setSenderPeer(Peer *peer) {
    sender_peer = peer;
}

Peer *Message::getSenderPeer() const {
    return sender_peer;
}

const std::vector<std::string> *Message::getMentions() const {
    return &mentions;
}

const std::string &Message::getChannel() const {
    return channel;
}
