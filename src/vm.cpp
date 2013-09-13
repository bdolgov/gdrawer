#include "gdrawer.hpp"

void const_t::addInstr(Instrs* instrs)
{
	instrs->emplace_back('C', 0, val);
}

void var_t::addInstr(Instrs* instrs)
{
	instrs->emplace_back('V', name - 'a');
}

void binop_t::addInstr(Instrs* instrs)
{
	l->addInstr(instrs);
	r->addInstr(instrs);
	instrs->emplace_back(op);
}

void unop_t::addInstr(Instrs* instrs)
{
	l->addInstr(instrs);
	instrs->emplace_back(op == '-' ? 'm' : op);
}

real_t Instrs::execute(Ctx* ctx)
{
	ctx->reset();
	real_t a = 0, b = 0;
	for (auto& i : *this)
	{
		switch(i.type)
		{
			case 'C':
				ctx->push(i.val);
				break;
			case 'V':
				ctx->push(ctx->vars[static_cast<int>(i.arg)]);
				break;
			case '+':
				a = ctx->pop(); b = ctx->pop();
				ctx->push(a + b);
				break;
			case '-':
				a = ctx->pop(); b = ctx->pop();
				ctx->push(a - b);
				break;
			case '*':
				a = ctx->pop(); b = ctx->pop();
				ctx->push(a * b);
				break;
			case '/':
				a = ctx->pop(); b = ctx->pop();
				ctx->push(a / b);
				break;
			case 'm':
				ctx->push(-ctx->pop());
				break;
			case '|':
				ctx->push(fabsl(ctx->pop()));
				break;
			default:
				;
		}
	}
	return ctx->pop();
}