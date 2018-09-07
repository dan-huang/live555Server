# live555Server

For Android.


Stage 1.

## Features

- [x] 封装了HTTP Method、Header、URL、Response Validation
- [x] Result Code Localization
- [x] AccessToken、RefreshToken、Region自管理
- [x] 接口的Success Block直接返回对应的Model对象
- [x] 接口的Failure Block包含了业务逻辑上所有的Failure
- [x] 每次接口调用可以单独定义timeout值
- [x] RefreshToken出现问题时发送广播
- [ ] 接口、ResultCode、LocalizationJSON的补全
- [ ] 生产环境和测试环境的切换
- [ ] Unit Test

## Installation

### CocoaPods

修改Podfile:

```ruby
source 'https://github.com/CocoaPods/Specs.git'
source 'http://HuangJunRen@gitlab.loomo.com/HuangJunRen/UserSDK.git'
platform :ios, '9.0'

target '<Your Target Name>' do
  pod 'UserSDK', :git => 'http://HuangJunRen@gitlab.loomo.com/HuangJunRen/UserSDK.git'
end
```

运行以下命令:

```bash
$ pod update
```

## 调用示例

[Obj-C](http://gitlab.loomo.com/HuangJunRen/PodTestOC/blob/master/PodTestOC/ViewController.m)

[Swift](http://gitlab.loomo.com/HuangJunRen/UserSDK/blob/master/UserSDK/ViewController.swift)

## Result Code Localization

需要按照[示例](http://gitlab.loomo.com/HuangJunRen/UserSDK/blob/master/UserSDK/LocalizedCodeDescriptionForUserSDK.json)的格式在主项目的Bundle.main中加入LocalizedCodeDescriptionForUserSDK.json文件。

```
{
	"90000": {"zh-Hans": "成功", "en": "success"},
	"90014": {"zh-Hans": "用户名或密码错误", "en": "username or password incorrect"}
}
```

### JSON Key

包含服务端的ResultCode和自定义Code，枚举在UserSDK的[ResultCode](http://gitlab.loomo.com/HuangJunRen/UserSDK/blob/master/Source/ResultCode.swift)中。

### JSON Value

语言的定义枚举在UserSDK的[Language](http://gitlab.loomo.com/HuangJunRen/UserSDK/blob/master/Source/Language.swift)中，内容可以根据产品需求自定义。

APP切换语言时，需要同步调用[UserSDK setLanguage:]方法。

## RefreshToken出错时

UserSDK会发送广播（USER_SDK_NOTIFICATION_REFRESH_TOKEN_INVALID），APP自定义相应行为（比如：调起登录界面让用户重新登录）

## Timeout自定义

每个接口都有parameters字典，在parameters加入如下KeyValue即可。

```
{ "timeout" : [NSNumber numberWithDouble: 0.5] }
```

如果不加入，则使用默认timeout值，可使用[UserSDK setDefaultTimeout:]更改默认值。

## 第一次兼容旧版本

调用[UserSDK setup:]和[UserSDK setLanguage:]方法，将原来存储在本地的AccessToken、RefreshToken、Region、Language转存到UserSDK中。使老用户不会受到影响。
