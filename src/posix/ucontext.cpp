#include <kernel/os.hpp>
#include <posix/ucontext.hpp>

extern "C" void restore_context_stack();

static void prepare_context_stack(ucontext_t *ucp, ucontext_t *successor_context, int argc, va_list args)
{
  size_t* stack_ptr = (size_t *)(ucp->uc_mcontext.ret_esp);

  // successor_context
  stack_ptr -= 1;

  if(successor_context == nullptr) {
    stack_ptr[0] = (size_t)&OS::get_default_successor_context();
  }
  else {
    stack_ptr[0] = (size_t)successor_context;
  }

  // arguments
  stack_ptr -= (argc + 1);
  stack_ptr[0] = (size_t)argc;

  for(int i = 1; i <= argc; i++) {
    stack_ptr[i] = va_arg(args, size_t);
  }

  //return function
  stack_ptr -= 1;
  stack_ptr[0] = (size_t)restore_context_stack;

  ucp->uc_mcontext.ret_esp = (size_t)stack_ptr;
}

extern "C" {

void makecontext(ucontext_t *ucp, void (*func)(), int argc, ...) {
  if (ucp == nullptr || func == nullptr || argc < 0) {
    errno = EINVAL;
    return;
  }

  if (ucp->uc_stack.ss_sp == nullptr || ucp->uc_stack.ss_size == 0
      || ucp->uc_stack.ss_size > ucontext_t::MAX_STACK_SIZE) {

    errno = EINVAL;
    return;
  }

  ucp->uc_mcontext.ret_esp = (size_t) ucp->uc_stack.ss_sp;
  ucp->uc_mcontext.ebp = ucp->uc_mcontext.ret_esp;
  ucp->uc_mcontext.eip = (size_t) func;

  va_list args;
  va_start(args, argc);

  prepare_context_stack(ucp, ucp->uc_link, argc, args);
}

int swapcontext(ucontext_t *oucp, ucontext_t *ucp)
{
  if(oucp == nullptr || ucp == nullptr) {
    errno = EINVAL;
    return -1;
  }

  static bool been_here = false;
  getcontext(oucp);

  if(!been_here) {
    been_here = true;
    return setcontext(ucp);
  }

  return 0;
}

}
