/**
 * @file    test.h
 * @brief   TODO brief documentation here.
 *
 * @author
 * @version $Id$
 */

#ifndef _TEST_H_
#define _TEST_H_

/*{{{ Headers ------------------------------------------------------------------------------------*/
#include <memory>
/*------------------------------------------------------------------------------------ Headers }}}*/

/*{{{ Typedefs -----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------- Typedefs }}}*/

namespace test {
class A {
 private:
   using underlying_t = int;

 public:
   explicit A(const int a) : a_(a), p_(nullptr) {
     p_ = new char[3];
   }
   ~A() { delete p_; p_ = nullptr; }
   underlying_t get() { return a_; }

 private:
   underlying_t a_;
   char *p_;
};

class B : public A {
 public:
   B() : A(0) {}
   ~B() {}
};

}  // namespace test

#endif /* no _TEST_H_ */
