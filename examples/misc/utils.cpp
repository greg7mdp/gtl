#include <cassert>
#include <gtl/utils.hpp>

struct A : public gtl::timestamp
{
    bool set_x(int v) { return set_with_ts(x_, v); }
    int  x_{ 0 };
};

struct B : public gtl::timestamp
{
    bool set_y(int v) { return set_with_ts(y_, v); }
    int  y_{ 0 };
};

struct C : public gtl::provides_timestamp<C>
{
    gtl::timestamp get_timestamp() const { return a_ | b_; }

    A a_;
    B b_;
};

int main()
{
    // gtl::timestamp
    // --------------
    A a;
    B b;
    C c;

    a.touch();
    assert(a.is_newer_than(b));

    b.touch();
    assert(b.is_newer_than(a));

    c.a_.touch();
    assert(c.is_newer_than(b));

    c.a_.set_x(3);
    c.b_.set_y(3);

    // timestamp of c and c._b should be the same
    assert(!c.is_newer_than(c.b_) && !c.is_older_than(c.b_));
    assert(c.is_newer_than(c.a_));

    // scoped_set_unset
    // ----------------
    int x = 3;
    {
        gtl::scoped_set_unset set_x([&]() { x = 5; }, [&]() { x = 3; });
        assert(x == 5);
    }
    assert(x == 3);

    // scoped_set_value
    // ----------------
    {
        gtl::scoped_set_value set_x(x, 5);
        assert(x == 5);
    }
    assert(x == 3);

    return 0;
}
