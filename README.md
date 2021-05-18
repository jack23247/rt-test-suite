# rt-test-suite

Testing the performance of the RT kernel + EDF

## Usage:

```
$ sudo ./rt-test-suite.x86_64 > data.csv
```

To see if there were any problems:
```
journalctl -e
```

And look for relevant errors.