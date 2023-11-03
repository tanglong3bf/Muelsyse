#include "../../src/Muelsyse.h"

#include <gtest/gtest.h>

TEST(MuelsyseTest, ToJsonTest1)
{
    using namespace tl::rpc;
    std::map<std::string, std::string> map{{"aaa", "111"}, {"bbb", "222"}};
    auto json = toJson(map);
    ASSERT_TRUE(json.isMember("aaa"));
    ASSERT_TRUE(json.isMember("bbb"));
    EXPECT_STREQ("111", json["aaa"].asCString());
    EXPECT_STREQ("222", json["bbb"].asCString());
}

TEST(MuelsyseTest, ToJsonTest2)
{
    using namespace tl::rpc;
    std::unordered_map<std::string, std::string> map{{"aaa", "111"},
                                                     {"bbb", "222"}};
    auto json = toJson(map);
    ASSERT_TRUE(json.isMember("aaa"));
    ASSERT_TRUE(json.isMember("bbb"));
    EXPECT_STREQ("111", json["aaa"].asCString());
    EXPECT_STREQ("222", json["bbb"].asCString());
}

TEST(MuelsyseTest, ToJsonTest3)
{
    using namespace tl::rpc;
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

TEST(MuelsyseTest, ToJsonTest4)
{
    using namespace tl::rpc;
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

TEST(MuelsyseTest, ToJsonTest5)
{
    using namespace tl::rpc;
    Hobby hobby;
    auto json = toJson(hobby);
    ASSERT_TRUE(json.isArray());
    ASSERT_EQ(2, json.size());
    EXPECT_STREQ("rubic's cube", json[0].asCString());
    EXPECT_STREQ("blues harmonica", json[1].asCString());
}

TEST(MuelsyseTest, ToJsonTest6)
{
    using namespace tl::rpc;
    auto json = toJson(1);
    ASSERT_TRUE(json.isInt());
    ASSERT_EQ(1, json.asInt());
}

TEST(MuelsyseTest, ArgumentTest1)
{
    using namespace tl::rpc;
    auto json = Argument(std::vector<std::string>{"emm", "aaa"}).toJson();
    ASSERT_TRUE(json.isArray());
    ASSERT_EQ(2, json.size());
    EXPECT_STREQ("emm", json[0].asCString());
    EXPECT_STREQ("aaa", json[1].asCString());
}

inline void test()
{
    RPC_CALL_SYNC(void, "name", "tanglong3bf");
}

TEST(MuelsyseTest, RpcCallSyncTest1)
{
    EXPECT_NO_THROW(test());
}

struct User
{
    void setByJson(const Json::Value &json)
    {
        if (json.isMember("id") && json["id"].isInt())
        {
            id = json["id"].asInt();
        }
        if (json.isMember("username") && json["username"].isString())
        {
            username = json["username"].asString();
        }
        if (json.isMember("password") && json["password"].isString())
        {
            password = json["password"].asString();
        }
    }
    Json::Value toJson() const
    {
        Json::Value json;
        json["id"] = id;
        json["username"] = username;
        json["password"] = password;
        return json;
    }
    int id;
    std::string username;
    std::string password;
};

inline User getUserById(int id)
{
    RPC_CALL_SYNC(User, "_", id);
}

TEST(MuelsyseTest, RpcCallSyncTest2)
{
    User user = getUserById(1);
    EXPECT_EQ(1, user.id);
    EXPECT_STREQ("tanglong3bf", user.username.c_str());
    EXPECT_STREQ("123456", user.password.c_str());
}

inline void vectorParam(const std::vector<int> &ints)
{
    RPC_CALL_SYNC(void, "ints", ints);
}

TEST(MuelsyseTest, RpcCallSyncTest3)
{
    EXPECT_NO_THROW(vectorParam({1, 2, 3, 4, 5}));
}

inline void root(const std::vector<int> &ints)
{
    RPC_CALL_SYNC(void, "", ints);
}

TEST(MuelsyseTest, RpcCallSyncTest4)
{
    EXPECT_NO_THROW(root({1, 2, 3, 4, 5}));
}

inline User complex(int id,
                    std::string name,
                    std::vector<User> userList,
                    std::unordered_map<std::string, std::vector<int>> param)
{
    RPC_CALL_SYNC(
        User, "_", id, "_", name, "user_list", userList, "param", param);
}

TEST(MuelsyseTest, RpcCallSyncTest5)
{
    std::vector<User> userList{{1, "tanglong3bf", "123456"},
                               {2, "Kal'tsit", "654321"},
                               {3, "3.14159", "2653589"}};
    std::unordered_map<std::string, std::vector<int>> param{
        {"aaa", {1, 2, 3}},
        {"bbb", {4, 5, 6}},
        {"ccc", {7, 8, 9}},
    };
    auto user = complex(123, "zhangsan", userList, param);
    EXPECT_EQ(233, user.id);
    EXPECT_STREQ("zhangsan", user.username.c_str());
    EXPECT_STREQ("fawaikuangtu", user.password.c_str());
}

namespace foo::bar
{
RPC_FUNC(Json::Value, funcWithNamespace, int a)
{
    RPC_CALL_SYNC(Json::Value, "a", a);
}
};  // namespace foo::bar

TEST(MuelsyseTest, RpcCallSyncTest6)
{
    auto json = foo::bar::funcWithNamespace(1);
    ASSERT_TRUE(json.isMember("hello"));
    EXPECT_STREQ("world", json["hello"].asCString());
}
