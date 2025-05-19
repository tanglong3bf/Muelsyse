
#include <gtest/gtest.h>
// FRIEND_TEST
#include "../../../src/Muelsyse.h"

drogon::HttpMethod fromString(const std::string &method);

TEST(HttpMethodFromStringTest, All)
{
    EXPECT_EQ(drogon::HttpMethod::Get, fromString("get"));
    EXPECT_EQ(drogon::HttpMethod::Post, fromString("post"));
    EXPECT_EQ(drogon::HttpMethod::Put, fromString("put"));
    EXPECT_EQ(drogon::HttpMethod::Delete, fromString("delete"));
    EXPECT_THROW(fromString("head"), std::invalid_argument);
}

class MuelsyseTest : public tl::rest::Muelsyse
{
  public:
    void initAndStart(const Json::Value &config)
    {
        tl::rest::Muelsyse::initAndStart(config);
    }

    void shutdown()
    {
        tl::rest::Muelsyse::shutdown();
    }

    std::string jsonToStringInPath(const Json::Value &json) const
    {
        return tl::rest::Muelsyse::jsonToStringInPath(json);
    }

    std::tuple<drogon::HttpClientPtr, drogon::HttpRequestPtr> prepare(
        const std::string &url,
        std::vector<tl::rest::Argument> &&args = {}) const
    {
        return tl::rest::Muelsyse::prepare(url, std::move(args));
    }
};

TEST(JsonToStringInPathTest, All)
{
    MuelsyseTest muelsyse;
    EXPECT_STREQ("", muelsyse.jsonToStringInPath(Json::nullValue).c_str());
    EXPECT_STREQ("1", muelsyse.jsonToStringInPath(1).c_str());
    EXPECT_STREQ("1", muelsyse.jsonToStringInPath(1u).c_str());
    EXPECT_STREQ("1.000000", muelsyse.jsonToStringInPath(1.).c_str());
    EXPECT_STREQ("Muelsyse", muelsyse.jsonToStringInPath("Muelsyse").c_str());
    EXPECT_STREQ("true", muelsyse.jsonToStringInPath(true).c_str());
    EXPECT_STREQ("false", muelsyse.jsonToStringInPath(false).c_str());
    Json::Value array(Json::arrayValue);
    EXPECT_STREQ("", muelsyse.jsonToStringInPath(array).c_str());
    array.append(1);
    array.append(2);
    array.append(3);
    array.append(4);
    array.append(5);
    EXPECT_STREQ("1,2,3,4,5", muelsyse.jsonToStringInPath(array).c_str());
    Json::Value object;
    object["name"] = "Muelsyse";
    EXPECT_THROW(muelsyse.jsonToStringInPath(object), std::invalid_argument);
}

TEST(ToJsonTest, Int)
{
    using namespace tl::rest;
    auto json = toJson(1);
    ASSERT_TRUE(json.isInt());
    ASSERT_EQ(1, json.asInt());
}

TEST(ToJsonTest, Map)
{
    using namespace tl::rest;
    std::map<std::string, std::string> map{{"aaa", "111"}, {"bbb", "222"}};
    auto json = toJson(map);
    ASSERT_TRUE(json.isMember("aaa"));
    ASSERT_TRUE(json.isMember("bbb"));
    EXPECT_STREQ("111", json["aaa"].asCString());
    EXPECT_STREQ("222", json["bbb"].asCString());
}

TEST(ToJsonTest, UMap)
{
    using namespace tl::rest;
    std::unordered_map<std::string, std::string> map{{"aaa", "111"},
                                                     {"bbb", "222"}};
    auto json = toJson(map);
    ASSERT_TRUE(json.isMember("aaa"));
    ASSERT_TRUE(json.isMember("bbb"));
    EXPECT_STREQ("111", json["aaa"].asCString());
    EXPECT_STREQ("222", json["bbb"].asCString());
}

TEST(ToJsonTest, VectorInt)
{
    using namespace tl::rest;
    std::vector<int> list{12, 23};
    auto json = toJson(list);
    ASSERT_TRUE(json.isArray());
    ASSERT_EQ(2, json.size());
    EXPECT_EQ(12, json[0].asInt());
    EXPECT_EQ(23, json[1].asInt());
}

struct Name
{
    std::string toString() const
    {
        return "tanglong";
    }
};

TEST(ToJsonTest, HasToString)
{
    using namespace tl::rest;
    Name name;
    auto json = toJson(name);
    ASSERT_TRUE(json.isString());
    EXPECT_STREQ("tanglong", json.asCString());
}

struct Hobby
{
    Json::Value toJson() const
    {
        Json::Value json;
        json.append("rubic's cube");
        json.append("blues harmonica");
        return json;
    }
};

TEST(ToJsonTest, HasToJson)
{
    using namespace tl::rest;
    Hobby hobby;
    auto json = toJson(hobby);
    ASSERT_TRUE(json.isArray());
    ASSERT_EQ(2, json.size());
    EXPECT_STREQ("rubic's cube", json[0].asCString());
    EXPECT_STREQ("blues harmonica", json[1].asCString());
}

TEST(ArgumentTest, All)
{
    using namespace tl::rest;
    auto json = Argument(std::vector<std::string>{"emm", "aaa"}).toJson();
    ASSERT_TRUE(json.isArray());
    ASSERT_EQ(2, json.size());
    EXPECT_STREQ("emm", json[0].asCString());
    EXPECT_STREQ("aaa", json[1].asCString());
}

TEST(PrepareTest, All)
{
    using namespace tl::rest;
    MuelsyseTest muelsyse;
    auto config = drogon::app().getCustomConfig();
    muelsyse.initAndStart(config);
    EXPECT_THROW(muelsyse.prepare("inexistent"), std::invalid_argument);
    EXPECT_THROW(muelsyse.prepare("testWithoutProtocol", {1, 1}),
                 std::invalid_argument);

    EXPECT_THROW(muelsyse.prepare("testWithoutProtocol", {"_", "path_param"}),
                 std::invalid_argument);

    Json::Value json;
    json["name"] = "Muelsyse";

    EXPECT_THROW(muelsyse.prepare("testWithoutProtocol",
                                  {"extra", "param", "", json}),
                 std::invalid_argument);

    auto [client, request] =
        muelsyse.prepare("test",
                         {"", json, "_", "path_param", "extra", "param"});
    EXPECT_NE(nullptr, client);
    // {"extra":"param","name":"Muelsyse"}
    auto requestBody = request->jsonObject();
    EXPECT_STREQ("Muelsyse", (*requestBody)["name"].asCString());
    EXPECT_STREQ("param", (*requestBody)["extra"].asCString());

    EXPECT_THROW(muelsyse.prepare("testWithErrorBrace", {"_", "path_param"}),
                 std::invalid_argument);
}

namespace test::sync
{

struct User
{
    void setByJson(const Json::Value &json)
    {
        id = json["id"].asInt();
        username = json["username"].asString();
        password = json["password"].asString();
    }

    int id;
    std::string username;
    std::string password;
};

REST_FUNC_SYNC(void, test, const std::string &name)
{
    REST_CALL_SYNC(void, NAMED_PARAM("name", name));
}

REST_FUNC_SYNC(Json::Value, respIsNotJson)
{
    REST_CALL_SYNC(Json::Value);
}

REST_FUNC_SYNC(Json::Value, jsonResp, int id)
{
    REST_CALL_SYNC(Json::Value, PATH_PARAM(id));
}

REST_FUNC_SYNC(User, getUserById, int id)
{
    REST_CALL_SYNC(User, PATH_PARAM(id));
}

TEST(SyncTest, Void)
{
    EXPECT_NO_THROW(test("tanglong3bf"));
}

TEST(SyncTest, NotJson)
{
    EXPECT_THROW(respIsNotJson(), std::runtime_error);
}

TEST(SyncTest, Json)
{
    auto user = jsonResp(1);
    EXPECT_EQ(1, user["id"].asInt());
    EXPECT_STREQ("tanglong3bf", user["username"].asCString());
    EXPECT_STREQ("123456", user["password"].asCString());
}

TEST(SyncTest, SetByJson)
{
    auto user = getUserById(1);
    EXPECT_EQ(1, user.id);
    EXPECT_STREQ("tanglong3bf", user.username.c_str());
    EXPECT_STREQ("123456", user.password.c_str());
}

}  // namespace test::sync

namespace test::async
{

struct User
{
    void setByJson(const Json::Value &json)
    {
        id = json["id"].asInt();
        username = json["username"].asString();
        password = json["password"].asString();
    }

    int id;
    std::string username;
    std::string password;
};

REST_FUNC_ASYNC(void, test, const std::string &name)
{
    REST_CALL_ASYNC(void, NAMED_PARAM("name", name));
}

REST_FUNC_ASYNC(Json::Value, respIsNotJson)
{
    REST_CALL_ASYNC(Json::Value);
}

REST_FUNC_ASYNC(Json::Value, jsonResp, int id)
{
    REST_CALL_ASYNC(Json::Value, PATH_PARAM(id));
}

REST_FUNC_ASYNC(User, getUserById, int id)
{
    REST_CALL_ASYNC(User, PATH_PARAM(id));
}

TEST(AsyncTest, Void)
{
    using namespace std::chrono_literals;
    {
        std::mutex mtx;
        std::condition_variable cv;
        bool ready = false;
        test(
            "tanglong3bf",
            [&]() {
                std::unique_lock<std::mutex> lock(mtx);
                ready = true;
                cv.notify_one();
            },
            [&](const std::exception &e) {
                std::unique_lock<std::mutex> lock(mtx);
                LOG_ERROR << e.what();
                cv.notify_one();
            });
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait_for(lock, 5s, [&]() { return ready; });
    }

    {
        std::mutex mtx;
        std::condition_variable cv;
        bool ready = false;
        test(
            "tanglong3bf",
            [&]() {
                std::unique_lock<std::mutex> lock(mtx);
                throw std::runtime_error("test exception");
                ready = true;
                cv.notify_one();
            },
            [&](const std::exception &e) {
                std::unique_lock<std::mutex> lock(mtx);
                LOG_ERROR << e.what();
                ready = true;
                cv.notify_one();
            });
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait_for(lock, 5s, [&]() { return ready; });
    }
}

TEST(AsyncTest, NotJson)
{
    using namespace std::chrono_literals;
    std::mutex mtx;
    std::condition_variable cv;
    bool ready = false;
    bool success = false;
    respIsNotJson(
        [&](Json::Value) {
            std::unique_lock<std::mutex> lock(mtx);
            ready = true;
            success = true;
            cv.notify_one();
        },
        [&](const std::exception &e) {
            std::unique_lock<std::mutex> lock(mtx);
            LOG_ERROR << e.what();
            ready = true;
            success = false;
            cv.notify_one();
        });
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait_for(lock, 5s, [&]() { return ready; });
    EXPECT_FALSE(success);
}

TEST(AsyncTest, Json)
{
    using namespace std::chrono_literals;
    std::mutex mtx;
    std::condition_variable cv;
    Json::Value json;
    bool ready = false;
    jsonResp(
        1,
        [&](Json::Value result) {
            std::unique_lock<std::mutex> lock(mtx);
            json = result;
            ready = true;
            cv.notify_one();
        },
        [&](const std::exception &e) {
            std::unique_lock<std::mutex> lock(mtx);
            LOG_ERROR << e.what();
            cv.notify_one();
        });
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait_for(lock, 5s, [&]() { return ready; });
    EXPECT_EQ(1, json["id"].asInt());
    EXPECT_STREQ("tanglong3bf", json["username"].asCString());
    EXPECT_STREQ("123456", json["password"].asCString());
}

TEST(AsyncTest, SetByJson)
{
    using namespace std::chrono_literals;
    std::mutex mtx;
    std::condition_variable cv;
    User user;
    bool ready = false;
    getUserById(
        1,
        [&](User result) {
            std::unique_lock<std::mutex> lock(mtx);
            user = result;
            ready = true;
            cv.notify_one();
        },
        [&](const std::exception &e) {
            std::unique_lock<std::mutex> lock(mtx);
            LOG_ERROR << e.what();
            cv.notify_one();
        });
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait_for(lock, 5s, [&]() { return ready; });
    EXPECT_EQ(1, user.id);
    EXPECT_STREQ("tanglong3bf", user.username.c_str());
    EXPECT_STREQ("123456", user.password.c_str());
}

}  // namespace test::async

namespace test::future
{
REST_FUNC_FUTURE(void, test)
{
    REST_CALL_FUTURE(void);
}

REST_FUNC_FUTURE(Json::Value, jsonResp, int id)
{
    REST_CALL_FUTURE(Json::Value, PATH_PARAM(id));
}

TEST(FutureTest, Void)
{
    auto future = test();
    future.wait();
    future.get();
}

TEST(FutureTest, Json)
{
    auto future = jsonResp(1);
    auto result = future.get();
    EXPECT_EQ(1, result["id"].asInt());
    EXPECT_STREQ("tanglong3bf", result["username"].asCString());
    EXPECT_STREQ("123456", result["password"].asCString());
}

}  // namespace test::future
