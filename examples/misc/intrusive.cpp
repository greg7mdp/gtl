#include <cstring>
#include <gtl/intrusive.hpp>

struct A : public gtl::intrusive_ref_counter<A, gtl::thread_safe_counter>
{
    virtual ~A() { std::cout << "A deleted" << std::endl; }
    int x;
};

struct D : public A
{
    ~D() { std::cout << "D deleted" << std::endl; }
    int y;
};

struct B : public gtl::intrusive_ref_counter<B, gtl::thread_unsafe_counter>
{
    ~B() { std::cout << "B deleted" << std::endl; }
    int x;
};

int main()
{
    {
        gtl::intrusive_ptr<A> a = new A;

        {
            gtl::intrusive_ptr<D> d = new D;
            {
                gtl::intrusive_ptr<A> d2 = d;
            }
        }
    }

    return 0;
}
