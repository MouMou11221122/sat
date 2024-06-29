# Shared memory API Testing
We just want to test the synchronization in multiple processes shared memory environment and choose/plan the most suitable API and data structure.

### Test the named semaphone API for mutual exclusion
We arrange two processes to run concurrently and perform the increament operation on a shared memory variable initialized by 0 for 100000 times 
seperately. We let the second process run first in the background. The second process will create the a shared memory variable
and blocked by sem1 used to wait the first process. Therefore, we can start to run the first process and it will open the shared 
memory variable that the second process create; then, it will wake up the second process and concurrently execute with it but the mutual exclusion of the shared variable isn't be guaranteed.

Compile them first:
```
$ make
```
Let's start running the second process in background:
```
$ ./second_process &
```
Then, start running the first process:
```
$ ./first_process
```
The wrong result will be outputed.

**Please follow the instructions above, or there may have shared memory object name in the kernel open file unclosed;then, you cannot create the shared memory object using the same name that I have coded in the source code even if the process has terminated unless you reboot.**