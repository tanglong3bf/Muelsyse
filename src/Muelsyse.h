/**
 * @file Muelsyse.h
 * @brief A plugin to simplify RPC for drogon projects.
 *
 * This project only supports synchronous interface calls and supports a small
 * number of data types. Using C++20 syntax, such as concepts, I don’t like
 * SNIFAE
 *
 * @author tanglong3bf
 * @date 2023-10-20
 * @version v0.0.1
 */

#pragma once

#include <concepts>

#include <drogon/HttpAppFramework.h>
#include <drogon/HttpClient.h>

/**
 * @brief Custom functions can call this macro to simplify development.
 * Please see tl::tpc::Muelsyse::rpcCallSync<T>() for specific parameters.
 */
#define RPC_CALL_SYNC(ret_type, ...)                                          \
    auto rpcCaller = drogon::app().getPlugin<tl::rpc::Muelsyse>();            \
    if constexpr (std::is_void_v<ret_type>)                                   \
    {                                                                         \
        rpcCaller->rpcCallSync<ret_type>(__FUNCTION__, {__VA_ARGS__});        \
    }                                                                         \
    else                                                                      \
    {                                                                         \
        return rpcCaller->rpcCallSync<ret_type>(__FUNCTION__, {__VA_ARGS__}); \
    }

namespace tl::rpc
{

template <typename T>
Json::Value toJson(const T &param)
{
    return param;
}

template <typename T>
concept HasToString = requires(T t) {
    {
        t.toString()
    } -> std::same_as<std::string>;
};

template <typename T>
concept HasToJson = requires(T t) {
    {
        t.toJson()
    } -> std::same_as<Json::Value>;
};

Json::Value toJson(const HasToString auto &param)
{
    return param.toString();
}

Json::Value toJson(const HasToJson auto &param)
{
    return param.toJson();
}

template <typename T>
concept HasToStringAndToJson = HasToString<T> && HasToJson<T>;

Json::Value toJson(const HasToStringAndToJson auto &param)
{
    return param.toJson();
}

template <typename T>
Json::Value toJson(const std::unordered_map<std::string, T> &param);
template <typename T>
Json::Value toJson(const std::vector<T> &param);

template <typename T>
Json::Value toJson(const std::map<std::string, T> &param)
{
    Json::Value result;
    for (const auto &[key, value] : param)
    {
        result[key] = toJson(value);
    }
    return result;
}

template <typename T>
Json::Value toJson(const std::unordered_map<std::string, T> &param)
{
    Json::Value result;
    for (const auto &[key, value] : param)
    {
        result[key] = toJson(value);
    }
    return result;
}

template <typename T>
Json::Value toJson(const std::vector<T> &param)
{
    Json::Value result;
    for (const auto &item : param)
    {
        result.append(toJson(item));
    }
    return result;
}

/**
 * @brief The parameters of functions
 *
 * Supported parameter types: basic data types, strings, Json::Value,
 * unordered_map<string, T>, map<string, T>, classes with toString() or toJson()
 *
 * @see toJson
 */
class Argument
{
  public:
    template <typename T>
    Argument(const T &data)
    {
        data_ = ::tl::rpc::toJson(data);
    }
    const Json::Value &toJson() const
    {
        return data_;
    }

  private:
    Json::Value data_;
};

/**
 * @brief The main part of plugin
 *
 * You can configure the function name, uri and HttpMethod in the configuration
 * file. See README.md for details.
 *
 * The `registerRpc` function is called in the `initAndStart` function to
 * register each function.
 *
 * Each function does not need to be implemented by yourself. Use the
 * RPC_CALL_SYNC macro inside the function to automatically call rpcCallSync.
 *
 * In the README.md file, there are instructions and examples for using the
 * RPC_CALL_SYNC macro.
 */
class Muelsyse : public drogon::Plugin<Muelsyse>
{
  public:
    Muelsyse()
    {
    }
    /// This method must be called by drogon to initialize and start the plugin.
    /// It must be implemented by the user.
    void initAndStart(const Json::Value &config) override;

    /// This method must be called by drogon to shutdown the plugin.
    /// It must be implemented by the user.
    void shutdown() override;

    /**
     * @brief Register the url and HttpMethod of a function
     *
     * @param [in] func_name The name of a function.
     * @param [in] url request url
     * @param [in] httpMethod request method
     */
    void registerRpc(const std::string &func_name,
                     const std::string &url,
                     drogon::HttpMethod httpMethod)
    {
        rpcMap_[func_name] = std::make_pair(url, httpMethod);
    }

    /**
     * @brief convert json to string
     *
     * {} -> ""
     *
     * [] -> ""
     *
     * [1] -> "1"
     *
     * [1, 2, 3] -> "1, 2, 3"
     *
     * true -> "true"
     *
     * false -> "false"
     *
     * {"key": "value"} -> throw runtime_error
     */
    std::string jsonToStringInPath(const Json::Value &json) const
        noexcept(false);

    /**
     * @brief Calling remote procedures synchronously
     *
     * This function processes all parameters, and then calls the function with
     * the same name.
     *
     * The args parameters are in a group of two. The first item of each group
     * needs to be a string, and the second item is the actual parameter that
     * needs to be passed.
     *
     * When the first item is "_", it means the second item is a dynamic path
     * parameter. It needs to have a `toJson()` or `toString()` member function.
     * `toJson()` is used first.
     *
     * When the first item is "", it means that the second item will be placed
     * at the root of the request body. However, if the request body already has
     * data before placing this item, an exception will be thrown.
     *
     * When the first item is a normal string, it means that the second item
     * will be placed in a sub-item of the request body.
     * @attention
     * suggest: use RPC_CALL_SYNC to call this function.
     */
    template <typename T>
    T rpcCallSync(const std::string &funcName,
                  const std::vector<Argument> &args) const noexcept(false);

    /**
     * @brief Calling remote procedures synchronously
     * @attention
     * suggest: use RPC_CALL_SYNC to call this function.
     */
    template <typename T>
    T rpcCallSync(std::string url,
                  drogon::HttpMethod httpMethod,
                  const Json::Value &requestBody) const noexcept(false);

  private:
    std::unordered_map<std::string, std::pair<std::string, drogon::HttpMethod>>
        rpcMap_;
};

template <typename T>
T Muelsyse::rpcCallSync(const std::string &funcName,
                        const std::vector<Argument> &args) const noexcept(false)
{
    if (this->rpcMap_.find(funcName) != rpcMap_.end())
    {
        auto [url, httpMethod] = rpcMap_.at(funcName);

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
                throw std::runtime_error(
                    "Incorrect parameter configuration of " + funcName);
            }
            std::string arg = args[i].toJson().asString();
            // path parameter
            if (arg == "_")
            {
                // Find the first {} in the url and replace it
                int startPos = url.find("{");
                if (startPos != std::string::npos)
                {
                    int endPos = url.find("}", startPos);
                    Json::Value json = args[i + 1].toJson();
                    url.replace(startPos,
                                endPos - startPos + 1,
                                jsonToStringInPath(json));
                }
            }
            // root parameter
            else if (arg == "")
            {
                if (!requestBody.empty())
                {
                    // If the request body already has data, the new parameter
                    // should not be placed as root
                    throw std::runtime_error(
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

        if constexpr (std::is_void_v<T>)
        {
            rpcCallSync<void>(url, httpMethod, requestBody);
            return;
        }
        else
        {
            return rpcCallSync<T>(url, httpMethod, requestBody);
        }
    }
    if constexpr (!std::is_void_v<T>)
    {
        return {};
    }
}

template <typename T>
T Muelsyse::rpcCallSync(std::string url,
                        drogon::HttpMethod httpMethod,
                        const Json::Value &requestBody) const noexcept(false)
{
    if (!url.starts_with("http://") && !url.starts_with("https://"))
    {
        url = "http://" + url;
    }

    int pos = 7;
    pos += (url[pos] == '/');

    std::string path;
    if ((pos = url.find('/', pos)) != std::string::npos)
    {
        path = url.substr(pos);
        url.resize(pos);
    }

    auto httpClient = drogon::HttpClient::newHttpClient(url);
    auto req = drogon::HttpRequest::newHttpJsonRequest(requestBody);
    req->setPath(path);
    req->setMethod(httpMethod);
    auto [result, resp] = httpClient->sendRequest(req);
    if (result == drogon::ReqResult::Ok)
    {
        if constexpr (std::is_void_v<T>)
        {
            return;
        }
        else if constexpr (std::is_same_v<T, Json::Value>)
        {
            auto jsonPtr = resp->getJsonObject();
            if (jsonPtr == nullptr)
            {
                throw std::runtime_error("response body is not json.");
            }
            return *jsonPtr;
        }
        else
        {
            T res;
            auto jsonPtr = resp->getJsonObject();
            if (jsonPtr == nullptr)
            {
                throw std::runtime_error("response body is not json.");
            }
            res.setByJson(*jsonPtr);
            return res;
        }
    }
    else
    {
        LOG_ERROR << result;
        throw std::runtime_error(
            "The request failed. It may be a network problem or a "
            "configuration error");
    }
}

}  // namespace tl::rpc
