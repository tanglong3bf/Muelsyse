#include "Muelsyse.h"
#include <stdexcept>

using namespace std;
using namespace drogon;
using namespace drogon::utils;
using namespace tl::rest;

HttpMethod from_string(const string &method)
{
    if (method == "get")
    {
        return Get;
    }
    if (method == "post")
    {
        return Post;
    }
    if (method == "put")
    {
        return Put;
    }
    if (method == "delete")
    {
        return Delete;
    }
    throw runtime_error("Unsupported HttpMethod: " + method);
}

void Muelsyse::initAndStart(const Json::Value &config)
{
    if (config.isMember("function_list") && config["function_list"].isArray())
    {
        for (const auto &function : config["function_list"])
        {
            string name{""};
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
            string url{""};
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
            string httpMethod{""};
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
            registerRest(name, url, from_string(httpMethod));
        }
    }
}

void Muelsyse::shutdown()
{
    /// Shutdown the plugin
}

string Muelsyse::jsonToStringInPath(const Json::Value &json) const
    noexcept(false)
{
    switch (json.type())
    {
        case Json::nullValue:
            return "";
        case Json::intValue:
            return formattedString("%d", json.asInt());
        case Json::uintValue:
            return formattedString("%u", json.asUInt());
        case Json::realValue:
            return formattedString("%lf", json.asDouble());
        case Json::stringValue:
            return json.asString();
        case Json::booleanValue:
            return json.asBool() ? "true" : "false";
        case Json::arrayValue:
        {
            if (json.size() == 0)
                return "";
            string result = "";
            for (const auto &item : json)
            {
                result += jsonToStringInPath(item);
                result += ',';
            }
            result.resize(result.size() - 1);
            return result;
        }
            [[unlikely]] default  // objectValue
                : throw runtime_error(
                      "JSON object cannot be converted to a string.");
    }
}
