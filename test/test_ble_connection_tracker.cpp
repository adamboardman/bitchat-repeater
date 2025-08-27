/**
* SPDX-FileCopyrightText: 2025, Adam Boardman
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <catch2/catch_test_macros.hpp>
#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <vector>

#include "pico_pi_mocks.h"
#include "../Bitchat/BinaryReader.h"
#include "../Bitchat/BinaryWriter.h"
#include "../Bitchat/ProtocolProcessor.h"

const uint8_t uint_array1[] = {
    0x01, 0x01, 0x03, 0x00, 0x00, 0x01, 0x98, 0x71, 0x83, 0xcd, 0xf9, 0x00, 0x00, 0x04, 0x1d, 0x3d, 0x6a, 0x26, 0x15,
    0x23, 0xa8, 0x28, 0x61, 0x64, 0x61, 0x6d, 0x00
};
const std::string data2 =
        "01010300000198717c111400000419077f0222faf5ce6164616de89f19e772b12aea88b2450fbd9c76161304d656dbf817366830b44031e6264714089006b960e6168f5eba7964bf0be4875057b678a4a076314723884b2ef2ae66a23476df2bea4fe370304ed0053e80c61b2dc3b354970dda20282bd5dbd3decd0618d3d8cdc9667a41f538453d0385570018e5f60872eb89e1b7917ae7bc41e3bdae1e70f28b06924f51f38a19535096f9cc66969343f068147c514d3ec0e6400a96833ccb22b5ed3ed225f76efe74e14fecfd54c00612dc2d90f7f1d96930348547eb47dbda537ea95e712dc39377d4304103a01abe8cbbf21e8674b53654989a";
const std::string data3 =
        "01130100000198717c111405004b19077f0222faf5ceffffffffffffffff009c7801639064af6752faf5f51c83c2eefd752fec1205f7059806e6c9a5fdbb77c4885571ebf650de7dd3632d6d0fc67a1056c192989298cbc0c038a3b0465084c18181420000f12524171faedad93d658acf4f0107ea080387f17180794fad21cc6207951f2df239ecad83639ebe60992ade12b4ebdf9b46a4369d44d884f46a0ee57e2868ede3356fe5aafe423b9ea639fd674fa09bf42d88fa94c7f9b3c880b3d24c2ac5f9f3a809a41f16c73615bfe434e2f1bfbd168eec040b496b1402d6c6359a82be2c9ae4d6ae49f6a9cb0f6889483dbfd659aa74b70428f572";
const std::string data4 =
        "01130100000198717c110705004b19077f0222faf5ceffffffffffffffff009c7801639064af6752faf5f51c83c2eefd752fec1205f7059806e6c9a5fdbb77c4885571ebf650de7dd3632d6d0fc67a1056c192989298cbc0c038a3b046908dc18181420000ed7b240994fd85a4429eb67b4ecb7c76da3fb5b6986f3f8f9d10708e57427ea9f1eb2cfbb32d17cb3755db5a872a6b8648f334b51423ffbb46dffcf7044e0212dc480c9b194c4202f1d1b82b2ab00ce36c721d37297f9685b0412fb0bef1b3d9503cc4527a96d4fccf460abe8debfe2265e95da5163b49cafc932b045e3217564f14d0e42d23e80264a4f031f9b4139864e2fc4b6f2ec0";
const std::string data5 =
        "0104070000019893abb14c01004feddd326fb00c2b40ffffffffffffffff100000019893abb14b2431454645413636352d313837382d343644312d393538452d46314535353731413833383008616e6f6e32303134000568656c6c6f1065646464333236666230306332623430151bdfade3b1eec297f58a83862efcf098857199715a2de8a28de951489319c101adce8799dee33409c120137e272b60522f6354c1af553b2949b7125d62d849ca69dc7f419dd3e880cd5b57ec6b718241015e75974563ba00a33c8556ef62fe2f266f16e3bde1d313c6209c53549ede95dd8635eb10326bebaca5b1d9f3a406c110ade6bff69f17417ae182436a89";
const std::string data6 =
        "01040700000198941571c301004bc67ff7caf2952326ffffffffffffffff1000000198941571c12434453337323032392d323035422d344644432d413844362d343736333437303436423944046164616d000568656c6c6f10633637666637636166323935323332368359aac1085a908e03d66c8ec83517ce63f5c202f174ae2e802d6539a7cc7ae3fbfb30dc8e34b188aa87f439d721d4b80adbd61013c7b112c8a3a121e96269e90d82dfe9a700bc4129cfc0a4b257fc94ae0a389066d2747a1f4ff10c0bd9b6aaa9a0365c84b0e18e388f2b0b647616deed3fec91085c49daf96aa7393cc94d039133bfcdbe9ec7b823149b26fb257e929aab60";
const std::string noise_encrypted1 =
    "01120700000198d284504b010114e74da1a856b690936ff9f65a6858d8ff000000006ee544eddd2b060d5b628002aed065491fc8b706ccbfafc518aedba00f4bf4d807d79cbcd1d32b8bc1fcb280781a22a2d6ea4814ed32dab76258a74cc695fb79fb520f1f020df4014888cb63d47c25de594c1ca4d76be437a87dec399c12c6d5473d2f44981d88e6f196c91292ecde587b7f2187c129b77b9947b0a48526c201b297204a9d8123f90e4abaa011aef35a93483957634a01d57b495f5dcbeb86d772eaa326575383c15482ddbfd84ce429d6f4390544fc56d55a3c966cecbf76c4837c3078fca2c5258ee7446987d10716b0c6c3233aad4b3f4d17";
const std::string noise_encrypted2 =
    "01120700000198d35e50ee0100861a4d912f6a99af5e6ff9f65a6858d8ff00000000a2973fc96e96acb6176fa9b4aa81f65b44d96e0d0c6956a1a22ab637ed4e2587d63f6490bac66e3b4b08ea6317574d6797132c28cf6b2f2587e62746f7db631a7410eca33be8d35ed6d20cdb56fde7b10bbe1387b274dd02df3c8708f2288943ed3aa0c5a9b14900ecfe87a02c4ef6fce1e849353a8d2a7032ca33ce1666a2ab672200";
BleConnectionTracker *connection_tracker_ptr=nullptr;

TEST_CASE("ProtocolProcessorAnnounce", "[data2]") {
    const uint8_t data_len = data2.length() / 2;
    uint8_t uint_array[data_len];
    populate_array_from_string(uint_array, data2);

    BleConnectionTracker tracker;
    const ProtocolProcessor processor(tracker);
    BleConnection connection;
    processor.processWrite(connection, 0, uint_array, sizeof(uint_array));

    const auto peer = tracker.peerWithId(0x19077f0222faf5ce);
    REQUIRE(0x19077f0222faf5ce == peer->getId());

    REQUIRE("adam" == peer->getName());

    tracker.printStats();
    tracker.cleanupStaleItems();
    tracker.printStats();
}

TEST_CASE("ProtocolProcessorNoiseAnnounce", "[data4]") {
    const uint8_t datalen = data4.length() / 2;
    uint8_t uint_array[datalen];
    populate_array_from_string(uint_array, data4);

    BleConnectionTracker tracker;
    ProtocolProcessor processor(tracker);
    BleConnection connection;
    processor.processWrite(connection, 0, uint_array, sizeof(uint_array));

    const auto peer = tracker.peerWithId(0x19077f0222faf5ce);
    REQUIRE(0x19077f0222faf5ce == peer->getId());

    REQUIRE("" == peer->getName());

    uint8_t publicKey[] = {
        0, 25, 7, 0x7F, 2, '"', 0xFA, 0xF5, 0xCE, 0, ' ', 0xBB, 0xBF, '~', 0xE8, '>', 'a', 17, 0xBE, 'P', '5', 'Q', 'n',
        30, 'f', 0xFE, 0xDE, 0xC4, '2', 5, '!', 0xB5, 0xB7, 'U', '\r', 0xBE, 0x97, ']', '9', '=', 0xC1, ']', 'H', 0,
        ' ', 0xBB, 0xBF, '~', 0xE8, '>', 'a', 17, 0xBE, 'P', '5', 'Q', 'n', 30, 'f', 0xFE, 0xDE, 0xC4, '2', 5, '!',
        0xB5, 0xB7, 'U', '\r', 0xBE, 0x97, ']', '9', '=', 0xC1, ']', 'H', 4, 'a', 'd', 'a', 'm', 0, 0, 1, 0x98, 'q',
        '|', 17, 6, 0, '@', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    auto equal = std::equal(std::begin(publicKey), std::end(publicKey), std::begin(peer->getPublicKey()));
    REQUIRE(equal);
}

TEST_CASE("ProtocolProcessorMessageAnon2014", "[data5]") {
    const uint8_t data_len = data5.length() / 2;
    uint8_t uint_array[data_len];
    populate_array_from_string(uint_array, data5);

    BleConnectionTracker tracker;
    connection_tracker_ptr = &tracker;
    const ProtocolProcessor processor(tracker);
    BleConnection connection;
    processor.processWrite(connection, 0, uint_array, sizeof(uint_array));

    auto message = tracker.messageWithId("1EFEA665-1878-46D1-958E-F1E5571A8380");
    REQUIRE("1EFEA665-1878-46D1-958E-F1E5571A8380" == message->getMessageId());

    REQUIRE("anon2014" == message->getSenderNickname());
}


TEST_CASE("ProtocolProcessorMessageEmpty", "[data5]") {
    const uint8_t data_len = data5.length() / 2;
    uint8_t uint_array[data_len];
    populate_array_from_string(uint_array, data5);

    BleConnectionTracker tracker;
    connection_tracker_ptr = &tracker;
    ProtocolProcessor processor(tracker);
    BleConnection connection;
    processor.processWrite(connection, 0, uint_array, sizeof(uint_array));

    const auto message = tracker.messageWithId("4E372029-205B-4FDC-A8D6-476347046B9D");
    REQUIRE(nullptr == message);
}

TEST_CASE("ProtocolProcessorMessageAdam", "[data6]") {
    const uint8_t data_len = data6.length() / 2;
    uint8_t uint_array[data_len];
    populate_array_from_string(uint_array, data6);

    BleConnectionTracker tracker;
    connection_tracker_ptr = &tracker;
    ProtocolProcessor processor(tracker);
    BleConnection &connectionTo = tracker.connectionForConnHandle(1);
    connectionTo.setConnected(true);
    connectionTo.setBitchatCharacteristicValueHandle(1);
    connectionTo.setMtu(517);

    BleConnection &connectionFrom = tracker.connectionForConnHandle(2);
    connectionFrom.setConnected(true);
    connectionFrom.setBitchatCharacteristicValueHandle(1);
    connectionFrom.setMtu(517);

    processor.processWrite(connectionFrom, 0, uint_array, sizeof(uint_array));

    auto message = tracker.messageWithId("4E372029-205B-4FDC-A8D6-476347046B9D");
    REQUIRE("4E372029-205B-4FDC-A8D6-476347046B9D" == message->getMessageId());

    REQUIRE("adam" == message->getSenderNickname());
    REQUIRE("hello" == message->getContent());
    REQUIRE(0xc67ff7caf2952326 == message->getSenderPeer()->getId());

    Peer &newPeerToSendTo = tracker.checkSenderInPeers(0x23789453);
    REQUIRE(0x23789453 == newPeerToSendTo.getId());
    reset_sent_for_test();
    tracker.sendPackets();

    REQUIRE(106 == mock_sent_data.size());
    std::vector<uint8_t> expected_data(uint_array, uint_array + mock_sent_data.size());
    //just compare the same size we wrote as we know recorded messages have lots of crap at the end
    expected_data[2] = message->getPacketTtl() - 1; // ttl should have been reduced
    if (mock_sent_data[mock_sent_data.size() - 1] == 0) {
        expected_data[mock_sent_data.size() - 1] = 0; //we know that the saved copy will not have a zero here
    }
    print_named_data("mock_sent_data", mock_sent_data.data(), mock_sent_data.size());
    print_named_data("expected_data ", expected_data.data(), expected_data.size());
    auto eq = std::equal(std::begin(mock_sent_data), std::end(mock_sent_data), std::begin(expected_data));
    REQUIRE(true == eq);

    set_mock_time(1000*1000*60*15);
    connectionFrom.setConnected(false);

    REQUIRE(2 == tracker.getConnectionsCount());
    tracker.cleanupStaleItems();
    REQUIRE(1 == tracker.getConnectionsCount());
}

TEST_CASE("TimeOffsetSet", "[time1]") {
    BleConnectionTracker tracker;
    connection_tracker_ptr = &tracker;
    const auto old_now = tracker.getTimeMs();
    uint64_t timestamp = 0x198c702ff54 / 1000;
    tracker.possiblyUpdateTimeOffset(timestamp);
    auto new_now = tracker.getTimeMs();
    REQUIRE(0 == old_now);
    REQUIRE(1755685519 == new_now);

    timestamp += 1000;
    tracker.possiblyUpdateTimeOffset(timestamp);
    new_now = tracker.getTimeMs();
    REQUIRE(0 == old_now);
    REQUIRE(1755686519 == new_now);
}

TEST_CASE("AnnounceToConnections", "[ann1]") {
    BleConnectionTracker tracker;
    connection_tracker_ptr = &tracker;
    tracker.possiblyUpdateTimeOffset(1755685519+40);
    const bd_addr_t addr{};
    tracker.reportConnection(1,addr,BD_ADDR_TYPE_LE_RANDOM);
    BleConnection &connection = tracker.connectionForConnHandle(1);
    connection.setBitchatCharacteristicValueHandle(1);
    connection.setMtu(517);

    tracker.announceToConnections();

    REQUIRE(tracker.getTargetedPacketsToSendSize() == 1);

    tracker.sendPackets();

    tracker.announceToConnections();
    REQUIRE(tracker.getTargetedPacketsToSendSize() == 0);

    set_mock_time(1000*1000*60*15);
    connection.setConnected(false);

    REQUIRE(1 == tracker.getConnectionsCount());
    tracker.cleanupStaleItems();
    REQUIRE(0 == tracker.getConnectionsCount());
}

TEST_CASE("NoiseEncryptedMessageBustedMTU", "[Enc1]") {
    const uint8_t data_len = noise_encrypted1.length() / 2;
    uint8_t uint_array[data_len];
    populate_array_from_string(uint_array, noise_encrypted1);

    BleConnectionTracker tracker;
    connection_tracker_ptr = &tracker;
    const ProtocolProcessor processor(tracker);
    BleConnection &connectionFrom = tracker.connectionForConnHandle(3);
    connectionFrom.setConnected(true);
    connectionFrom.setBitchatCharacteristicValueHandle(7);
    processor.processWrite(connectionFrom, 0, uint_array, sizeof(uint_array));

    const auto packet = tracker.getAnyPacket();
    REQUIRE(nullptr == packet); // expect broken packet
}

TEST_CASE("NoiseEncryptedMessageSmaller", "[Enc2]") {
    const uint8_t data_len = noise_encrypted2.length() / 2;
    uint8_t uint_array[data_len];
    populate_array_from_string(uint_array, noise_encrypted2);

    BleConnectionTracker tracker;
    connection_tracker_ptr = &tracker;
    ProtocolProcessor processor(tracker);
    BleConnection &connectionFrom = tracker.connectionForConnHandle(3);
    connectionFrom.setConnected(true);
    connectionFrom.setBitchatCharacteristicValueHandle(7);
    processor.processWrite(connectionFrom, 0, uint_array, sizeof(uint_array));

    auto packet = tracker.getAnyPacket();
    REQUIRE(nullptr != packet);
    REQUIRE(18 == packet->getPacketType());
    REQUIRE(7 == packet->getPacketTtl());
    REQUIRE(1 == packet->getPacketFlags());
    REQUIRE(0x198d35e50ee == packet->getPacketTimestamp());
    REQUIRE(0x1a4d912f6a99af5e == packet->getPacketSenderId());
    REQUIRE(0x6ff9f65a6858d8ff == packet->getPacketRecipientId());

    auto pass_along = reinterpret_cast<PacketPassAlong*>(packet);
    REQUIRE(nullptr != pass_along);
}

TEST_CASE("AnnounceOnlyOnce","[Ann1]") {
    BleConnectionTracker tracker = {};
    connection_tracker_ptr = &tracker;
    const uint64_t timestamp = 0x198c702ff54 / 1000;
    tracker.possiblyUpdateTimeOffset(timestamp);

    BleConnection &connection1 = tracker.connectionForConnHandle(3);
    connection1.setConnected(true);
    connection1.setBitchatCharacteristicValueHandle(7);
    connection1.setMtu(517);

    tracker.announceToConnections();

    reset_sent_for_test();
    tracker.sendPackets();

    REQUIRE(35 == mock_sent_data.size());

    BleConnection &connection2 = tracker.connectionForConnHandle(5);
    connection2.setConnected(true);
    connection2.setBitchatCharacteristicValueHandle(7);
    connection2.setMtu(517);

    tracker.announceToConnections();

    reset_sent_for_test();
    tracker.sendPackets();

    REQUIRE(35 == mock_sent_data.size());

    set_mock_time(1000*1000*60*15);
}

TEST_CASE("AnnounceOnlyOnceNoSendCleanup","[Ann2]") {
    BleConnectionTracker tracker;
    connection_tracker_ptr = &tracker;
    constexpr uint64_t timestamp = 0x198c702ff54 / 1000;
    tracker.possiblyUpdateTimeOffset(timestamp);

    BleConnection &connection1 = tracker.connectionForConnHandle(3);
    connection1.setConnected(true);
    connection1.setBitchatCharacteristicValueHandle(7);
    connection1.setMtu(517);

    tracker.announceToConnections();

    reset_sent_for_test();
    REQUIRE(0 == mock_sent_data.size());

    BleConnection &connection2 = tracker.connectionForConnHandle(5);
    connection2.setConnected(true);
    connection2.setBitchatCharacteristicValueHandle(7);
    connection2.setMtu(517);

    tracker.announceToConnections();

    REQUIRE(0 == mock_sent_data.size());

    set_mock_time(timestamp + 1000*1000*60*15);

    REQUIRE(3 == tracker.getTargetedPacketsToSendSize());
    tracker.cleanupStaleItems();
    REQUIRE(0 == tracker.getTargetedPacketsToSendSize());

}