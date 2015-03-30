#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN 1
#include <windows.h>
#endif

#include <stdio.h>
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

class Foo {
public:
  Foo() : name_(L"hello"), yes_(L"sure") {
    fwprintf(stderr, L"Foo: From the ctor %p\n", this);
  }
  Foo(const Foo &other) : name_(other.name_), yes_(other.yes_) {}
  Foo &operator=(const Foo &other) {
    this->name_ = other.name_;
    this->yes_ = other.yes_;
    return *this;
  }
  virtual ~Foo() { fwprintf(stderr, L"Foo: I'm dead %p\n", this); }

  std::wstring name_, yes_;
};

std::unique_ptr<Foo> inst;
std::once_flag create;

void do_once() {
  fwprintf(stderr, L"Hello from thread 0x%x\n", std::this_thread::get_id());
  // call_once( create, [=]{ inst = std::make_unique<Foo>(); } );
  call_once(create, [=] { inst = std::unique_ptr<Foo>(new Foo); });
}

int main() {
  fwprintf(stderr, L"Main\n");
  std::thread t0(do_once);
  std::thread t1(do_once);
  std::thread t2(do_once);
  std::thread t3(do_once);
  t0.join();
  t1.join();
  t2.join();
  t3.join();

  fwprintf(stderr, L"Foo: %s %s\n", inst->name_.c_str(), inst->yes_.c_str());
  return 0;
}
