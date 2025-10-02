# x86 locking techniques

To provide concurrency on write back memory, we can use
the `lock` prefix, like in

```
lock add dword:[esp], 1
```

We can test a flag for being zero atomically using

```
lock add dword:[esp], 0
```

which reads and modifies the flags atomically.
The value of zero written back makes it a NOP,
but the flags are altered! The ZF is set if the
result is zero.

```
lock cmpxchg
```


Usually, a locking mechanism has a fast path and
a slow path to not make it produce CPU heat in a
loop.

In the fast path, a lock is tried to apply, if
it fails, a spin lock loop should check memory
read only before a retry also utilizing `_mm_pause`.

```
align 16
mutex_lock:
    mov eax, 1
.retry:
    xchg al, [edi]          ; first param...
    test al, al             ; check if we got the lock
    jnz .spinloop
    ret

align 8
.spinloop:
    pause                   ; rep nop
    cmp byte [edi], al
    jne .retry
    jmp .spinloop

mutex_unlock:
    mov byte [rdi], 0
    ret
```


First access to the lock should be atomic RMW (the xchg above).
This is atomic because since the 386, `xchg` and `lock xchg`
are equivalent (But not `cmpxchg` and `lock cmpxchg`).

If the first access is read-only, the CPU might have sent
a share request for the cache line and send another request
on midification for ownership, which doess unneeded requests
to the cache line in case we succeed on first try.



