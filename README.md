# Muelsyse

本项目是Drogon的一个插件，用于简化Drogon项目中远程调用的开发。

本插件是一个半成品，功能并不多，仅仅做到了基本能用的程度。

# 如何使用本插件

## 将插件导入到Drogon项目中

1. 将src目录下的两个文件拷贝到项目中

路径随意，可以按照你的个人喜好进行修改，可以像这样：

```shell
$ tree .
.
├── CMakeLists.txt
├── config.yaml
├── main.cc
├── plugins
│   └── tl
│       └── rpc
│           ├── Muelsyse.cc
│           └── Muelsyse.h
```

2. 修改`CMakeLists.txt`文件，将插件编译进项目中

路径需要和上一步存储的位置相匹配

```cmake
// ...
aux_source_directory(plugins/tl/rpc RPC_SRC)
// ...
target_sources(${PROJECT_NAME}
               PRIVATE
               ${SRC_DIR}
               ${CTL_SRC}
               ${FILTER_SRC}
               ${PLUGIN_SRC}
               ${MODEL_SRC}
               ${RPC_SRC})
```

3. 修改配置文件

将插件注册到框架里，可以让框架管理此插件。

**config.json**

```json
{
  "plugins": [
    {
      "name": "tl::rpc::Muelsyse",
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
  - name: tl::rpc::Muelsyse
    config:
      function_list: []
```

## 使用本插件

1. 修改配置文件，以yaml格式为例，json同理

```yaml
plugins:
  - name: tl::rpc::Muelsyse
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
    void setByJson(const Json::Value &json); // 必须定义此函数，其余函数或成员无要求
    int getId() const;
    std::string getUsername() const;
    std::string getPassword() const;
  private:
    int id;
    std::string username;
    std::string password;
};

User getUserById(int userId) {
    RPC_CALL_SYNC(User, "_", userId);
}

```

# RPC_CALL_SYNC

`RPC_CALL_SYNC`这个宏的第一个参数需要是自定义函数的返回值类型，后续的参数每两个参数为一组。

每一组参数的第一项需要是一个字符串，第二个参数是希望实际传递的值，它的类型比较灵活。

## 传递参数的方式

在每一组参数中，第一个字符串参数用来指定后面参数的实际传递的位置：

1. 当第一个参数为"\_"时，表明第二个参数是动态路径参数。
2. 当第一个参数为""时，表明第二个参数直接以json格式作为请求体。
3. 当第一个参数为其余的普通字符串时，表明第二个参数希望作为请求体的一个子项，以第一个参数为key，以第二个参数为value。

**一些例子**：

例子1

```yaml
plugins:
  - name: tl::rpc::Muelsyse
    config:
      function_list:
        - name: updatePassword # 函数的名字
          url: localhost:10000/user/update_password # 调用对应函数后实际调用的地址
          http_method: put # 调用此接口时使用的Http请求方式，支持：get, post, put, delete
```

```cpp
void updatePassword(int userId, const std::string &password) {
    RPC_CALL_SYNC(void, "user_id", userId, "password", password);
}

```

例子2

```yaml
plugins:
  - name: tl::rpc::Muelsyse
    config:
      function_list:
        - name: updateUserById # 函数的名字
          url: localhost:10000/user/{user_id} # 调用对应函数后实际调用的地址
          http_method: put # 调用此接口时使用的Http请求方式，支持：get, post, put, delete
```

```cpp
void updateUserById(int userId, const User &user) { // User类需要提供toJson成员函数
    RPC_CALL_SYNC(void, "_", userId, "", user);
}

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
