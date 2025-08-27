#pragma once

enum PacketType {
    type_unknown = 0,
    type_announce = 0x01,
    type_leave = 0x03,
    type_message = 0x04, // All user messages (private and broadcast)
    type_fragment_start = 0x05,
    fragmentContinue = 0x06,
    fragmentEnd = 0x07,
    channelAnnounce = 0x08u, // Announce password-protected channel status
    channelRetention = 0x09u, // Announce channel retention status
    deliveryAck = 0x0A, // Acknowledge message received
    deliveryStatusRequest = 0x0B, // Request delivery status update
    readReceipt = 0x0C, // Message has been read/viewed

    // Noise Protocol messages
    noiseHandshakeInit = 0x10, // Noise handshake initiation
    noiseHandshakeResp = 0x11, // Noise handshake response
    noiseEncrypted = 0x12, // Noise encrypted transport message
    noiseIdentityAnnounce = 0x13, // Announce static public key for discovery

    channelKeyVerifyRequest = 0x14u, // Request key verification for a channel
    channelKeyVerifyResponse = 0x15u, // Response to key verification request
    channelPasswordUpdate = 0x16u, // Distribute new password to channel members
    channelMetadata = 0x17u, // Announce channel creator and metadata

    // Protocol version negotiation
    versionHello = 0x20, // Initial version announcement
    versionAck = 0x21, // Version acknowledgment

    // Protocol-level acknowledgments
    protocolAck = 0x22, // Generic protocol acknowledgment
    protocolNack = 0x23, // Negative acknowledgment (failure)
    systemValidation = 0x24, // Session validation ping
    handshakeRequest = 0x25, // Request handshake for pending messages

    // Favorite system messages
    favorited = 0x30, // Peer favorited us
    unfavorited = 0x31 // Peer unfavorited us
};

enum PacketFlag {
    packet_flag_has_recipient = 0x01,
    packet_flag_has_signature = 0x02,
    packet_flag_is_compressed = 0x04
};
