#include <yuser.h>

int main(void)
{
  int pid;

  // find a bunch of evil memory addresses that might cause the kernel to fault
  int num_evil_addresses = 6;
  void** evil_addresses = malloc(sizeof (void *) * num_evil_addresses);
  // NULL
  evil_addresses[0] = NULL;
  // address in kernel space (unmapped, hopefully)
  evil_addresses[1] = (void*)(0x100000 / 2);
  // address in kernel space (in the kernel stack)
  evil_addresses[2] = (void*)0xfffdd;
  // address in user TEXT
  evil_addresses[3] = (void*)(0x100000 + 0x1000);
  // address in (hopefully) unmapped user space
  evil_addresses[4] = (void*)(0x100000 + 0x100000 / 2);
  // address above user space
  evil_addresses[5] = (void*)(0x100000 * 3);

//  // PipeInit, with a flawed pipe_idp
//  pid = Fork();
//  if (pid == 0) {
//    for (int i = 0; i < num_evil_addresses; i++) {
//      TtyPrintf(0, "PipeInitTest: about to test pipe init with evil input %x\n", evil_addresses[i]);
//      PipeInit(evil_addresses[i]);
//    }
//    TtyPrintf(0, "PipeInitTest: passed all tests\n");
//    Exit(0);
//  }
//
//  // LockInit, with a flawed lock_idp
//  pid = Fork();
//  if (pid == 0) {
//    for (int i = 0; i < num_evil_addresses; i++) {
//      TtyPrintf(1, "LockInitTest: about to test lock init with evil input %x\n", evil_addresses[i]);
//      LockInit(evil_addresses[i]);
//    }
//    TtyPrintf(1, "LockInitTest: passed all tests\n");
//    Exit(0);
//  }
//
//  // CvarInit, with a flawed cvar_idp
//  pid = Fork();
//  if (pid == 0) {
//    for (int i = 0; i < num_evil_addresses; i++) {
//      TtyPrintf(1, "CvarInitTest: about to test cvar init with evil input %x\n", evil_addresses[i]);
//      CvarInit(evil_addresses[i]);
//    }
//    TtyPrintf(1, "CvarInitTest: passed all tests\n");
//    Exit(0);
//  }
//
//  // PipeRead, with a flawed buf
//  int pipe_id;
//  PipeInit(pipe_id);
//  pid = Fork();
//  if (pid == 0) {
//    for (int i = 0; i < num_evil_addresses; i++) {
//      TtyPrintf(1, "PipeReadTest: about to test pipe read with evil input %x\n", evil_addresses[i]);
//      PipeRead(pipe_id, evil_addresses[i], 10);
//    }
//    TtyPrintf(1, "PipeReadTest: passed all tests\n");
//    Exit(0);
//  }
//
//  // PipeWrite, with a flawed buf
//  pid = Fork();
//  if (pid == 0) {
//    for (int i = 0; i < num_evil_addresses; i++) {
//      TtyPrintf(1, "PipeWriteTest: about to test pipe write with evil input %x\n", evil_addresses[i]);
//      PipeWrite(pipe_id, evil_addresses[i], 10);
//    }
//    TtyPrintf(1, "PipeWriteTest: passed all tests\n");
//    Exit(0);
//  }

//   exec, with either flawed filename or argvec
  pid = Fork();
  if (pid == 0) {
    char** valid_argv = malloc(sizeof (char *) * 3);
    valid_argv[0] = "hello";
    valid_argv[1] = "world";
    valid_argv[2] = NULL;
    for (int i = 0; i < num_evil_addresses; i++) {
      pid = Fork();
      if (pid == 0) {
        TtyPrintf(3, "ExecTest: Trying to exec with evil name %x\n", evil_addresses[i]);
        TracePrintf(1, "ExecTest: Trying to exec with evil name %x\n", evil_addresses[i]);
        Exec(evil_addresses[i], valid_argv);
        TtyPrintf(3, "ExecTest: Exec failed when trying to load name %x\n", evil_addresses[i]);
        Exit(0);
      }

      pid = Fork();
      if (pid == 0) {
        TtyPrintf(3, "ExecTest: Trying to exec with evil argv %x\n", evil_addresses[i]);
        TracePrintf(1, "ExecTest: Trying to exec with evil argv %x\n", evil_addresses[i]);
        Exec("checkpoint_1/test_processes/test_message", evil_addresses[i]);
        TtyPrintf(3, "ExecTest: Exec failed when trying to load argv %x\n", evil_addresses[i]);
        Exit(0);
      }
    }
    TtyPrintf(3, "ExecTest: started all tests\n");
    Exit(0);
  }

  // TODO -- wait, with a flawed status_ptr

  // TODO -- brk, with a flawed addr

  // TODO -- TtyRead, with a flawed buf, or len > buf

  // TODO -- TtyWrite, with a flawed buf, or len > buf

//  pid = Fork();
//  if (pid == 0)
//    GarbageMan();
//  if (pid < 0)
//    TracePrintf(0,"GarbageMan fork failed\n");

  while(1)
  {
    Delay(100);
    TtyPrintf(0, "Parent woke up; going back to sleep\n");
  }

  TtyPrintf(0, "*** I SHOULDN'T BE HERE!!!\n");
  return -1;
}
