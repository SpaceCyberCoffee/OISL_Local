#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <chrono>
#include <thread>

// Simulated network class
class Network {
public:
    bool sendSegment(const std::string &segment, int segmentNumber) {
        // Simulate network sending
        std::cout << "Sending segment #" << segmentNumber << ": " << segment << std::endl;
        return true; // Assume it always succeeds for this example
    }

    bool receiveAck(int segmentNumber) {
        // Simulate receiving an ACK
        std::cout << "Received ACK for segment #" << segmentNumber << std::endl;
        return true;
    }

    void simulateNetworkDelay() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate network delay
    }
};

class CFDP {
public:
    CFDP(Network &network) : network(network) {}

    void sendFile(const std::string &fileContent) {
        int segmentNumber = 0;
        for (const auto &segment : segmentFile(fileContent)) {
            bool sent = false;
            while (!sent) {
                sent = network.sendSegment(segment, segmentNumber);
                network.simulateNetworkDelay();

                if (network.receiveAck(segmentNumber)) {
                    segmentNumber++;
                    sent = true;
                } else {
                    std::cout << "Resending segment #" << segmentNumber << std::endl;
                }
            }
        }
    }

private:
    Network &network;

    std::vector<std::string> segmentFile(const std::string &fileContent) {
        std::vector<std::string> segments;
        int segmentSize = 4; // Example segment size
        for (size_t i = 0; i < fileContent.size(); i += segmentSize) {
            segments.push_back(fileContent.substr(i, segmentSize));
        }
        return segments;
    }
};

int main() {
    Network network;
    CFDP cfdp(network);

    std::string fileContent = "This is a test file for CFDP implementation.";
    cfdp.sendFile(fileContent);

    return 0;
}
