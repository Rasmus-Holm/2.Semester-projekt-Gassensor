
#include "mqtt/async_client.h"
#include <random>
#include <algorithm>

const std::string DFLT_SERVER_URI(mqtt_host);
const std::string TOPIC(mqtt_sub_topic);

const int QOS = mqtt_sub_QOS;
const int N_RETRY_ATTEMPTS = mqtt_sub_retryAttemps;

// Function generates a semi random client id to avoid blocking from same id
std::string generate_client_id(const std::string &prefix)
{
    const std::string chars = "0123456789abcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, chars.size() - 1);

    std::string id = prefix + "_";
    for (int i = 0; i < 8; ++i)
    {
        id += chars[distribution(generator)];
    }
    std::cout << id << std::endl;
    return id;
}

const std::string CLIENT_ID = generate_client_id("paho_cpp");


void handleMessage_questdb(const mqtt::const_message_ptr msg)
{
    std::string topic = msg->get_topic();
    std::string payload = msg->to_string();
    // std::string payload = msg->get_payload_str(); // to_string() bruger bare get_payload_str()

    // questdb::ingress::table_name_view tableName = questdb_tablename_macro;
    questdb::ingress::utf8_view topic_questdb = topic;   // _utf8
    questdb::ingress::utf8_view value_questdb = payload; // _utf8
    // std::string_view host = "localhost"sv;
    // std::string_view port = "9009"sv;

    if (topic == "bmp_forced/temperature") // unified_sensor har også unified_sensor/temperature
    {
        saveToDatabase_questdb(topic_questdb, value_questdb, "temperature"_tn, questdb_hostname, questdb_port);
    }
    else if (topic == "bmp_forced/pressure")
    {
        saveToDatabase_questdb(topic_questdb, value_questdb, "pressure"_tn, questdb_hostname, questdb_port);
    }
    else if (topic == "unified_sensor/humidity")
    {
        saveToDatabase_questdb(topic_questdb, value_questdb, "humidity"_tn, questdb_hostname, questdb_port);
    }
    else if (topic == "o2sensor/oxygen")
    {
        saveToDatabase_questdb(topic_questdb, value_questdb, "oxygen"_tn, questdb_hostname, questdb_port);
    }
    else if (topic == "log")
    {
        saveToDatabase_questdb(topic_questdb, value_questdb, "log"_tn, questdb_hostname, questdb_port);
    }
    else
    {
        std::cout << "Questdb Unknown topic received: " << topic << std::endl;
        //saveToDatabase_questdb(topic_questdb, value_questdb, "mixed_data_cpp"_tn, questdb_hostname, questdb_port);
    }
}

// Callbacks for the success or failures of requested actions.
// This could be used to initiate further action, but here we just log the
// results to the console.

class action_listener : public virtual mqtt::iaction_listener
{
    std::string name_;

    void on_failure(const mqtt::token &tok) override
    {
        std::cout << name_ << " failure";
        if (tok.get_message_id() != 0)
            std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
        std::cout << std::endl;
    }

    void on_success(const mqtt::token &tok) override
    {
        std::cout << name_ << " success";
        if (tok.get_message_id() != 0)
            std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
        auto top = tok.get_topics();
        if (top && !top->empty())
            std::cout << "\ttoken topic: '" << (*top)[0] << "', ..." << std::endl;
        std::cout << std::endl;
    }

public:
    action_listener(const std::string &name) : name_(name) {}
};

/**
 * Local callback & listener class for use with the client connection.
 * This is primarily intended to receive messages, but it will also monitor
 * the connection to the broker. If the connection is lost, it will attempt
 * to restore the connection and re-subscribe to the topic.
 */

class callback : public virtual mqtt::callback, public virtual mqtt::iaction_listener
{
    // Counter for the number of connection retries
    int nretry_;
    // The MQTT client
    mqtt::async_client &cli_;
    // Options to use if we need to reconnect
    mqtt::connect_options &connOpts_;
    // An action listener to display the result of actions.
    action_listener subListener_;

    // This deomonstrates manually reconnecting to the broker by calling
    // connect() again. This is a possibility for an application that keeps
    // a copy of it's original connect_options, or if the app wants to
    // reconnect with different options.
    // Another way this can be done manually, if using the same options, is
    // to just call the async_client::reconnect() method.
    void reconnect()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(2500));
        try
        {
            cli_.connect(connOpts_, nullptr, *this);
        }
        catch (const mqtt::exception &exc)
        {
            std::cerr << "Error: " << exc.what() << std::endl;
            exit(1);
        }
    }

    // Re-connection failure
    void on_failure(const mqtt::token &tok) override
    {
        std::cout << "Connection attempt failed" << std::endl;
        if (++nretry_ > N_RETRY_ATTEMPTS)
            exit(1);
        reconnect();
    }

    // (Re)connection success
    // Either this or connected() can be used for callbacks.
    void on_success(const mqtt::token &tok) override {}

    // (Re)connection success
    void connected(const std::string &cause) override
    {
        std::cout << "\nConnection success" << std::endl;
        std::cout << "\nSubscribing to topic '" << TOPIC << "'\n"
                  << "\tfor client " << CLIENT_ID << " using QoS" << QOS << "\n"
                  << "\nPress Q<Enter> to quit\n"
                  << std::endl;

        cli_.subscribe(TOPIC, QOS, nullptr, subListener_);
    }

    // Callback for when the connection is lost.
    // This will initiate the attempt to manually reconnect.
    void connection_lost(const std::string &cause) override
    {
        std::cout << "\nConnection lost" << std::endl;
        if (!cause.empty())
            std::cout << "\tcause: " << cause << std::endl;

        std::cout << "Reconnecting..." << std::endl;
        nretry_ = 0;
        reconnect();
    }

    // Callback for when a message arrives.
    void message_arrived(mqtt::const_message_ptr msg) override
    {
        std::cout << "\nMessage arrived" << std::endl;
        std::cout << "\ttopic: '" << msg->get_topic() << "'" << std::endl;
        std::cout << "\tpayload: '" << msg->to_string() << std::endl;

        // ------------------------------------ Callback function -----------------------------------------------------------

        // cout << saveUser(testDb);
        // saveToTempTable(testDb, msg->get_topic(), msg->to_string());


#if questdb_activate
        // example("localhost"sv, "9009"sv);
        handleMessage_questdb(msg);
#endif

        const mqtt::properties &props = msg->get_properties();
        if (size_t n = props.size(); n != 0)
        {
            std::cout << "\tproperties (" << n << "):\n\t  [";
            for (size_t i = 0; i < n - 1; ++i)
                std::cout << props[i] << ", ";
            std::cout << props[n - 1] << "]" << std::endl;
        }
    }

    void delivery_complete(mqtt::delivery_token_ptr token) override {}

public:
    callback(mqtt::async_client &cli, mqtt::connect_options &connOpts)
        : nretry_(0), cli_(cli), connOpts_(connOpts), subListener_("Subscription")
    {
    }
};
