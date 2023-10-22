#include "Muelsyse.h"
#include <stdexcept>

drogon::HttpMethod from_string(const std::string &method)
{
    if (method == "get")
    {
        return drogon::Get;
    }
    if (method == "post")
    {
        return drogon::Post;
    }
    if (method == "put")
    {
        return drogon::Put;
    }
    if (method == "delete")
    {
        return drogon::Delete;
    }
    throw std::runtime_error("Unsupported HttpMethod: " + method);
}

void tl::rpc::Muelsyse::initAndStart(const Json::Value &config)
{
    if (config.isMember("function_list") && config["function_list"].isArray())
    {
        for (const auto &function : config["function_list"])
        {
            std::string name{""};
            if (function.isMember("name") &&
                function["name"].type() == Json::ValueType::stringValue)
            {
                name = function["name"].asString();
            }
            else
            {
                LOG_WARN
                    << "An item in function_list is missing a required item "
                    << "or is in the wrong format: name.";
                continue;
            }
            std::string url{""};
            if (function.isMember("url") &&
                function["url"].type() == Json::ValueType::stringValue)
            {
                url = function["url"].asString();
            }
            else
            {
                LOG_WARN
                    << "An item in function_list is missing a required item "
                    << "or is in the wrong format: url.";
                continue;
            }
            std::string httpMethod{""};
            if (function.isMember("http_method") &&
                function["http_method"].type() == Json::ValueType::stringValue)
            {
                httpMethod = function["http_method"].asString();
            }
            else
            {
                LOG_WARN
                    << "An item in function_list is missing a required item "
                    << "or is in the wrong format: http_method.";
                continue;
            }
            registerRpc(name, url, from_string(httpMethod));
        }
    }
}

void tl::rpc::Muelsyse::shutdown()
{
    /// Shutdown the plugin
}

std::string tl::rpc::Muelsyse::jsonToStringInPath(const Json::Value &json) const
    noexcept(false)
{
    switch (json.type())
    {
        case Json::nullValue:
            return "";
        case Json::intValue:
            return drogon::utils::formattedString("%d", json.asInt());
        case Json::uintValue:
            return drogon::utils::formattedString("%u", json.asUInt());
        case Json::realValue:
            return drogon::utils::formattedString("%lf", json.asDouble());
        case Json::stringValue:
            return json.asString();
        case Json::booleanValue:
            return json.asBool() ? "true" : "false";
        case Json::arrayValue:
        {
            if (json.size() == 0)
                return "";
            std::string result = "";
            for (const auto &item : json)
            {
                result += jsonToStringInPath(item);
                result += ',';
            }
            result.resize(result.size() - 1);
            return result;
        }
            [[unlikely]] default  // objectValue
                : throw std::runtime_error(
                      "JSON object cannot be converted to a string.");
    }
}
