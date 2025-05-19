#include "Muelsyse.h"
#include <stdexcept>

using namespace std;
using namespace drogon;
using namespace drogon::utils;
using namespace tl::rest;

/**
 * @date 2025-04-27
 * @since v0.0.1
 */
HttpMethod fromString(const string &method)
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
    throw invalid_argument("Unsupported HttpMethod: " + method);
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
                    << "or is in the wrong format: "
                    << function.toStyledString();
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
                    << "or is in the wrong format: "
                    << function.toStyledString();
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
                    << "or is in the wrong format: "
                    << function.toStyledString();
                continue;
            }
            registerRest(name, url, fromString(httpMethod));
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
                : throw invalid_argument(
                      "JSON object cannot be converted to a string.");
    }
}

std::tuple<drogon::HttpClientPtr, drogon::HttpRequestPtr> tl::rest::Muelsyse::
    prepare(const std::string &funcName,
            const std::vector<Argument> &args) const
{
    assert(args.size() % 2 == 0);
    if (this->restMap_.find(funcName) == restMap_.end())
    {
        throw std::invalid_argument("rest function not found: " + funcName);
    }

    auto [url, httpMethod] = restMap_.at(funcName);
    if (!url.starts_with("http://") && !url.starts_with("https://"))
    {
        url = "http://" + url;
    }

    Json::Value requestBody(Json::objectValue);
    // parameter processing
    for (int i = 0; i < args.size(); i += 2)
    {
        if (!args[i].toJson().isString())
        {
            throw std::invalid_argument(
                "Incorrect parameter configuration of " + funcName);
        }
        std::string arg = args[i].toJson().asString();
        // path parameter
        if (arg == "_")
        {
            // Find the first {} in the url and replace it
            int startPos = url.find("{");
            if (startPos == std::string::npos)
            {
                throw std::invalid_argument(
                    "Incorrect parameter configuration of " + funcName);
            }
            int endPos = url.find("}", startPos);
            if (endPos == std::string::npos)
            {
                throw std::invalid_argument(
                    "Incorrect parameter configuration of " + funcName);
            }
            Json::Value json = args[i + 1].toJson();
            url.replace(startPos,
                        endPos - startPos + 1,
                        jsonToStringInPath(json));
        }
        // root parameter
        else if (arg == "")
        {
            if (!requestBody.empty())
            {
                // If the request body already has data, the new parameter
                // should not be placed as root
                throw std::invalid_argument(
                    "Incorrect parameter configuration of " + funcName);
            }
            requestBody = args[i + 1].toJson();
        }
        // request body parameter
        else
        {
            requestBody[arg] = args[i + 1].toJson();
        }
    }
    int pos = 7;
    pos += (url[pos] == '/');

    std::string path = "/";
    if ((pos = url.find('/', pos)) != std::string::npos)
    {
        path = url.substr(pos);
        url.resize(pos);
    }
    auto httpClient = getHttpClient(url);
    auto req = drogon::HttpRequest::newHttpJsonRequest(requestBody);
    req->setPath(path);
    req->setMethod(httpMethod);
    return {httpClient, req};
}

HttpClientPtr Muelsyse::getHttpClient(const string &url) const
{
    auto &mtx = getMapMutex();
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto iter = httpClientMap_.find(url);
        if (iter != httpClientMap_.end())
            return iter->second;
    }
    auto newHttpClient = HttpClient::newHttpClient(url);

    std::lock_guard<std::mutex> lock(mtx);
    auto ret =
        httpClientMap_.emplace(std::make_pair(url, std::move(newHttpClient)));
    return ret.first->second;
}

void Muelsyse::restCallAsync(
    const std::string &funcName,
    const std::vector<Argument> &args,
    std::function<void()> successCallback,
    std::function<void(const std::exception &)> errorCallback) const
{
    auto [httpClient, req] = prepare(funcName, args);
    httpClient->sendRequest(
        req,
        [successCallback, errorCallback](drogon::ReqResult result,
                                         const drogon::HttpResponsePtr &resp) {
            if (result == drogon::ReqResult::Ok)
            {
                try
                {
                    successCallback();
                }
                catch (const std::exception &e)
                {
                    if (errorCallback)
                    {
                        errorCallback(e);
                    }
                }
            }
            else if (errorCallback)
            {
                errorCallback(std::runtime_error(
                    "The request failed. It may be a network problem "
                    "or a configuration error"));
            }
        });
}
