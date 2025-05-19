# Muelsyse

本项目是[Drogon](https://github.com/drogonframework/drogon)的一个插件，用于简化Drogon项目中HTTP客户端的开发。

本插件是一个半成品，功能并不多，仅仅做到了基本能用的程度。

## 如何使用本插件

### 将插件导入到Drogon项目中

1. 将src目录下的两个文件拷贝到项目中

路径随意，可以按照你的个人喜好进行修改，可以像这样：

```shell
$ tree .
.
├── CMakeLists.txt
├── config.yaml
├── main.cc
└── plugins
    └── tl
        └── rest
            ├── Muelsyse.cc
            └── Muelsyse.h
```

2. 修改`CMakeLists.txt`文件，将插件编译进项目中

路径需要和上一步存储的位置相匹配

```cmake
// ...
aux_source_directory(plugins/tl/rest REST_SRC)
// ...
target_sources(${PROJECT_NAME}
               PRIVATE
               ${SRC_DIR}
               ${CTL_SRC}
               ${FILTER_SRC}
               ${PLUGIN_SRC}
               ${MODEL_SRC}
               ${REST_SRC})
```

3. 修改配置文件

将插件注册到框架里，可以让框架管理此插件。

**config.json**

```json
{
  "plugins": [
    {
      "name": "tl::rest::Muelsyse",
      "config": {
        "function_list": []
      }
    }
  ]
}
```

**config.yaml**（Drogon 1.8.5 版本开始支持了使用yaml完成配置）

```yaml
plugins:
  - name: tl::rest::Muelsyse
    config:
      function_list: []
```

### 使用本插件

1. 修改配置文件，以yaml格式为例，json同理

```yaml
plugins:
  - name: tl::rest::Muelsyse
    config:
      function_list:
        - name: getUserById # 函数的名字
          url: localhost:10000/user/{user_id} # 调用对应函数后实际调用的地址
          http_method: get # 调用此接口时使用的Http请求方式，支持：get, post, put, delete
```

2. 使用辅助宏实现这个函数

```cpp

class User {
  public:
    // 如果希望作为函数的返回值，必须要提供此函数
    void setByJson(const Json::Value &json);
    // 如果希望作为函数的参数，以下两个函数需要提供其一
    // 两个函数同时存在，则会优先使用`toJson()`
    Json::Value toJson() const;
    std::string toString() const;
    // 其余函数无要求
    int getId() const;
    std::string getUsername() const;
    std::string getPassword() const;
  private:
    int id;
    std::string username;
    std::string password;
};

REST_FUNC_SYNC(User, getUserById, int userId)
{
    REST_CALL_SYNC(User, PATH_PARAM(userId));
}

```

## 一些例子

### 同步接口

**配置文件**

```yaml
plugins:
  - name: tl::rest::Muelsyse
    config:
      function_list:
        - name: getUserById
          url: localhost:10000/user/{user_id}
          http_method: get
```

**函数定义**

```cpp
// User类需要有一个`setByJson(const Json::Value&)`成员函数
REST_FUNC_SYNC(User, getUserById, int userId)
{
    REST_CALL_SYNC(User, PATH_PARAM(userId));
}
```

**函数的使用**

```cpp
User user = getUserById(1);
```

### 回调式异步接口

**配置文件**

```yaml
plugins:
  - name: tl::rest::Muelsyse
    config:
      function_list:
        - name: updateUser
          url: localhost:10000/user
          http_method: put
```

**函数定义**

```cpp
// User类需要有一个`Json::Value toJson() const`成员函数
REST_FUNC_ASYNC(void, updateUser, const User& user)
{
    REST_CALL_ASYNC(void, ROOT_PARAM(userId));
}
```

**函数的使用**

```cpp
// User user;
updateUser(user, []() {
    LOG_INFO << "update user success";
}, [](const std::exception &e) {
    LOG_ERROR << e.what();
});
```

### future式异步接口

**配置文件**

```yaml
plugins:
  - name: tl::rest::Muelsyse
    config:
      function_list:
        - name: getUserById
          url: localhost:10000/user/{user_id}
          http_method: get
```

**函数定义**

```cpp
// User类需要有一个`void setByJson(const Json::Value&)`成员函数
REST_FUNC_FUTURE(User, getUserById, int userId)
{
    REST_CALL_FUTURE(User, PATH_PARAM(userId));
}
```

**函数的使用**

```cpp
auto future = getUserById(1);
User user = future.get();
```

## 参数支持的类型

- 基本数据类型
- 字符串
- `unordered_map<string, T>`
- `map<string, T>`
- `vector<T>`
- 带有`toJson()`成员函数的类
- 带有`toString()`成员函数的类

注意：如果一个自定义类型同时支持了`toJson()`和`toString()`，优先使用`toJson()`。

### 路径参数

如果一个参数希望被放到请求路径里，它会被先转换为json，再尝试着按照如下逻辑转换为字符串：

1. `{}` -> `""`
2. `[]` -> `""`
3. `[1]` -> `"1"`
4. `[1, 2, 3]` -> `"1, 2, 3"`
5. `true` -> `"true"`
6. `false` -> `"false"`
7. `{"key": "value"}` -> throw runtime_error

（这里处理的可能有问题，比如嵌套数组没有做出判断，估计结果会很奇怪）

## 返回值支持的类型

1. `void`
2. `Json::Value`
3. 带有`setByJson()`成员函数的类
