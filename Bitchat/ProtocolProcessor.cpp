#include "Debugging.h"
#include "int_types.h"

#include "ProtocolProcessor.h"

#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <string>
#include <cinttypes>

#include "BinaryReader.h"
#include "BitchatPacketTypes.h"
#include "ProtocolWriter.h"
#include "libdeflate.h"
#include "PacketPassAlong.h"

extern void print_named_data(const char *name, const uint8_t *data, uint16_t data_size);

const char *ProtocolProcessor::stringForType(const uint8_t type) {
    switch (type) {
        case type_announce:
            return "Announce";
        case type_leave:
            return "Leave";
        case type_message:
            return "Message";
        case type_fragment_start:
            return "FRAGMENT_START";
        case fragmentContinue:
            return "FRAGMENT_CONTINUE";
        case fragmentEnd:
            return "FRAGMENT_END";
        case channelAnnounce:
            return "CHANNEL_ANNOUNCE";
        case channelRetention:
            return "CHANNEL_RETENTION";
        case deliveryAck:
            return "DELIVERY_ACK";
        case deliveryStatusRequest:
            return "DELIVERY_STATUS_REQUEST";
        case readReceipt:
            return "READ_RECEIPT";
        case noiseHandshakeInit:
            return "NOISE_HANDSHAKE_INIT";
        case noiseHandshakeResp:
            return "NOISE_HANDSHAKE_RESP";
        case noiseEncrypted:
            return "NOISE_ENCRYPTED";
        case noiseIdentityAnnounce:
            return "NOISE_IDENTITY_ANNOUNCE";
        case channelKeyVerifyRequest:
            return "CHANNEL_KEY_VERIFY_REQUEST";
        case channelKeyVerifyResponse:
            return "CHANNEL_KEY_VERIFY_RESPONSE";
        case channelPasswordUpdate:
            return "CHANNEL_PASSWORD_UPDATE";
        case channelMetadata:
            return "CHANNEL_METADATA";
        case handshakeRequest:
            return "HANDSHAKE_REQUEST";
        case versionHello:
            return "VERSION_HELLO";
        case versionAck:
            return "VERSION_ACK";
        default:
            return "UnknownType";
    }
}

void ProtocolProcessor::updateOrStorePeerName(const uint64_t sender, const std::string &peer_name, uint8_t ttl, BleConnection &connection) const {
    auto &peer = ble_connection_tracker.checkSenderInPeers(sender);
    peer.updateName(peer_name);
    if (ttl >= peer.getAnnounceTtl()) {
        peer.setAnnounceTtl(ttl);
        ble_connection_tracker.setConnectionHandleForPeer(connection.getConnectionHandle(), &peer);
    }
}

void ProtocolProcessor::updateOrStorePeerNoisePublicKey(const uint64_t sender,
                                                        const std::vector<uint8_t> &public_key) const {
    auto &peer = ble_connection_tracker.checkSenderInPeers(sender);
    peer.updateNoisePublicKey(public_key);
}

bool ProtocolProcessor::processMessage(Message &message, const uint8_t *payload, uint16_t payload_length) const {
    if (payload_length < 13) {
        LOG_DEBUG("Payload too small to be a message: %d\n", payload_length);
        return false;
    }

    BinaryReader reader(0, payload, payload_length);
    message.setMessageFlags(reader.read_uint8());
    message.setMessageTimestamp(reader.read_uint64());

    const auto id_len = reader.read_uint8();
    const auto id = reader.read_data(id_len);
    if (id == nullptr) {
        LOG_DEBUG("Data corrupted: invalid message id\n");
        return false;
    }
    print_named_data("message id", id, id_len);
    message.setMessageId(std::string(reinterpret_cast<const char *>(id), id_len));

    const auto sender_len = reader.read_uint8();
    const auto sender = reader.read_data(sender_len);
    if (sender == nullptr) {
        LOG_DEBUG("Data corrupted: invalid sender nickname id\n");
        return false;
    }
    print_named_data("sender", sender, sender_len);
    message.setSenderNickname(std::string(reinterpret_cast<const char *>(sender), sender_len));

    const auto content_len = reader.read_uint16();
    const auto content = reader.read_data(content_len);
    if (content == nullptr) {
        LOG_DEBUG("Data corrupted: invalid content\n");
        return false;
    }
    print_named_data("content", content, content_len);
    if (message.isEncrypted()) {
        message.setEncryptedContent(std::string(reinterpret_cast<const char *>(content), content_len));
    } else {
        message.setContent(std::string(reinterpret_cast<const char *>(content), content_len));
    }

    if (message.hasOriginalSender()) {
        const auto original_sender_len = reader.read_uint8();
        const auto original_sender = reader.read_data(original_sender_len);
        if (original_sender == nullptr) {
            LOG_DEBUG("Data corrupted: invalid original_sender\n");
            return false;
        }
        print_named_data("original_sender", original_sender, original_sender_len);
        message.setOriginalSenderNickname(std::string(reinterpret_cast<const char *>(original_sender),
                                                      original_sender_len));
    }

    if (message.hasRecipientNickname()) {
        const auto recipient_nickname_len = reader.read_uint8();
        const auto recipient_nickname = reader.read_data(recipient_nickname_len);
        if (recipient_nickname == nullptr) {
            LOG_DEBUG("Data corrupted: invalid recipient_nickname\n");
            return false;
        }
        print_named_data("recipient_nickname", recipient_nickname, recipient_nickname_len);
        message.setRecipientNickname(std::string(reinterpret_cast<const char *>(recipient_nickname),
                                                 recipient_nickname_len));
    }

    if (message.hasSenderPeerID()) {
        const auto sender_peer_id_len = reader.read_uint8();
        const auto sender_peer_id = reader.read_data(sender_peer_id_len);
        if (sender_peer_id == nullptr) {
            LOG_DEBUG("Data corrupted: invalid sender_peer_id\n");
            return false;
        }
        print_named_data("sender_peer_id", sender_peer_id, sender_peer_id_len);
        if (sender_peer_id_len == 16) {
            uint64_t peer_id = 0;
            for (int shift_by = 56, i = 0; i < sender_peer_id_len; shift_by -= 8, i++) {
                uint8_t uint8 = (sender_peer_id[i] & '@' ? sender_peer_id[i] + 9 : sender_peer_id[i]) << 4;
                i++;
                uint8 |= (sender_peer_id[i] & '@' ? sender_peer_id[i] + 9 : sender_peer_id[i]) & 0xF;
                peer_id |= static_cast<uint64_t>(uint8) << shift_by;
            }
            auto &peer = ble_connection_tracker.checkSenderInPeers(peer_id);
            message.setSenderPeer(&peer);
        }
    }

    if (message.hasMentions()) {
        const auto number_of_mentions = reader.read_uint8();
        for (int i = 0; i < number_of_mentions; i++) {
            const auto mention_len = reader.read_uint8();
            const auto mention = reader.read_data(mention_len);
            if (mention == nullptr) {
                LOG_DEBUG("Data corrupted: invalid mention\n");
                return false;
            }
            print_named_data("mention", mention, mention_len);
            message.addMention(std::string(reinterpret_cast<const char *>(mention), mention_len));
        }
    }

    if (message.hasChannel()) {
        const auto channel_len = reader.read_uint8();
        const auto channel = reader.read_data(channel_len);
        if (channel == nullptr) {
            LOG_DEBUG("Data corrupted: invalid channel\n");
            return false;
        }
        print_named_data("channel", channel, channel_len);
        message.setChannel(std::string(reinterpret_cast<const char *>(channel),
                                       channel_len));
    }

    return true;
}

void ProtocolProcessor::processWrite(BleConnection &connection, const uint16_t offset, const uint8_t *buffer,
                                     const uint16_t buffer_size) const {
    BinaryReader reader(offset, buffer, buffer_size);
    if (const auto version = reader.read_uint8(); version != 1) {
        LOG_DEBUG("Unknown Protocol Version: %d\n", version);
        return;
    }
    const auto type = reader.read_uint8();
    LOG_DEBUG("type: %d (%s)\n", type, stringForType(type));

    const auto ttl = reader.read_uint8();
    LOG_DEBUG("ttl: %d\n", ttl);
    const auto timestamp_ms = reader.read_uint64(); //time in ms since 1970
    LOG_DEBUG("timestamp: 0x%" PRIx64 "\n", timestamp_ms);
    const auto packet_flags = reader.read_uint8();
    LOG_DEBUG("flags: %d\n", packet_flags);
    auto payload_length = reader.read_uint16();
    LOG_DEBUG("payload length: %d\n", payload_length);
    const auto sender = reader.read_uint64();
    LOG_DEBUG("sender: 0x%" PRIx64 "\n", sender);
    uint64_t recipient = 0;
    if (packet_flags & packet_flag_has_recipient) {
        recipient = reader.read_uint64();
        LOG_DEBUG("recipient: 0x%" PRIx64 "\n", recipient);
    }
    uint16_t originalSize = 0;
    if (packet_flags & packet_flag_is_compressed) {
        //compressed gives original size before payload
        originalSize = reader.read_uint16();
        LOG_DEBUG("originalSize: %d\n", originalSize);
        payload_length -= 2;
        //removes the two extra chars added to cover these bytes for simpler clients who don't unpack the payload
    }
    //payload
    auto payload = reader.read_data(payload_length);

    std::vector<uint8_t> decompressed;
    if (payload && packet_flags & packet_flag_is_compressed) {
        decompressed.reserve(originalSize);

        // Decompress using zlib
        size_t decompressedSize;
        const auto d = libdeflate_alloc_decompressor();
        const auto err = libdeflate_zlib_decompress(d, payload, payload_length, decompressed.data(), originalSize,
                                                    &decompressedSize);
        libdeflate_free_decompressor(d);

        if (err == LIBDEFLATE_SUCCESS && decompressedSize > 0) {
            payload = decompressed.data();
            payload_length = decompressedSize;
        }

        if (decompressedSize != static_cast<int>(originalSize) || err != LIBDEFLATE_SUCCESS) {
            LOG_DEBUG("decompressed to something other than the original size - err: %d\n", err);
        }
    } else if (payload) {
        print_named_data("bitchat payload", payload, payload_length);
    } else {
        LOG_DEBUG("payload not readable\n");
        return;
    }

    std::string packet_signature;
    if (packet_flags & packet_flag_has_signature) {
        constexpr uint8_t signature_length = 64;
        uint8_t signature[signature_length];
        memcpy(signature, reader.read_data(signature_length), signature_length);
        print_named_data("bitchat signature", signature, signature_length);
        packet_signature.assign(reinterpret_cast<const char *>(signature), signature_length);
    }

    const auto padding_to_remove = buffer[buffer_size - 1];
    // LOG_DEBUG("padding_to_remove: %d, pos: %d\n", padding_to_remove, reader.test_only_current_pos());
    // if (padding_to_remove < buffer_size) {
    //     print_named_data("bitchat extra data", &buffer[reader.test_only_current_pos()],
    //                      buffer_size - padding_to_remove);
    // }

    switch (type) {
        case noiseIdentityAnnounce: {
            // Usually has a TTL of 0 so we don't pass these along
            std::vector<uint8_t> publicKey(payload_length);
            for (int i = 0; i < payload_length; i++) {
                publicKey.at(i) = payload[i];
            }
            updateOrStorePeerNoisePublicKey(sender, publicKey);
            break;
        }
        case type_message: {
            auto &peer = ble_connection_tracker.checkSenderInPeers(sender);
            if (Message message(ttl, timestamp_ms, packet_flags, sender, recipient, packet_signature);
                processMessage(message, payload, payload_length)) {
                if (const auto stored_message = ble_connection_tracker.storeMessageAndReturnIfNew(message)) {
                    ble_connection_tracker.enqueueBroadcastPacket(stored_message, &connection, &peer);
                }
            }
            break;
        }
        case type_announce: {
            //store a peer as we might need to pass along a message later
            std::string peerName;
            peerName.assign(reinterpret_cast<const char *>(payload), payload_length);
            updateOrStorePeerName(sender, peerName, ttl, connection);
            ble_connection_tracker.possiblyUpdateTimeOffset(timestamp_ms);
            //continue into pass along
        }
        case type_fragment_start:
        case fragmentContinue:
        case fragmentEnd:
        case deliveryAck:
        case deliveryStatusRequest:
        case readReceipt:
        case noiseHandshakeInit:
        case noiseHandshakeResp:
        case noiseEncrypted:
        case channelKeyVerifyRequest:
        case channelKeyVerifyResponse:
        case channelPasswordUpdate:
        case channelMetadata: {
            if (ttl < 1) {
                break;
            }
            //pass along for most types of message
            auto &peer = ble_connection_tracker.checkSenderInPeers(sender);
            std::string payloadString;
            payloadString.assign(reinterpret_cast<const char *>(payload), payload_length);
            PacketPassAlong pass_along(type, ttl, timestamp_ms, packet_flags, sender, recipient,
                                       packet_signature);
            pass_along.setPayload(payloadString);
            if (const auto stored_packet = ble_connection_tracker.storePacketAndReturnIfNew(pass_along)) {
                ble_connection_tracker.enqueueBroadcastPacket(stored_packet, &connection, &peer);
            }
        }

        case handshakeRequest:
        case versionHello:
        case versionAck:
        default:
            break;
    }
}
