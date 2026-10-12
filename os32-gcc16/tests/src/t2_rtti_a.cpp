// t2_rtti: dynamic_cast and typeid across translation units, virtual inheritance.
#include <cstdio>
#include <cstring>
#include <typeinfo>
struct Base { virtual ~Base() {} virtual int id() const { return 0; } };
struct Mid : virtual Base { int id() const override { return 1; } };
struct Leaf : Mid { int id() const override { return 2; } };
Base *make_leaf();   // t2_rtti_b.cpp
Base *make_base();
int main()
{
    int fails = 0;
    Base *l = make_leaf(), *b = make_base();
    Leaf *pl = dynamic_cast<Leaf *>(l); Mid *pm = dynamic_cast<Mid *>(b);
    if (!pl || pl->id() != 2) { std::puts("FAIL dynamic_cast to Leaf"); ++fails; } else std::puts("PASS dynamic_cast to Leaf");
    if (pm) { std::puts("FAIL dynamic_cast of a plain Base"); ++fails; } else std::puts("PASS dynamic_cast of a plain Base");
    if (typeid(*l) != typeid(Leaf)) { std::puts("FAIL typeid across units"); ++fails; } else std::puts("PASS typeid across units");
    if (!std::strstr(typeid(*l).name(), "Leaf")) { std::puts("FAIL typeid name"); ++fails; } else std::puts("PASS typeid name");
    try { (void)dynamic_cast<Leaf &>(*b); std::puts("FAIL bad_cast"); ++fails; } catch (const std::bad_cast &) { std::puts("PASS bad_cast"); }
    delete l; delete b;
    std::puts(fails ? "t2 FAILED" : "t2 OK");
    return fails ? 20 : 0;
}
