# 关于Yet Another Compiler
<!-- 分工介绍不会放到报告里的，主要是为了我的GitHub要装逼。我就自私地用第一人称介绍。 By sunziping2016 -->
这是我为了编译原理大作业用C++写的一个编译器，它支持将一种仿C设计的语言编译到LLVM，以及直接利用JIT在x86平台上解释运行。

## 架构及依赖

编译器采用以下的架构：
* 词法分析器：借助flex，参考[ANSI C grammar (Lex)](https://www.lysator.liu.se/c/ANSI-C-grammar-l.html)
* 语法分析器：借助bison，参考[ANSI C grammar (Yacc)](https://www.lysator.liu.se/c/ANSI-C-grammar-y.html)
* 语义分析器：使用C++，原创，部分语法参考[C language - cppreference.com](http://en.cppreference.com/w/c/language)
* 后端部分：LLVM

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

## 语言
其基本语法类似标准C语言，下面就着重介绍其和C语言的差异。

