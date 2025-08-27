#pragma once

#include <cstdint>
#include <string>

#include "PacketBase.h"
#include "Peer.h"


class Message final : public PacketBase {
public:
    Message();

    Message(uint8_t ttl, uint64_t timestamp, uint8_t packet_flags, uint64_t sender);

    Message(uint8_t ttl, uint64_t timestamp, uint8_t packet_flags, uint64_t sender, uint64_t recipient, const std::string &signature);

    void setMessageFlags(uint8_t flags);

    [[nodiscard]] bool isRelay() const {
        return (message_flags & 0x01) != 0;
    }

    [[nodiscard]] bool isPrivate() const {
        return (message_flags & 0x02) != 0;
    }

    [[nodiscard]] bool hasOriginalSender() const {
        return (message_flags & 0x04) != 0;
    }

    [[nodiscard]] bool hasRecipientNickname() const {
        return (message_flags & 0x08) != 0;
    }

    [[nodiscard]] bool hasSenderPeerID() const {
        return (message_flags & 0x10) != 0;
    }

    [[nodiscard]] bool hasMentions() const {
        return (message_flags & 0x20) != 0;
    }

    [[nodiscard]] bool hasChannel() const {
        return (message_flags & 0x40) != 0;
    }

    [[nodiscard]] bool isEncrypted() const {
        return (message_flags & 0x80) != 0;
    }

    void setMessageTimestamp(uint64_t value);

    void setMessageId(const std::string &string);

    void setSenderNickname(const std::string &string);

    void setContent(const std::string &string);

    void setEncryptedContent(const std::string &string);

    void setOriginalSenderNickname(const std::string &string);

    void setRecipientNickname(const std::string &string);

    void setSenderPeer(Peer *peer);

    void addMention(const std::string &string);

    void setChannel(const std::string &string);

    [[nodiscard]] uint8_t getMessageFlags() const;

    [[nodiscard]] uint64_t getMessageTimestamp() const;

    [[nodiscard]] const std::string &getMessageId() const;

    [[nodiscard]] const std::string &getSenderNickname() const;

    [[nodiscard]] const std::string &getContent() const;

    [[nodiscard]] const std::string &getOriginalSenderNickname() const;

    [[nodiscard]] const std::string &getRecipientNickname() const;

    [[nodiscard]] Peer *getSenderPeer() const;

    [[nodiscard]] const std::vector<std::string> *getMentions() const;

    [[nodiscard]] const std::string &getChannel() const;

private:
    uint8_t message_flags = 0;
    uint64_t message_timestamp = 0;
    std::string message_id{};
    std::string sender_nickname{};
    std::string original_sender_nickname{};
    std::string content{};
    std::string encrypted_content{};
    std::string recipient_nickname{};
    Peer *sender_peer;
    std::vector<std::string> mentions;
    std::string channel{};
};
