# ntnx-hash-device-library
This repository contains a driver which will enable applications to use a fictional MD5 hashing device.

Directory Structure
-----------------------------------------
The following descriptions just give an overview of what each file does. Please refer to the individual files to know in details.
- ```ntnx-hash-device-library/ntnx_hash.c``` This file contains the main driver code. The md5 hashing algorithm is in this file.
- ```ntnx-hash-device-library/ntnx_hash.h``` This header file contains some of the ```ioctl``` constants and other constants and macros used in the driver as well the wrapper library(```ntnxhashlib.a```)
- ```ntnx-hash-device-library/ntnxhashlib.c``` This contains the wrapper functions that make the ```ioctl``` calls to get the md5 hash. This creates the ```ntnxhashlib.a``` wrapper static library that can be used by any ```C``` program (in our case it is used by test application ```ntnx_hash_test.c```)
- ```ntnx-hash-device-library/ntnx_hash_test.c``` This is the test file that gets linked to ```ntnxhashlib.a``` and calls and tests the wrapper functions inside it.

Build and Install the MD5 hashing device and running the test
-----------------------------------------
### Build, install and run everything
This will do the following:
- Build the driver ```ntnx_hash_mod.ko```
- Install the driver (do an uninstall before installation if a previous version exists)
- Build the ```ntnxhashlib.a``` static wrapper library
- Build the test
- Run the tests

Please see the following sections if you want to build the components individually.

**Note:** This will require ```sudo``` access or ```sudo``` password as it inserts a kernel module.

```
$ git clone https://github.com/NirjharRoy/ntnx-hash-device-library.git
$ cd ntnx-hash-device-library.git
$ make run-tests
```

Sample full test run output with 2 processes with 2 threads per process and input buffer size of 1KB

```
./ntnx_hash_test
Child 0 (PID: 393789) started

 STARTING TEST
Child 1 (PID: 393790) started

 STARTING TEST
Hashes matched. TEST PASSED for thread index 1 pid = 393789

Hashes matched. TEST PASSED for thread index 0 pid = 393789

ALL TEST PASSED!!
Child 0 (PID: 393789) exiting
Parent: Child 0 (PID: 393789) exited with status 0
Hashes matched. TEST PASSED for thread index 0 pid = 393790

Hashes matched. TEST PASSED for thread index 1 pid = 393790

ALL TEST PASSED!!
Child 1 (PID: 393790) exiting
Parent: Child 1 (PID: 393790) exited with status 0

!!!!! All tests passed for all the processes and their child threads !!!!!!
```

### Clean everything
This will do the following:
- Remove the driver file ```ntnx_hash_mod.ko```
- Uninstall the driver
- Remove the ```ntnxhashlib.a``` static wrapper library
- Remove the tests binaries
- All other remaining build artifacts

**Note:** This will require ```sudo``` access or ```sudo``` password as it removes a kernel module.
```
$ cd ntnx-hash-device-library.git
$ make clean
```


### Only Build the driver (without installation)
```
$ git clone https://github.com/NirjharRoy/ntnx-hash-device-library.git
$ cd ntnx-hash-device-library.git
$ make device
```
This will produce the the module file ```ntnx-hash-device-library/ntnx_hash_mod.ko```

### Build and install driver
```
$ git clone https://github.com/NirjharRoy/ntnx-hash-device-library.git
$ cd ntnx-hash-device-library.git
$ make install
```
This will produce the the module file ```ntnx-hash-device-library/ntnx_hash_mod.ko``` and install it as well. In order to check this run ```ls /dev/ntnx_hash```

### Uninstall the driver
```
$ git clone https://github.com/NirjharRoy/ntnx-hash-device-library.git
$ cd ntnx-hash-device-library.git
$ make uninstall
```

### Build the .a wrapper ntnxhashlib library
```
$ git clone https://github.com/NirjharRoy/ntnx-hash-device-library.git
$ cd ntnx-hash-device-library.git
$ make ntnxlib
```

This will produce the the static wrapper library ```ntnx-hash-device-library/libntnxhashlib.a```

### Build the test (without running)
```
$ git clone https://github.com/NirjharRoy/ntnx-hash-device-library.git
$ cd ntnx-hash-device-library.git
$ make test
```
This will produce the the test binary ```ntnx-hash-device-library/ntnx_hash_test```
If you want to run this binary manually, run ```./ntnx_hash_test```
