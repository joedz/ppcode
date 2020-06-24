## 日志模块

### Logger 日志器

默认 debug 日志
默认日志格式   "%D{%y-%m-%d %H:%M:%S}%T%t %N fiber=%F%T[%c] [%N]%T%f:%L%T%e %E%T%m%n" 位置。修改位置 log/log_format.cc
拥有多个日志输出地： 默认输出 屏幕

### LogFormatter 日志格式化器
格式化说明
```
    %c 日志名称
    %D 时间 需要 + {日期格式} 
       日期格式参数以C语言函数strftime相同 默认 "%y:%m:%d %H-%M-%S" 位置log/log_format.cc ：TimerFormatter
    %e 错误信息error
    %E 错误信息字符串
    %f 文件名
    %F 协程id
    %L 行号
    %m 消息
    %N 线程名称
    %P 日志级别
    %t 线程id
    %n 换行
    %T 制表符
    %t 线程id
```

### LogLevel 
日志级别 
            ALL,		// 打印所有日志类型 
			DEBUG,		// 打印debug信息日志
			INFO,		// 打印调试信息日志
			WARN,		// 打印警告日志 
			ERROR,		// 打印错误日志 但是不影响系统
			FATAL,		// 打印错误日志 严重错误
			OFF			// 不打印日志

###  日志器使用
```
1. 通过LOG_NAME的宏创建一个日志器，需要自己加入日志输出器
2. 通过LOG_LOAD_FILE的宏， 从指定的配置文件中指定名称的日志器，可以配置日志器名称，级别，格式，输出器
3. 通过LOG_DEBUG(logger)的宏，从指定日志器中输出DEBUG的日志

```
### 日志配置格式

```yaml
testlog:   # 日志加载是名称
  name: "testlog"  # 日志在程序中打印的名称
  level: "debug"   # 日志器的级别
  formatter: "%D%T%t%P%c%m%N%n"  # 日志器的格式化
  appenders:
    - type :  "fileappender"  # 日志输出器类型
      path :  "outlog.txt"    # 日志输出器输出指定文件
      level : "debug"         # 日志输出器输出级别
      formatter : "%D%T%t%P%c%m%N%n" # 日志格式化
    - type : "consoleappender"       # 日志输出器类型
      level : "debug"               #日志输出器级别
```

## 性能测试




