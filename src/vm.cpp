#include "gdrawer.hpp"
#include <QDebug>

void const_t::addInstr(MathVm* instrs) const
{
	instrs->emplace_back('C', 0, val);
}

void var_t::addInstr(MathVm* instrs) const
{
	instrs->emplace_back('V', name - 'a');
}

void binop_t::addInstr(MathVm* instrs) const
{
	if (op == '^')
	{
		/* Optimize a ^ 2 to a * a */
		if (auto c = dynamic_cast<const const_t*>(&*r))
		{
			int p = c->val;
			if (fabs(p - c->val) < EPS && 1 <= p && p <= 4)
			{
				l->addInstr(instrs);
				for (int i = 1; i < p; ++i)
				{
					instrs->emplace_back('D');
				}
				for (int i = 1; i < p; ++i)
				{
					instrs->emplace_back('*');
				}
				return;
			}
		}
	}

	if (op == '+' || op == '-')
	{
		/* Optimize subexpressions like |x| + x */
		if (auto l2 = dynamic_cast<const unop_t*>(&*l))
		{
			if (l2->l->equalsTo(&*r))
			{
				r->addInstr(instrs);
				instrs->emplace_back('D');
				instrs->emplace_back(l2->opcode());
				if (op == '-') instrs->emplace_back('S');
				instrs->emplace_back(op);
				return;
			}
		}
		
		if (auto r2 = dynamic_cast<const unop_t*>(&*r))
		{
			if (l->equalsTo(&*r2->l))
			{
				l->addInstr(instrs);
				instrs->emplace_back('D');
				instrs->emplace_back(r2->opcode());
				instrs->emplace_back(op);
				return;
			}
		}
	}
		
	l->addInstr(instrs);
	r->addInstr(instrs);
	instrs->emplace_back(opcode());
}

void unop_t::addInstr(MathVm* instrs) const
{
	l->addInstr(instrs);
	instrs->emplace_back(opcode());
}

bool const_t::equalsTo(const expr_t* other) const
{
	if (auto c = dynamic_cast<const const_t*>(other))
	{
		return fabs(val - c->val) < EPS;
	}
	return false;
}

bool var_t::equalsTo(const expr_t* other) const
{
	if (auto v = dynamic_cast<const var_t*>(other))
	{
		return name == v->name;
	}
	return false;
}

bool binop_t::equalsTo(const expr_t* other) const
{
	if (auto b = dynamic_cast<const binop_t*>(other))
	{
		return op == b->op && l->equalsTo(&*b->l) && r->equalsTo(&*b->r);
	}
	return false;
}

bool unop_t::equalsTo(const expr_t* other) const
{
	if (auto u = dynamic_cast<const unop_t*>(other))
	{
		return op == u->op && l->equalsTo(&*u->l);
	}
	return false;
}

void MathVm::dump()
{
	for (auto& i : *this)
	{
		qDebug() << i.type << char(i.arg + 'a') << double(i.val);
	}
}

Real MathVm::execute(Ctx* _ctx) const
{
	MathCtx *ctx = static_cast<MathCtx*>(_ctx);
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
			case 'D':
				ctx->push(ctx->top());
				break;
			case 'S':
				ctx->swap();
				break;
			default:
				throw Exception(QString("Unknown instruction: %1").arg(i.type));
		}
	}
	return ctx->pop();
}
