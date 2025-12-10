#include <iostream>
#include <mqtt/client.h>

const std::string SERVER_ADDRESS = "tcp://dellxps13-master:1883";
const std::string CLIENT_ID = "OdroidAudioNodeClient";
const std::string USERNAME = "ppaudionode";
const std::string PASSWORD = "audio";
const std::string TOPIC = "test/topic";
const std::string PAYLOAD = "Hello from C++ MQTT!";

// Simple class to handle incoming messages
class action_listener : public virtual mqtt::callback {
public:
    void connection_lost(const std::string& cause) override {
        std::cout << "\nConnection lost: " << (cause.empty() ? "No Reason" : cause) << std::endl;
    }

    void message_arrived(mqtt::const_message_ptr msg) override {
        std::cout << "------------------------------------------" << std::endl;
        std::cout << "TOPIC: " << msg->get_topic() << std::endl;
        std::cout << "PAYLOAD: " << msg->to_string() << std::endl;
        std::cout << "QoS: " << msg->get_qos() << std::endl;
        std::cout << "------------------------------------------" << std::endl;
    }

    void delivery_complete(mqtt::delivery_token_ptr tok) override {
        // Not used for subscribers, but required by interface
    }
};

int main(int argc, char* argv[]) {
    mqtt::client client(SERVER_ADDRESS, CLIENT_ID);
    mqtt::connect_options connOpts;

    // Set Authentication Options
    connOpts.set_user_name(USERNAME);
    connOpts.set_password(PASSWORD);

    // Set the callback handler
    action_listener listener;
    client.set_callback(listener);

    try {
        std::cout << "Connecting to secured broker at " << SERVER_ADDRESS << "..." << std::endl;
        client.connect(connOpts);
        std::cout << "Connected. Client ID: " << CLIENT_ID << std::endl;

        // Subscribe to the test topic
        std::cout << "Subscribing to topic: " << TOPIC << std::endl;
        client.subscribe(TOPIC, 1);

        std::cout << "Successfully subscribed. Waiting for messages..." << std::endl;
        std::cout << "Press Ctrl+C to exit." << std::endl;

        // Keep the main thread alive to allow the client to receive messages
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

    } catch (const mqtt::exception& exc) {
        std::cerr << "MQTT Error: " << exc.what() << std::endl;
        return 1;
    }

    return 0;
}