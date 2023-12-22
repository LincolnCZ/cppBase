## 安装
```shell
brew install protobuf
protoc --version
```
## 编译
```shell
protoc --proto_path=. --cpp_out=. addressbook.proto
```




