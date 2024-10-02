# Interoperability with Python

Moss is able to use Python API and therefore Python libraries and functions.
For this it is important to have Python installed as it is just embedded.

Module `python` provides all needed resources.

```cpp
import python

datetime = python.module("datetime")
now = datetime::datetime.now()

print(now)
```