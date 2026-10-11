struct Base { virtual ~Base() {} virtual int id() const { return 0; } };
struct Mid : virtual Base { int id() const override { return 1; } };
struct Leaf : Mid { int id() const override { return 2; } };
Base *make_leaf() { return new Leaf; }
Base *make_base() { return new Base; }
