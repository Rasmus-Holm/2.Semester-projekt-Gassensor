// ------------------ UPDATE MAKEFILE when switching branches ------------------
#define questdb_activate 1

// værdier til questdb ---------------- QuestDB uses UTC timestamp!!!! ---------------
// "localhost"sv // "127.0.0.1"sv
#define questdb_hostname "localhost"sv
#define questdb_port "9009"sv

// Værdier til mqtt // "mqtt://localhost:1883" // "mqtt://192.168.87.130:1883"
#define mqtt_host "mqtt://localhost:1883"
#define mqtt_sub_topic "#"
#define mqtt_sub_QOS 1
#define mqtt_sub_retryAttemps 5

#include <string>
#include <iostream>

#include "questdb_header.hpp"
#include "mqtt_header.hpp"


int main(int argc, char *argv[])
{
    // A subscriber often wants the server to remember its messages when its
    // disconnected. In that case, it needs a unique ClientID and a
    // non-clean session.

    auto serverURI = (argc > 1) ? std::string{argv[1]} : DFLT_SERVER_URI; // TODO check

    mqtt::async_client cli(serverURI, CLIENT_ID);

    auto connOpts = mqtt::connect_options_builder::v5().clean_start(true).finalize();

    // Install the callback(s) before connecting.
    callback cb(cli, connOpts);
    cli.set_callback(cb);

    // Start the connection.
    // When completed, the callback will subscribe to topic.

    try
    {
        std::cout << "Connecting to the MQTT server '" << serverURI << "'..." << std::flush;
        cli.connect(connOpts, nullptr, cb);
    }
    catch (const mqtt::exception &exc)
    {
        std::cerr << "\nERROR: Unable to connect to MQTT server: '" << serverURI << "'" << exc
                  << std::endl;
        return 1;
    }

    // #include <chrono>
    // #include <thread>

    while (1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    /*     while (std::tolower(std::cin.get()) != 'q')
        {
        } */

    // Disconnect

    try
    {
        std::cout << "\nDisconnecting from the MQTT server..." << std::flush;
        cli.disconnect()->wait();
        std::cout << "OK" << std::endl;
    }
    catch (const mqtt::exception &exc)
    {
        std::cerr << exc << std::endl;
        return 1;
    }

    return 0;
}
