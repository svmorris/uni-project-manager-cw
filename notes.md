Debugging memory leaks
======================

* Make Asan pause before crashing the program.
```
export ASAN_OPTIONS=sleep_before_dying=10000000000
```

* Run the program


* When the program pauses, attach gdb.
```
gdb -q -p $(pidof program)
```
