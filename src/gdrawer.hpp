#ifndef GDRAWER_HPP
#define GDRAWER_HPP

#include <memory>
#include <vector>
#include <ctype.h>
#include <QString>

typedef long double real_t;

struct Ctx
{
	std::unique_ptr<real_t[]> origStack;
	real_t *stack, *vars;

	Ctx(int stackSize, real_t *_vars): origStack(new real_t[stackSize]), stack(origStack.get()), vars(_vars) {}
	void reset() { stack = origStack.get(); }
	inline void push(real_t val)
	{
		*(++stack) = val;
	}
	inline real_t pop()
	{
		return *(stack--);
	}
};

struct Instr
{
	char type;
	char arg;
	real_t val;

	Instr(char _type, char _arg = 0, real_t _val = 0): type(_type), arg(_arg), val(_val) {}
};

struct Instrs : public std::vector<Instr>
{
	int requiredStackSize;
	real_t execute(Ctx* ctx);
	static Instrs get(const QString& expr);
};

struct expr_t
{
	virtual int getDepth() = 0;
	virtual void addInstr(Instrs* instrs) = 0;
	virtual ~expr_t() {}
};

struct const_t : expr_t
{
	real_t val;
	const_t(float_t _val): val(_val) {}
	int getDepth() { return 1; }
	void addInstr(Instrs* instrs);
};

struct var_t : expr_t
{
	char name;
	var_t(char _name): name(tolower(_name)) {}
	int getDepth() { return 1; }
	void addInstr(Instrs* instrs);
};

struct binop_t : expr_t
{
	char op;
	std::unique_ptr<expr_t> l, r;
	binop_t(char _op, expr_t* _l, expr_t* _r = NULL): op(_op), l(_l), r(_r) {}
	int getDepth() { return l->getDepth() + r->getDepth(); }
	void addInstr(Instrs* instrs);
};

struct unop_t : expr_t
{
	char op;
	std::unique_ptr<expr_t> l;
	unop_t(char _op, expr_t* _l): op(_op), l(_l) {}
	int getDepth() { return l->getDepth(); }
	void addInstr(Instrs* instrs);
};


#endif
