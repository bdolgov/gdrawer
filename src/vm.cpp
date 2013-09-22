#include "gdrawer.hpp"
#include <QDebug>

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

void Instrs::dump()
{
	for (auto& i : *this)
	{
		qDebug() << i.type << char(i.arg + 'a') << double(i.val);
	}
}

Real Instrs::execute(Ctx* ctx)
{
	ctx->reset();
	Real a = 0, b = 0;
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
				b = ctx->pop(); a = ctx->pop();
				ctx->push(a + b);
				break;
			case '-':
				b = ctx->pop(); a = ctx->pop();
				ctx->push(a - b);
				break;
			case '*':
				b = ctx->pop(); a = ctx->pop();
				ctx->push(a * b);
				break;
			case '/':
				b = ctx->pop(); a = ctx->pop();
				ctx->push(a / b);
				break;
			case 'm':
				ctx->push(-ctx->pop());
				break;
			case '|':
				ctx->push(ctx->pop().abs());
				break;
			case '^':
				b = ctx->pop(); a = ctx->pop();
				ctx->push(a.pow(b));
				break;
			default:
				throw std::runtime_error("Unknown instruction");
		}
	}
	return ctx->pop();
}
