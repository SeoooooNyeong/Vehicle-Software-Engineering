#include <iostream>
#include <fstream>
#include <random>
#include <thread>
#include <chrono>
#include <vector>
#include <sstream>

struct CANMessage {
    uint32_t id;
    uint8_t dlc;
    std::vector<uint8_t> payload;
};

CANMessage generateRandomCANMessage() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint32_t> id_dist(0, 0x1FFFFFFF);
    static std::uniform_int_distribution<uint8_t> dlc_dist(0, 8);
    static std::uniform_int_distribution<uint8_t> payload_dist(0, 255);

    CANMessage msg;
    msg.id = id_dist(gen);
    msg.dlc = dlc_dist(gen);
    msg.payload.resize(msg.dlc);
    for (uint8_t &byte : msg.payload) {
        byte = payload_dist(gen);
    }
    return msg;
}

void logCANMessage(const CANMessage &msg, std::ofstream &logFile) {
    logFile << std::hex << msg.id << " " << std::dec << static_cast<int>(msg.dlc);
    for (uint8_t byte : msg.payload) {
        logFile << " " << std::hex << static_cast<int>(byte);
    }
    logFile << std::endl;
}

void replayCANMessages(const std::string &filename) {
    std::ifstream logFile(filename);
    std::string line;
    while (std::getline(logFile, line)) {
        std::istringstream iss(line);
        uint32_t id;
        int dlc;
        iss >> std::hex >> id >> std::dec >> dlc;
        std::vector<uint8_t> payload(dlc);
        for (int i = 0; i < dlc; ++i) {
            int byte;
            iss >> std::hex >> byte;
            payload[i] = static_cast<uint8_t>(byte);
        }
        // Simulate sending the CAN message (here we just print it)
        std::cout << "Replaying CAN message: ID=" << std::hex << id << " DLC=" << std::dec << dlc << " Payload=";
        for (uint8_t byte : payload) {
            std::cout << std::hex << static_cast<int>(byte) << " ";
        }
        std::cout << std::endl;
        std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(100)); // Delay between messages
    }
}

int main() {
    std::ofstream logFile("can_log.txt");
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file." << std::endl;
        return 1;
    }

    // Generate and log 10 random CAN messages
    for (int i = 0; i < 10; ++i) {
        CANMessage msg = generateRandomCANMessage();
        logCANMessage(msg, logFile);
    }
    logFile.close();

    // Replay the logged CAN messages
    replayCANMessages("can_log.txt");

    return 0;
}