```cpp
1、for(auto it = container.begin(); it != container.end(); ++it)
   for(const auto& item: container)
   for(auto&& item: container)
2、auto f = [](){};
3、auto p = new very_very_long_class_name;
   auto p = std::make_shared<class_name>(...);
   auto p = std::make_unique<class_name>(...);
   auto p = std::make_tuple(5, 2.0f, std::string("123"));
4、auto [it, ok] = my_map.insert(...);
   for(auto&& [key, value]: my_map)

```