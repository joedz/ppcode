

## 一般c++ auto变量的使用
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

```cpp
typedef struct ucontext {
    struct ucontext *uc_link;
    sigset_t uc_sigmask;
    stack_t uc_stack;
    mcontext_t uc_mcontext;
    ...
} ucontext_t;

```


