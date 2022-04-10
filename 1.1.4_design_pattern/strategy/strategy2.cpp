class Context {};

class ProStrategy {
public:
    virtual double CalcPro(const Context& ctx) = 0;
    virtual ~ProStrategy();
};

class VAC_Spring : public ProStrategy {
public:
    virtual double CalcPro(const Context& ctx) {}
};

class VAC_QIYI : public ProStrategy {
public:
    virtual double CalcPro(const Context& ctx) {}
};

class VAC_QIYI1 : public VAC_QIYI {
public:
    virtual double CalcPro(const Context& ctx) {}
};

class VAC_Wuyi : public ProStrategy {
public:
    virtual double CalcPro(const Context& ctx) {}
};

class VAC_Guoqing : public ProStrategy {
public:
    virtual double CalcPro(const Context& ctx) {}
};

class Promotion {
public:
    Promotion(ProStrategy* sss) : s(sss) {}
    ~Promotion(){}
    double CalcPromotion(const Context& ctx) {
        return s->CalcPro(ctx);
    }
private:
    ProStrategy* s;
};

int main() {
    Context ctx;
    ProStrategy* s = new VAC_QIYI1;
    Promotion* p = new Promotion(s);
    p->CalcPromotion(ctx);
    return 0;
}
