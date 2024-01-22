import sys

print("Content-type: text/html\r\n\r\n", end="")

for line in sys.stdin:
    print(line)
    print("="*10)
    print("="*10)