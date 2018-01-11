# 关于Yet Another Compiler
<!-- 分工介绍不会放到报告里的，主要是为了我的GitHub要装逼。我就自私地用第一人称介绍。 By sunziping2016 -->
这是我为了编译原理大作业用C++写的一个编译器，它支持将一种仿C设计的语言编译到LLVM，以及直接利用JIT在x86平台上解释运行。

## 架构及依赖

编译器采用以下的架构：
* 词法分析器：借助flex，参考[ANSI C grammar (Lex)](https://www.lysator.liu.se/c/ANSI-C-grammar-l.html)
* 语法分析器：借助bison，参考[ANSI C grammar (Yacc)](https://www.lysator.liu.se/c/ANSI-C-grammar-y.html)
* 语义分析器：使用C++，纯原创，参考[C language - cppreference.com](http://en.cppreference.com/w/c/language)
* 中间码生成：调用LLVM

其构建需要`cmake`，其编译需要以下组件：
* C++11
* Lex/Flex & YACC/Bison
* LLVM
* `getopt()`

## 构建及运行
```bash
# Build
mkdir build
cd build
cmake ..
make
```

# 设计
## 语言
其基本语法类似标准C语言，下面就着重介绍其和C语言的差异。

## 可能的改进
限于精力，我无法完整地实现一个C语言编译器。这里指出一些后人可改进的地方（按迫切程度）：

* 类型和值的系统推导应当提前到语义分析，而非借助LLVM的中间码生成：除了这是更加合理，错误信息更友好的设计之外，还有如下的现实因素：
    * 类型系统：抛弃LLVM自带的类型系统，使用自己的类型系统并手动对表达式的类型（type of expression）进行推导。目前有以下两点原因：
        * 无符号类型系统：LLVM自带的类型系统不存在无符号类型。实现时，应当在使用LLVM算术运算和类型转换的时候设置正确的符号标志。
        * const和volatile系统：LLVM自带的类型系统不存在const和volatile。实现时，const类型会影响值的分类（使之为const左值）进而使赋值或自增表达式非法；在使用LLVM的访存指令的时候，设置正确的volatile位；声明的时候需要注意指针是cv-qualified。
    * 值系统：LLVM不存在左值、右值、常量左值之类的信息，需要自动实现。
