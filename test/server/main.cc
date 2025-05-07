#include <drogon/drogon.h>

using namespace drogon;

int main(int argc, char* argv[])
{
    app().registerHandler(
        "/test",
        [](const HttpRequestPtr& req,
           std::function<void(const HttpResponsePtr&)>&& callback) {
            auto jsonPtr = req->jsonObject();
            assert(jsonPtr);
            const auto& json = *jsonPtr;
            assert(json.isMember("name"));
            assert(json["name"].isString());
            assert(json["name"].asString() == "tanglong3bf");
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k200OK);
            callback(resp);
        },
        {Post});

    app().registerHandler(
        "/user/{user_id}",
        [](const HttpRequestPtr& req,
           std::function<void(const HttpResponsePtr&)>&& callback,
           int userId) {
            assert(userId == 1);
            Json::Value json;
            json["id"] = 1;
            json["username"] = "tanglong3bf";
            json["password"] = "123456";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(json);
            callback(resp);
        },
        {Get});

    // req: {"ints":[1,2,3,4,5]}
    app().registerHandler(
        "/list",
        [](const HttpRequestPtr& req,
           std::function<void(const HttpResponsePtr&)>&& callback) {
            auto jsonPtr = req->jsonObject();
            assert(jsonPtr);
            const auto& json = *jsonPtr;
            assert(json.isMember("ints"));
            auto ints = json["ints"];
            assert(ints.isArray());
            assert(ints.size() == 5);
            assert(ints[0].asInt() == 1);
            assert(ints[1].asInt() == 2);
            assert(ints[2].asInt() == 3);
            assert(ints[3].asInt() == 4);
            assert(ints[4].asInt() == 5);
            auto resp = drogon::HttpResponse::newHttpResponse();
            callback(resp);
        },
        {Post});

    // req: [1,2,3,4,5]
    app().registerHandler(
        "/root",
        [](const HttpRequestPtr& req,
           std::function<void(const HttpResponsePtr&)>&& callback) {
            auto jsonPtr = req->jsonObject();
            assert(jsonPtr);
            const auto& json = *jsonPtr;
            assert(json.isArray());
            assert(json.size() == 5);
            assert(json[0].asInt() == 1);
            assert(json[1].asInt() == 2);
            assert(json[2].asInt() == 3);
            assert(json[3].asInt() == 4);
            assert(json[4].asInt() == 5);
            auto resp = drogon::HttpResponse::newHttpResponse();
            callback(resp);
        },
        {Post});

    app().registerHandler(
        "/regex/{id}/{name}",
        [](const HttpRequestPtr& req,
           std::function<void(const HttpResponsePtr&)>&& callback,
           int id,
           const std::string& name) {
            assert(id == 123);
            assert(name == "zhangsan");
            auto jsonPtr = req->jsonObject();
            assert(jsonPtr);
            const auto& json = *jsonPtr;
            assert(json.isObject());
            auto userList = json["user_list"];
            assert(userList.isArray());
            assert(userList.size() == 3);
            auto param = json["param"];
            assert(param.isObject());

            Json::Value result;
            result["id"] = 233;
            result["username"] = "zhangsan";
            result["password"] = "fawaikuangtu";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(result);
            callback(resp);
        },
        {Post});
    app().registerHandler(
        "/funcWithNamespace",
        [](const HttpRequestPtr& req,
           std::function<void(const HttpResponsePtr&)>&& callback) {
            auto jsonPtr = req->jsonObject();
            assert(jsonPtr);
            const auto& json = *jsonPtr;
            assert(json.isObject());
            assert(json.isMember("a"));
            auto a = json["a"];
            assert(a.isInt());
            assert(a.asInt() == 1);

            Json::Value result;
            result["hello"] = "world";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(result);
            callback(resp);
        },
        {Get});

    app().addListener("0.0.0.0", 8000);
    app().run();
}
