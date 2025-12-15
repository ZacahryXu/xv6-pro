// Mutual exclusion lock.
struct spinlock {
  uint locked;       // Is the lock held? //是拥有锁吗

  // For debugging:
  char *name;        // Name of lock. 锁名
  struct cpu *cpu;   // The cpu holding the lock.拥有锁的cpu
};

