日志（Logging）
===============

日志是一组帮助你记录程序运行状态的宏。它可以记录你的程序运行时的状态，比如函数调用，变量值，错误信息等等。
所有的数据都会被写入到文件中。

## LOG condition (when the log is enabled)

- INFO_IF()
- INFO_EVERY_()
- INFO_IF_EVERY_()
- INFO_FIRST()
- INFO_FIRST_()

## Severity Level

这里有五种Log等级: INFO, LOG, WARNING, ERROR, FATAL.

INFO等级的日志会在当前上下文中没有其他LOG, WARNING, ERROR, and FATAL时被丢弃。

注意：LOG FATAL 会导致程序终止

## LOG Only

LOG_ONLY 可以让你只在某个函数，或者某个文件，或者某个模块中打印日志。

## Trace

你可以使用info来打印一些信息，然后使用LOG宏来记录它们。这样可以构建更加完整的日志trace信息。
