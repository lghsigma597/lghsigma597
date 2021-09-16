/**
 * @file    XXXallocator.cc
 * @brief   TODO brief documentation here.
 *
 * @author
 * @version $Id$
 */

/*{{{ Headers ------------------------------------------------------------------------------------*/
#include "server/executor/XXXallocator.h" /* SELF */
/*------------------------------------------------------------------------------------ Headers }}}*/

namespace executor {
namespace test {

/*{{{ AllocatorImpl ------------------------------------------------------------------------------*/
AllocatorImpl::AllocatorImpl(alloc_type type, alloc_scope scope)
    : type_(type),
      scope_(scope),
      memories_(std::deque<uchar *>{}),
      deleters_(std::deque<deleter>{}),
      sizes_(std::deque<size_type>{}),
      fn_release_(nullptr),
      fn_dump_(nullptr) {}

AllocatorImpl::~AllocatorImpl() {}
/*------------------------------------------------------------------------------ AllocatorImpl }}}*/

/*{{{ Allocator ----------------------------------------------------------------------------------*/
Allocator::Allocator(alloc_type type, alloc_scope scope) : p_alloc_(nullptr) {
  if (type == alloc_type::STACK) {
    assert("STACK's scope shuld not be NONE" && scope != alloc_scope::NONE);
    p_alloc_ = new Stack(type, scope);
  } else {
    assert("HEAP's scope shuld be NONE" && scope == alloc_scope::NONE);
    p_alloc_ = new Heap(type, scope);
  }
}

Allocator::Allocator(alloc_type type) : Allocator(type, alloc_scope::NONE) {}

#if 0
Allocator &Allocator::operator=(const Allocator &rhs) {
  p_alloc_ = rhs.p_alloc_;

  return *this;
}
#endif
/*---------------------------------------------------------------------------------- Allocator }}}*/

/*{{{ STACK --------------------------------------------------------------------------------------*/
Stack::Stack(alloc_type type, alloc_scope scope) : AllocatorImpl(type, scope) {
  fn_release_ = [this](void *obj) { this->Stack::ReleaseObjectInternal(obj); };
  fn_dump_ = [this]() { this->Stack::DumpInternal(); };
}

Stack::~Stack() {
  std::deque<deleter>::reverse_iterator rit = deleters_.rbegin();
  for (; rit != deleters_.rend(); ++rit) {
    (*rit)(true);

    memories_.pop_back();
    deleters_.pop_back();
    sizes_.pop_back();
  }

  if (sizes_.size() > 0) this->Dump();
}

/**
 * @brief       Call ONLY destructor
 *
 * @param[in]   obj
 *
 * @warning     destructor에서 해제하는 모든 자원에 대해 nullptr check 및 set to nullptr이 필요
 */
void Stack::ReleaseObjectInternal(void *obj) {
  std::deque<uchar *>::iterator it_mem = memories_.begin();
  std::deque<deleter>::iterator it_del = deleters_.begin();

  for (uchar *obj_addr = static_cast<uchar *>(obj); it_mem != memories_.end(); ++it_mem, ++it_del)
    if (*it_mem == obj_addr) {
      (*it_del)(false);
      break;
    }
}

/* debug */
void Stack::DumpInternal() const {
  std::cout << "  STACK ALLOCATOR" << std::endl;

  std::cout << "SCOPE ===" << std::endl;
  switch (scope_) {
    case alloc_scope::SESSION:
      std::cout << "  SESSION" << std::endl;
      break;
    case alloc_scope::STATEMENT:
      std::cout << "  STATEMENT" << std::endl;
      break;
    case alloc_scope::COMPONENT_MANAGER:
      std::cout << "  COMPONENT_MANAGER" << std::endl;
      break;
    default:
      assert("STACK's scope shuld not be NONE" && false);
      break;
  }

  int count = 0;
  for (size_type i = 0; i < sizes_.size(); ++i)
    if (memories_[i]) {
      std::cout << "MEMORIES " << i << " ===" << std::endl
                << "  Addr : " << &memories_[i] << std::endl
                << "  Size : " << sizes_[i] << std::endl;
      ++count;
    }
  std::cout << std::endl << "Total allocated count : " << count << std::endl << std::endl;
}
/*-------------------------------------------------------------------------------------- STACK }}}*/

/*{{{ HEAP ---------------------------------------------------------------------------------------*/
Heap::Heap(alloc_type type, alloc_scope scope) : AllocatorImpl(type, scope) {
  fn_release_ = [this](void *obj) { this->Heap::ReleaseObjectInternal(obj); };
  fn_dump_ = [this]() { this->Heap::DumpInternal(); };
}

/**
 * @brief       Destructor of Heap
 *
 * @warning     It does not delete allocated memories
 */
Heap::~Heap() {
  if (sizes_.size() > 0) this->Dump();
}

void Heap::ReleaseObjectInternal(void *obj) {
  std::deque<uchar *>::iterator it_mem = memories_.begin();
  std::deque<deleter>::iterator it_del = deleters_.begin();
  std::deque<size_type>::iterator it_size = sizes_.begin();

  for (uchar *obj_addr = static_cast<uchar *>(obj); it_mem != memories_.end();)
    if (*it_mem == obj_addr) {
      (*it_del)(true);

      memories_.erase(it_mem++);
      deleters_.erase(it_del++);
      sizes_.erase(it_size++);
      break;
    }
}

/* debug */
void Heap::DumpInternal() const {
  std::cout << "  HEAP ALLOCATOR" << std::endl;

  std::cout << "SCOPE ===" << std::endl;
  switch (scope_) {
    case alloc_scope::NONE:
      std::cout << "  NONE " << std::endl;
      ;
      break;
    default:
      assert("HEAP's scope shuld be NONE" && false);
      break;
  }

  int count = 0;
  for (size_type i = 0; i < sizes_.size(); ++i)
    if (memories_[i]) {
      std::cout << "MEMORIES " << i << " ===" << std::endl
                << "  Addr : " << &memories_[i] << std::endl
                << "  Size : " << sizes_[i] << std::endl;
      ++count;
    }
  std::cout << std::endl << "Total allocated count : " << count << std::endl << std::endl;
}
/*--------------------------------------------------------------------------------------- HEAP }}}*/

}  // namespace test
}  // namespace executor

/* end of XXXallocator.cc */
