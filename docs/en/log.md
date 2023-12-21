LOG Macro
=========

LOG is a macro help you to LOG structured data into a file and record the meta data about it (FILE, LINE, FUNCTION and LOG messages).

All the data that put in LOG should be on disk and saved.

### LOG condition (when the log is enabled)

- INFO_IF()
- INFO_EVERY_()
- INFO_IF_EVERY_()
- INFO_FIRST()
- INFO_FIRST_()

### Severity Level

There 5 levels of log information: INFO, LOG, WARNING, ERROR, and FATAL.

The INFO level will discard the information if there is no other LOG, WARNING, ERROR, and FATAL happens in the context.

Note: LOG FATAL will cause the program terminate

### LOG Only

Log only on a function, or on a file, or on a module

### Trace

You can use info to get some information and later use LOG macro to record them.
