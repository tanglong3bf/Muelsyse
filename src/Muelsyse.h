/**
 * @file Muelsyse.h
 * @brief Muelsyse is a drogon plugin used to simplify HTTP client operations.
 *
 * This project offers three distinct methods for invoking third-party HTTP
 * services: synchronous calls, callback-based asynchronous calls, and
 * future-based asynchronous calls. Using C++20 syntax, such as concepts, I
 * don’t like SNIFAE.
 *
 * @author tanglong3bf
 * @date 2025-05-18
 * @version v0.4.0
 */

#pragma once

#include <drogon/HttpAppFramework.h>
#include <drogon/HttpClient.h>

/**
 * @brief Normal functions DO NOT have the classTypeName() member function.
 * Please use the REST_FUNC macro to declare functor.
 *
 * @date 2025-04-27
 * @since v0.1.0
 *
 */
inline std::string classTypeName() noexcept(false)
{
    throw std::runtime_error(
        "Normal functions DO NOT have member function classTypeName(). "
        "Please use REST_FUNC to declare functor.");
}

/**
 * @brief Define a functor for synchronous HTTP requests.
 *
 * @param ret_type The return type of the functor.
 * @param func_name The name of the functor.
 * @param ... The parameter list for the functor.
 *
 * @date 2025-04-27
 * @since v0.1.0
 */
#define REST_FUNC(ret_type, func_name, ...)               \
    struct func_name : public drogon::DrObject<func_name> \
    {                                                     \
        ret_type operator()(__VA_ARGS__) const;           \
    } static func_name;                                   \
    inline ret_type func_name::operator()(__VA_ARGS__) const

/**
 * @brief Similar to REST_FUNC, used to declare synchronous functions.
 *
 * @date 2025-05-18
 * @version v0.4.0
 */
#define REST_FUNC_SYNC(...) REST_FUNC(__VA_ARGS__)

/**
 * @brief Define a functor for callback-based asynchronous HTTP requests.
 *
 * @param ret_type The return type of the functor.
 * @param func_name The name of the functor.
 * @param ... The parameter list for the functor, followed by success and
 * error callbacks.
 *
 * @date 2025-05-18
 * @version v0.4.0
 */
#define REST_FUNC_ASYNC(ret_type, func_name, ...)                            \
    struct func_name : public drogon::DrObject<func_name>                    \
    {                                                                        \
        void operator()(__VA_ARGS__ __VA_OPT__(, )                           \
                            std::function<void(ret_type)> &&successCallback, \
                        std::function<void(const std::exception &)>          \
                            &&errorCallback) const;                          \
    } static func_name;                                                      \
    inline void func_name::operator()(                                       \
        __VA_ARGS__ __VA_OPT__(, )                                           \
            std::function<void(ret_type)> &&successCallback,                 \
        std::function<void(const std::exception &)> &&errorCallback) const

/**
 * @brief Define a functor for future-based asynchronous HTTP requests.
 *
 * @param ret_type The return type of the functor.
 * @param func_name The name of the functor.
 * @param ... The parameter list for the functor.
 *
 * @date 2025-05-18
 * @version v0.4.0
 */
#define REST_FUNC_FUTURE(ret_type, func_name, ...)           \
    struct func_name : public drogon::DrObject<func_name>    \
    {                                                        \
        std::future<ret_type> operator()(__VA_ARGS__) const; \
    } static func_name;                                      \
    inline std::future<ret_type> func_name::operator()(__VA_ARGS__) const

/**
 * @addtogroup param_macros
 * @{
 * @date 2025-05-18
 * @since 0.4.0
 */

/// Define a path parameter
#define PATH_PARAM(value) "_", value
/// Use the parameter as the request body
#define ROOT_PARAM(value) "", value
/// Define an additional attribute in the request body
#define NAMED_PARAM(name, value) name, value

/** @} */

/**
 * @brief Custom functions can call this macro to simplify synchronous HTTP
 * request development.
 *
 * @see restCallSync<T>()
 *
 * @date 2025-04-29
 * @since v0.0.1
 */
#define REST_CALL_SYNC(ret_type, ...)                                \
    auto restCaller = drogon::app().getPlugin<tl::rest::Muelsyse>(); \
    std::string func_name{""};                                       \
    try                                                              \
    {                                                                \
        func_name = classTypeName();                                 \
    }                                                                \
    catch (const std::runtime_error &e)                              \
    {                                                                \
        func_name = __FUNCTION__;                                    \
    }                                                                \
    return restCaller->restCallSync<ret_type>(func_name, {__VA_ARGS__})

template <typename Ret>
struct RestCallWrapper
{
    template <typename... Args>
    static void invoke(auto &&restCaller, Args &&...args)
    {
        restCaller->template restCallAsync<Ret>(std::forward<Args>(args)...);
    }
};

/// Specialization of RestCallWrapper for void return type.
template <>
struct RestCallWrapper<void>
{
    template <typename... Args>
    static void invoke(auto &&restCaller, Args &&...args)
    {
        restCaller->restCallAsync(std::forward<Args>(args)...);
    }
};

/**
 * @brief Custom functions can call this macro to simplify callback-based
 * asynchronous HTTP request development.
 * @see tl::tpc::Muelsyse::restCallAsync<T>()
 * @see tl::tpc::Muelsyse::restCallAsync()
 *
 * @date 2025-05-18
 * @since 0.4.0
 */
#define REST_CALL_ASYNC(ret_type, ...)                                 \
    auto restCaller = drogon::app().getPlugin<tl::rest::Muelsyse>();   \
    std::string func_name{""};                                         \
    try                                                                \
    {                                                                  \
        func_name = classTypeName();                                   \
    }                                                                  \
    catch (const std::runtime_error &e)                                \
    {                                                                  \
        func_name = __FUNCTION__;                                      \
    }                                                                  \
    RestCallWrapper<ret_type>::invoke(restCaller,                      \
                                      func_name,                       \
                                      std::vector<tl::rest::Argument>{ \
                                          __VA_ARGS__},                \
                                      successCallback,                 \
                                      errorCallback)

/**
 * @brief Custom functions can call this macro to simplify future-based
 * asynchronous HTTP request development.
 * @see tl::tpc::Muelsyse::restCallFuture<T>()
 *
 * @date 2025-05-18
 * @since 0.4.0
 */
#define REST_CALL_FUTURE(ret_type, ...)                              \
    auto restCaller = drogon::app().getPlugin<tl::rest::Muelsyse>(); \
    std::string func_name{""};                                       \
    try                                                              \
    {                                                                \
        func_name = classTypeName();                                 \
    }                                                                \
    catch (const std::runtime_error &e)                              \
    {                                                                \
        func_name = __FUNCTION__;                                    \
    }                                                                \
    return restCaller->restCallFuture<ret_type>(func_name, {__VA_ARGS__})

namespace tl::rest
{

/**
 * @addtogroup toJson
 * @{
 * @date 2023-10-20
 * @since v0.0.1
 */

/// Convert the parameter to Json::Value
template <typename T>
Json::Value toJson(const T &param)
{
    return param;
}

/// Concept to check if the T type has a toString() member function
template <typename T>
concept HasToString = requires(T t) {
    {
        t.toString()
    } -> std::same_as<std::string>;
};

/// Concept to check if the T type has a toJson() member function
template <typename T>
concept HasToJson = requires(T t) {
    {
        t.toJson()
    } -> std::same_as<Json::Value>;
};

/// Convert a type with a toString() member function to Json::Value
Json::Value toJson(const HasToString auto &param)
{
    return param.toString();
}

/// Convert a type with a toJson() member function to Json::Value
Json::Value toJson(const HasToJson auto &param)
{
    return param.toJson();
}

/// Concept to check if T type has both toString() and toJson() member functions
template <typename T>
concept HasToStringAndToJson = HasToString<T> && HasToJson<T>;

/// For types that have both toString() and toJson() member functions,
/// prioritize using toJson().
Json::Value toJson(const HasToStringAndToJson auto &param)
{
    return param.toJson();
}

template <typename T>
Json::Value toJson(const std::unordered_map<std::string, T> &param);

template <typename T>
Json::Value toJson(const std::vector<T> &param);

/// Convert std::map<std::string, T> to Json::Value
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

/// Convert std::unordered_map<std::string, T> to Json::Value
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

/// Convert std::vector<T> to Json::Value
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

/// @}

/**
 * @brief The parameters of functions
 *
 * Supported parameter types: basic data types, strings, Json::Value,
 * unordered_map<string, T>, map<string, T>, classes with toString() or toJson()
 *
 * @see toJson
 *
 * @date 2025-04-27
 * @since v0.0.1
 */
class Argument
{
  public:
    /// Convert the incoming data to Json::Value for storage
    template <typename T>
    Argument(const T &data)
    {
        data_ = tl::rest::toJson(data);
    }

    /// Retrieve the stored Json::Value.
    const Json::Value &toJson() const
    {
        return data_;
    }

  private:
    Json::Value data_;
};

/**
 * @brief The main class of the Muelsyse plugin.
 *
 * You can configure the function name, URI, and HttpMethod in the configuration
 * file. Refer to README.md for detailed configuration instructions.
 *
 * The `registerRest` function is called during the `initAndStart` function to
 * register each function.
 *
 * Each function does not need to be implemented manually. Instead, use the
 * REST_CALL_SYNC macro within the function to automatically invoke
 * restCallSync.
 *
 * @see REST_FUNC_SYNC
 * @see REST_FUNC_ASYNC
 * @see REST_FUNC_FUTURE
 * @see REST_CALL_SYNC
 * @see REST_CALL_ASYNC
 * @see REST_CALL_FUTURE
 *
 * @date 2025-04-27
 * @since v0.0.1
 */
class Muelsyse : public drogon::Plugin<Muelsyse>
{
  public:
    Muelsyse()
    {
    }

    /**
     * @date 2025-05-18
     * @since 0.4.0
     */
    void initAndStart(const Json::Value &config) override;

    void shutdown() override;

    /**
     * @brief Send HTTP requests synchronously.
     *
     * @param funcName The name of the function or functor.
     * @param args The parameters of the function or functor.
     * @return The response of the HTTP request.
     *
     * @attention
     * It is recommended to use REST_CALL_SYNC to invoke this function.
     *
     * @date 2025-05-18
     * @since 0.0.1
     */
    template <typename T>
    T restCallSync(const std::string &funcName,
                   const std::vector<Argument> &args) const noexcept(false);

    /**
     * @brief Send HTTP requests asynchronously using a callback mechanism.
     *
     * @param funcName The name of the function or functor.
     * @param args The parameters for the function or functor.
     * @param successCallback The callback function to handle successful
     * responses.
     * @param errorCallback The callback function to handle error responses
     * (optional).
     *
     * @attention
     * It is recommended to use REST_CALL_ASYNC to invoke this function.
     *
     * @date 2025-05-18
     * @since 0.4.0
     */
    template <typename T>
    void restCallAsync(const std::string &funcName,
                       const std::vector<Argument> &args,
                       std::function<void(T)> successCallback,
                       std::function<void(const std::exception &)>
                           errorCallback = nullptr) const;

    /**
     * @brief Send HTTP requests asynchronously using a callback mechanism.
     *
     * @param funcName The name of the function or functor.
     * @param args The parameters for the function or functor.
     * @param successCallback The callback function to handle successful
     * responses.
     * @param errorCallback The callback function to handle error responses
     * (optional).
     *
     * @attention
     * It is recommended to use REST_CALL_ASYNC to invoke this function.
     *
     * @date 2025-05-18
     * @since 0.4.0
     */
    void restCallAsync(const std::string &funcName,
                       const std::vector<Argument> &args,
                       std::function<void()> successCallback,
                       std::function<void(const std::exception &)>
                           errorCallback = nullptr) const;

    /**
     * @brief Send HTTP requests asynchronously using a future mechanism.
     *
     * @param funcName The name of the function or functor.
     * @param args The parameters for the function or functor.
     * @return A future object that can be used to retrieve the result of the
     * HTTP request.
     *
     * @attention
     * It is recommended to use REST_CALL_FUTURE to invoke this function.
     *
     * @date 2025-05-18
     * @since 0.4.0
     */
    template <typename T>
    std::future<T> restCallFuture(const std::string &funcName,
                                  const std::vector<Argument> &args) const
        noexcept(false);

  protected:
    /**
     * @brief Register the url and HttpMethod of a function
     *
     * @param func_name The name of the function.
     * @param url The request url
     * @param httpMethod The HTTP method for the request.
     *
     * @date 2025-04-27
     * @since 0.0.1
     */
    void registerRest(const std::string &func_name,
                      const std::string &url,
                      drogon::HttpMethod httpMethod)
    {
        restMap_[func_name] = std::make_pair(url, httpMethod);
    }

    /**
     * @brief Convert a Json::Value to a string suitable for inclusion in a URL
     * path.
     *
     * - `{}` -> `""`
     * - `[]` -> `""`
     * - `[1]` -> `"1"`
     * - `[1, 2, 3]` -> `"1, 2, 3"`
     * - `true` -> `"true"`
     * - `false` -> `"false"`
     * - `{"key": "value"}` -> Throws std::runtime_error
     *
     * @param json The Json::Value to convert.
     * @return The converted string.
     *
     * @date 2025-05-18
     * @since 0.0.1
     */
    std::string jsonToStringInPath(const Json::Value &json) const
        noexcept(false);

    /**
     * @brief Prepare parameters for the HTTP request.
     *
     * Processes all parameters provided to the request.
     *
     * The args parameter is a vector of pairs. The first element of each pair
     * needs to be a string, and the second element is the actual parameter to
     * pass.
     *
     * When the first element is "_", it indicates a dynamic path parameter,
     * which should have either a `toJson()` or `toString()` member function.
     * `toJson()` is given priority.
     *
     * When the first element is "", it indicates that the second element should
     * be placed at the root of the request body. If the request body already
     * contains data, an exception will be thrown.
     *
     * When the first element is a normal string, it indicates that the second
     * element should be placed in a sub-object within the request body.
     *
     * @see PATH_PARAM
     * @see ROOT_PARAM
     * @see NAMED_PARAM
     * @see getHttpClient
     *
     * @date 2025-05-18
     * @since 0.4.0
     */
    std::tuple<drogon::HttpClientPtr, drogon::HttpRequestPtr> prepare(
        const std::string &funcName,
        const std::vector<Argument> &args = {}) const;

    /**
     * @brief Retrieve the HttpClient object for the specified URL.
     *
     * @param url The URL for the HTTP request.
     * @return The HttpClient object associated with the specified URL.
     *
     * @date 2025-04-29
     * @since 0.3.0
     */
    drogon::HttpClientPtr getHttpClient(const std::string &url) const;

    /**
     * @brief Retrieve the mutex for the restMap_ member variable.
     *
     * @return A reference to the mutex for the restMap_ member variable.
     *
     * @date 2025-04-29
     * @since 0.3.0
     */
    static std::mutex &getMapMutex()
    {
        static std::mutex mtx;
        return mtx;
    }

  private:
    std::unordered_map<std::string, std::pair<std::string, drogon::HttpMethod>>
        restMap_;
    mutable std::unordered_map<std::string, drogon::HttpClientPtr>
        httpClientMap_;
};

template <typename T>
T Muelsyse::restCallSync(const std::string &funcName,
                         const std::vector<Argument> &args) const
    noexcept(false)
{
    auto [httpClient, req] = prepare(funcName, args);

    auto [result, resp] = httpClient->sendRequest(req);
    if (result == drogon::ReqResult::Ok)
    {
        if constexpr (std::is_void_v<T>)
        {
            return;
        }
        else
        {
            auto jsonPtr = resp->getJsonObject();
            if (jsonPtr == nullptr)
            {
                throw std::runtime_error("response body is not json.");
            }
            if constexpr (std::is_same_v<T, Json::Value>)
            {
                return *jsonPtr;
            }
            else
            {
                T res;
                res.setByJson(*jsonPtr);
                return res;
            }
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

template <typename T>
void Muelsyse::restCallAsync(
    const std::string &funcName,
    const std::vector<Argument> &args,
    std::function<void(T)> successCallback,
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
                    auto jsonPtr = resp->getJsonObject();
                    if (jsonPtr == nullptr)
                    {
                        throw std::runtime_error("response body is not json.");
                    }
                    if constexpr (std::is_same_v<T, Json::Value>)
                    {
                        successCallback(*jsonPtr);
                    }
                    else
                    {
                        T res;
                        res.setByJson(*jsonPtr);
                        successCallback(res);
                    }
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

template <typename T>
std::future<T> Muelsyse::restCallFuture(const std::string &funcName,
                                        const std::vector<Argument> &args) const
    noexcept(false)
{
    auto promisePtr = std::make_shared<std::promise<T>>();

    if constexpr (std::is_void_v<T>)
    {
        std::future<void> future = promisePtr->get_future();

        restCallAsync(
            funcName,
            args,
            [promisePtr]() mutable { promisePtr->set_value(); },
            [promisePtr](const std::exception &e) mutable {
                promisePtr->set_exception(std::make_exception_ptr(e));
            });
        return future;
    }
    else
    {
        std::future<T> future = promisePtr->get_future();

        restCallAsync<T>(
            funcName,
            args,
            [promisePtr](T result) mutable { promisePtr->set_value(result); },
            [promisePtr](const std::exception &e) mutable {
                promisePtr->set_exception(std::make_exception_ptr(e));
            });
        return future;
    }
}

}  // namespace tl::rest
