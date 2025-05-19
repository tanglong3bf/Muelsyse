#include <drogon/drogon.h>

using namespace drogon;

int main(int argc, char* argv[])
{
    app().registerHandler(
        "/test",
        [](const HttpRequestPtr& req,
           std::function<void(const HttpResponsePtr&)>&& callback) {
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

    app().addListener("0.0.0.0", 8000);
    app().run();
}
