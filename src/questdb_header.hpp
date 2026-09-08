#include <questdb/ingress/line_sender.hpp>
#include <iostream>

using namespace std::literals::string_view_literals;
using namespace questdb::ingress::literals;

/**
 * Generic function to save 4-column data (id, topic, value, timestamp)
 * to a specified MariaDB table.
 */
bool saveToDatabase_questdb(questdb::ingress::utf8_view topic = "none/none"_utf8, questdb::ingress::utf8_view value = "NULLstr"_utf8, questdb::ingress::table_name_view tableName = "testdb"_tn, std::string_view host = "localhost"sv, std::string_view port = "9009"sv)
{
    try
    {
        auto sender = questdb::ingress::line_sender::from_conf(
            "tcp::addr=" + std::string{host} + ":" + std::string{port} +
            ";protocol_version=2;");

        // We prepare all our table names and column names in advance.
        // If we're inserting multiple rows, this allows us to avoid
        // re-validating the same strings over and over again.
        const auto table_name = tableName;
        const auto topic_name = "topic"_cn;
        const auto value_name = "value"_cn;

        questdb::ingress::line_sender_buffer buffer = sender.new_buffer();
        buffer.table(table_name)
            .symbol(topic_name, topic)
            .symbol(value_name, value)
            .at(questdb::ingress::timestamp_nanos::now());

        // To insert more records, call `buffer.table(..)...` again.

        sender.flush(buffer);

        // It's recommended to keep a timer and/or maximum buffer size to flush
        // the buffer periodically with any accumulated records.

        std::cout << "Saved to questdb" << std::endl;

        return true;
    }
    catch (const questdb::ingress::line_sender_error &err)
    {
        std::cerr << "Error running saveToDatabase_questdb: " << err.what() << std::endl;

        return false;
    }
}