LOG Macro
=====================


LOG is a macro help you to LOG structured data into a file and record the meta data about it (FILE, LINE, FUNCTION and LOG messages). 

All the data that put in LOG should be on disk and saved. 

## LOG condition (when the log is enabled)

INFO_IF()
INFO_EVERY_()
INFO_IF_EVERY_()
INFO_FIRST()
INFO_FIRST_()
INFO_LAST()
INFO_LAST_()


## Severity Level

There 5 levels of log information: INFO, LOG, WARNING, ERROR, and FATAL
LOG FATAL will cause the program terminate
The INFO level will discard the information if there is no other LOG, WARNING, ERROR, and FATAL happens in the context.


## LOG Only

Log only on a function, on a file, on a module


## Trace



