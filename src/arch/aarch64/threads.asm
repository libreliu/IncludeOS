.arch armv8-a
.global __clone_return
.type __clone_return, %function
.global __thread_yield
.type __thread_yield, %function
.global __thread_restore
.type __thread_restore, %function
/* extern __thread_suspend_and_yield */
/* Don't know how to make sth extern in aarch64 gas syntax
 * the whole thing is a mess..
 */

.text
__clone_return:
    /* IMPLEMENT ME! */
    nop
__thread_yield:
    /* IMPLEMENT ME! */
    nop
__thread_restore:
    /* IMPLEMENT ME! */
    nop
