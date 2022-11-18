#include <yuser.h>

int main(void)
{
   int pid;

   // find a bunch of evil memory addresses that might cause the kernel to fault
   int num_evil_addresses = 7;
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
   // kernel text
   evil_addresses[6] = (void*)(0x1000);


   // PipeInit, with a flawed pipe_idp
   pid = Fork();
   if (pid == 0) {
     for (int i = 0; i < num_evil_addresses; i++) {
       TtyPrintf(0, "PipeInitTest: about to test pipe init with evil input %x\n", evil_addresses[i]);
       PipeInit(evil_addresses[i]);
     }
     TtyPrintf(0, "PipeInitTest: passed all tests\n");
     Exit(0);
   }

   // LockInit, with a flawed lock_idp
   pid = Fork();
   if (pid == 0) {
     for (int i = 0; i < num_evil_addresses; i++) {
       TtyPrintf(1, "LockInitTest: about to test lock init with evil input %x\n", evil_addresses[i]);
       LockInit(evil_addresses[i]);
     }
     TtyPrintf(1, "LockInitTest: passed all tests\n");
     Exit(0);
   }

   // CvarInit, with a flawed cvar_idp
   pid = Fork();
   if (pid == 0) {
     for (int i = 0; i < num_evil_addresses; i++) {
       TtyPrintf(1, "CvarInitTest: about to test cvar init with evil input %x\n", evil_addresses[i]);
       CvarInit(evil_addresses[i]);
     }
     TtyPrintf(1, "CvarInitTest: passed all tests\n");
     Exit(0);
   }

   // PipeRead, with a flawed buf
   int pipe_id;
   PipeInit(&pipe_id);
   pid = Fork();
   if (pid == 0) {
     for (int i = 0; i < num_evil_addresses; i++) {
       TtyPrintf(1, "PipeReadTest: about to test pipe read with evil input %x\n", evil_addresses[i]);
       PipeRead(pipe_id, evil_addresses[i], 10);
     }
     TtyPrintf(1, "PipeReadTest: passed all tests\n");
     Exit(0);
   }

   // PipeWrite, with a flawed buf
   pid = Fork();
   if (pid == 0) {
     for (int i = 0; i < num_evil_addresses; i++) {
       TtyPrintf(1, "PipeWriteTest: about to test pipe write with evil input %x\n", evil_addresses[i]);
       PipeWrite(pipe_id, evil_addresses[i], 10);
     }
     TtyPrintf(1, "PipeWriteTest: passed all tests\n");
     Exit(0);
   }

//    exec, with either flawed filename or argvec
   pid = Fork();
   if (pid == 0) {
     char** valid_argv = malloc(sizeof (char *) * 3);
     valid_argv[0] = "hello";
     valid_argv[1] = "world";
     valid_argv[2] = NULL;
     for (int i = 0; i < num_evil_addresses; i++) {
       TtyPrintf(0, "ExecTest: Trying to exec with evil name %x\n", evil_addresses[i]);
       TracePrintf(1, "ExecTest: Trying to exec with evil name %x\n", evil_addresses[i]);
       int rc = Exec(evil_addresses[i], valid_argv);
       TracePrintf(1, "ExecTest: Return code is %d\n", rc);
       TtyPrintf(0, "ExecTest: Exec failed when trying to load name %x\n", evil_addresses[i]);

       TtyPrintf(0, "ExecTest: Trying to exec with evil argv %x\n", evil_addresses[i]);
       TracePrintf(1, "ExecTest: Trying to exec with evil argv %x\n", evil_addresses[i]);
       Exec("src/test_processes/test_message", evil_addresses[i]);
       TtyPrintf(0, "ExecTest: Exec failed when trying to load argv %x\n", evil_addresses[i]);
     }
     TtyPrintf(0, "ExecTest: finished all tests\n");
     Exit(0);
   }

//   wait, with a flawed status_ptr
  for (int i = 0; i < num_evil_addresses; i++) {
    TtyPrintf(2, "WaitTest: Waiting with evil status pointer %d\n", evil_addresses[i]);
    Wait(evil_addresses[i]);
  }
  TtyPrintf(2, "WaitTest: All tests passed\n");

  // ---- brk, with a flawed addr ---- //
  for (int i = 0; i < num_evil_addresses; i++ ) {
    TtyPrintf(2, "BrkTest: Setting brk to evil address %d\n", evil_addresses[i]);
    Brk(evil_addresses[i]);
  }
  TtyPrintf(2, "BrkTest: All tests passed\n");

  //---- TtyRead, with a flawed buf, or len > buf -------//
  TracePrintf(1, "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n");
  TracePrintf(1, "Beginning to tickle possible bugs in TtyRead\n");
  TtyRead(0, "hello", 0);
  TtyRead(0, "hello", 10);
  for (int i = 0; i < num_evil_addresses; i++) {
    TtyPrintf(1, "TtyReadTest: Reading into evil address %d\n", evil_addresses[i]);
    TtyRead(0, evil_addresses[i], 10);
  }
  TtyPrintf(1, "TtyReadTest: All tests passed\n");
  TracePrintf(1, "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n");
  TracePrintf(1, "Finished tickling TtyRead; now tickling TtyWrite\n");

  // ----- TtyWrite, with a flawed buf, or len > buf ------//
  // write with a flawed length
  TtyWrite(0, "hello", 0);
  // writes with length > buf; fails without writing
  TtyWrite(0, "hello", 10000000);
  // Null checks
  TtyWrite(-1, "hello", 5);
  TtyWrite(400, "hello", 5);
  TtyWrite(4105, "hello", 5);
  for (int i = 0; i < num_evil_addresses; i++) {
    TtyPrintf(1, "TtyWriteTest: Writing from evil address %d\n", evil_addresses[i]);
    // note that 0x101000 should be able to write bytes, as user text is readable!
    TtyWrite(1, evil_addresses[i], 5);
  }
  TtyPrintf(1, "TtyWriteTest: All tests passed\n");
  TracePrintf(1, "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n");
  TracePrintf(1, "Finished tickling TtyWrite\n");

  //------ TTYPrintf Tests----------//
  // When uncommented, these segfault and halt, as they should.
  // TtyPrintf(-1, "Hello from process %d\n", GetPid());
  // null checks - segfaults and halts
  // TtyPrintf(0, NULL);
  // TtyPrintf(NULL, NULL);
  // write from kernel text
  // TtyPrintf(0, 0x02000); 

  while(1)
  {
    Delay(100);
    TtyPrintf(0, "Parent woke up; going back to sleep\n");
  }

  TtyPrintf(0, "*** I SHOULDN'T BE HERE!!!\n");
  return -1;
}
