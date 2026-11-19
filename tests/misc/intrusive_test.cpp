#include "gtest/gtest.h"
#include <gtl/intrusive.hpp>

#include <utility>

namespace {

// abstract base, so Derived is not convertible to Base by value
struct Base : gtl::intrusive_ref_counter<Base, gtl::thread_unsafe_counter> {
    virtual ~Base()        = default;
    virtual int id() const = 0;
};

struct Derived : Base {
    int id() const override { return 42; }
};

} // namespace

TEST(IntrusivePtrTest, ConvertingCopyAssign) {
    gtl::intrusive_ptr<Derived> d(new Derived);
    gtl::intrusive_ptr<Base>    b;
    b = d;
    EXPECT_EQ(b.get(), d.get());
    EXPECT_EQ(2u, d->use_count());
    EXPECT_EQ(42, b->id());

    gtl::intrusive_ptr<Derived> other(new Derived);
    b = other;
    EXPECT_EQ(b.get(), other.get());
    EXPECT_EQ(1u, d->use_count());
    EXPECT_EQ(2u, other->use_count());
}

TEST(IntrusivePtrTest, ConvertingMoveConstructEmptiesSource) {
    gtl::intrusive_ptr<Derived> d(new Derived);
    Derived*                    raw = d.get();
    gtl::intrusive_ptr<Base>    b(std::move(d));
    EXPECT_EQ(nullptr, d.get());
    EXPECT_EQ(raw, b.get());
    EXPECT_EQ(1u, b->use_count());
}

TEST(IntrusivePtrTest, ConvertingMoveAssignEmptiesSource) {
    gtl::intrusive_ptr<Derived> d(new Derived);
    Derived*                    raw = d.get();
    gtl::intrusive_ptr<Base>    b;
    b = std::move(d);
    EXPECT_EQ(nullptr, d.get());
    EXPECT_EQ(raw, b.get());
    EXPECT_EQ(1u, b->use_count());
}

TEST(IntrusivePtrTest, SameTypeAssign) {
    gtl::intrusive_ptr<Derived> a(new Derived);
    gtl::intrusive_ptr<Derived> b;
    b = a;
    EXPECT_EQ(2u, a->use_count());
    b = std::move(a);
    EXPECT_EQ(nullptr, a.get());
    EXPECT_EQ(1u, b->use_count());
    auto& self = b;
    b          = self;
    EXPECT_EQ(1u, b->use_count());
}
