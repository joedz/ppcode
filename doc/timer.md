# 定时器

## 设计思想：

### 定时方法

1. SIGSARM信号
2. socket超时SO_RECVTIMEO和SO_SNDTIMEO
3. I/O复用系统调用的超时函数

## 定时器实现

创建一个定时器线程去管理定时器任务。

有多个定时器任务，使用map或者优先级队列保存定时器任务。

使用epoll等待定时时间到达。

## 工具

map / 最小堆

function

epoll





## 红黑树与最小堆的实现的比较

**红黑树**

|              | 红黑树                        | 最小堆      | 优势比较                                                     |
| ------------ | ----------------------------- | ----------- | ------------------------------------------------------------ |
| 查找         | 最坏2LogN次                   | ----------- | 对于定时器来讲，需要频繁的查找最小值，或许有特殊需要找到某一项任务将其删除。但是最小堆对于随机存储这一点来讲不太支持。 |
| 插入         | 最坏比较2logN，外加两次旋转。 | 最坏logN次  | 最小堆的优势在于插入基本有序的数据，红黑树需要多次的对比才能插入。 |
| 删除         | 需要最多三次的旋转            | 2logN       | 最小堆的头删，需要进行一次的调堆操作，将造成大量比较         |
| 查找最小结点 | logN                          | O1          | 最小堆总是能在01的时间复杂度里面找到最小值，但是红黑树有特殊实现也可以做到这一点。 |
| 删除最小值   |                               | 2logN       | 红黑树的性能要好于最小堆。最小堆删除头结点后，将造成大量数据的移动。这一点性能很差。 |

最小堆一般是采用堆的方式实现，元素访问速度远高于采用链表方式的红黑树，插入性能快红黑树好几倍，但最小堆的删除性能并不快于红黑树

最小堆的最大缺点就在于必须是连续的内存。




