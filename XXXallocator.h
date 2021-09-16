/**
 * @file    XXXallocator.h
 * @brief   TODO brief documentation here.
 *
 * @author
 * @version $Id$
 */

#ifndef _XXXALLOCATOR_H_
#define _XXXALLOCATOR_H_

/*{{{ Headers ------------------------------------------------------------------------------------*/
#include "server/executor/XXXcommon.h"
/*------------------------------------------------------------------------------------ Headers }}}*/

/*{{{ Typedefs -----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------- Typedefs }}}*/

/*{{{ WARNING ------------------------------------------------------------------------------------*/
/***************************************************************************************************
 *
 *
 *
 *
 * EX FRAME 전용 test Allocator입니다.
 * 사장될 코드이며, 타 모듈의 사용을 엄격히 금하고
 * 이를 지키지 않고 사용하여 발생하는 모든 문제들에 대해 일체 책임을 지지 않습니다.
 *
 *
 *
 *
 **************************************************************************************************/
/*------------------------------------------------------------------------------------ WARNING }}}*/

namespace executor {
namespace test {
enum class alloc_type { STACK, HEAP };
enum class alloc_scope { NONE, SESSION, STATEMENT, COMPONENT_MANAGER };

/* AllocatorImpl */
class AllocatorImpl : private std::allocator<uchar> {
 public:
  using size_type = std::deque<uchar *>::size_type;
  using deleter = std::function<void(bool)>;

  AllocatorImpl(alloc_type type, alloc_scope scope);
  virtual ~AllocatorImpl();

  template <typename T, typename... Args>
  T *GetObject(int size, Args &&... args) {
    uchar *memory = this->operator()(size);
    deleters_.emplace_back([this, memory, size](bool do_free) {
      reinterpret_cast<T *>(memory)->~T();
      if (do_free) this->deallocate(memory, size);
    });

    return new (memory) T(std::forward<Args>(args)...);
  }

  void ReleaseObject(void *obj) const { return fn_release_(obj); }
  void Dump() const { return fn_dump_(); }

  inline const size_type GetCount() const { return sizes_.size(); }
  inline const alloc_type GetType() const { return type_; }
  inline const alloc_scope GetScope() const { return scope_; }

  AllocatorImpl() = delete;
  AllocatorImpl(alloc_type &type) = delete;
  AllocatorImpl(const alloc_type &type) = delete;
  AllocatorImpl(AllocatorImpl &) = delete;
  AllocatorImpl(const AllocatorImpl &) = delete;
  AllocatorImpl &operator=(AllocatorImpl &) = delete;
  AllocatorImpl &operator=(const AllocatorImpl &) = delete;

 protected:
  uchar *operator()(int size) {
    memories_.emplace_back(this->allocate(size));
    sizes_.emplace_back(size);

    return memories_.back();
  }

  const alloc_type type_;
  const alloc_scope scope_;

  std::deque<uchar *> memories_;
  std::deque<deleter> deleters_;
  std::deque<size_type> sizes_;

  std::function<void(void *)> fn_release_;
  std::function<void(void)> fn_dump_;
};

/* STACK */
class Stack final : public AllocatorImpl {
 public:
  Stack(alloc_type type, alloc_scope scope);
  virtual ~Stack();

  void ReleaseObjectInternal(void *obj);
  void DumpInternal() const;

  Stack() = delete;
  Stack(Stack &) = delete;
  Stack(const Stack &) = delete;
  Stack &operator=(Stack &) = delete;
  Stack &operator=(const Stack &) = delete;
};

/* HEAP */
class Heap final : public AllocatorImpl {
 public:
  Heap(alloc_type type, alloc_scope scope);
  virtual ~Heap();

  void ReleaseObjectInternal(void *obj);
  void DumpInternal() const;

  Heap() = delete;
  Heap(Heap &) = delete;
  Heap(const Heap &) = delete;
  Heap &operator=(Heap &) = delete;
  Heap &operator=(const Heap &) = delete;
};

/**
 * USAGE {
 *   // STACK
 *   Allocator stack_alloc(STACK, STATEMENT);
 *   T *obj = stack_alloc.GetObject<T>(constructor args);
 *   T *objs = stack_alloc.GetObject<T>(n, constructor args);
 *   stack_alloc.Dump();
 *
 *
 *   // HEAP
 *   Allocator heap_alloc(HEAP);
 *   ValuePtr *cols = heap_alloc.GetObject<ValuePtr>(n);
 *   uchar *offset = heap_alloc.GetObject<uchar>(n);
 *   heap_alloc.Dump();
 *   heap_alloc.ReleaseObject(cols);
 *   heap_alloc.Dump();
 *   heap_alloc.ReleaseObject(offset);
 *   heap_alloc.Dump();
 *
 *   // PASS
 *   Allocator *alloc = new Allocator(type, scope);
 *   Cursor->alloc = alloc;
 * }
 **/
class Allocator final {
 public:
  Allocator(alloc_type type, alloc_scope scope);
  explicit Allocator(alloc_type type);

  ~Allocator() {
    if (p_alloc_) delete p_alloc_;
    p_alloc_ = nullptr;
  }

  /* for object */
  template <typename T, typename... Args>
  T *GetObject(Args &&... args) const {
    return p_alloc_->GetObject<T, Args...>(sizeof(T), std::forward<Args>(args)...);
  }
  /* for object * cnt */
  template <typename T, typename... Args>
  T *GetObject(int cnt, Args &&... args) const {
    return p_alloc_->GetObject<T, Args...>(sizeof(T) * cnt, std::forward<Args>(args)...);
  }

  // template <typename T> void ReleaseObject(T *obj) { p_alloc_->ReleaseObject(obj); }
  // 로 하면 ReleaseObject(obj) 가능하고 deleter도 필요 없지 않을까?
  void ReleaseObject(void *obj) const { p_alloc_->ReleaseObject(obj); }

  /* debug */
  void Dump() const { p_alloc_->Dump(); }

  inline const AllocatorImpl::size_type GetCount() const { return p_alloc_->GetCount(); }
  inline const alloc_type GetType() const { return p_alloc_->GetType(); }
  inline const alloc_scope GetScope() const { return p_alloc_->GetScope(); }

  Allocator() = delete;
  Allocator(Allocator &) = delete;
  Allocator(const Allocator &) = delete;
  Allocator &operator=(Allocator &) = delete;
  Allocator &operator=(const Allocator &) = delete;

 private:
  AllocatorImpl *p_alloc_;
};

}  // namespace test
}  // namespace executor

#endif /* no _XXXALLOCATOR_H_ */
