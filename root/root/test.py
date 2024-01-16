import time
import sys

print("Posted data:")
print("=" * 20)
for data in sys.stdin:
    print(data, end = "")
print("=" * 20)