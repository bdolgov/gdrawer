#include "gdrawer.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/home/phoenix/object/new.hpp>
#include <stdexcept>

using namespace boost;
using namespace spirit;
using namespace ascii;
using namespace phoenix;

template<class Iterator>
struct ExprGrammar : qi::grammar<Iterator, expr_t*(), ascii::space_type>
{
	ExprGrammar(): ExprGrammar::base_type(expr, "Expression")
	{
		primitive.name("Primitive");
		factor.name("Factor");
		term.name("Term");
		expr.name("Expression");

		primitive 
			= ('(' >> expr >> ')')[_val = _1]
			| ('|' >> expr >> '|')[_val = new_<unop_t>('|', _1)]
			| (char_('a', 'z') | char_('A', 'Z'))[_val = new_<var_t>(_1)]
			| real[_val = new_<const_t>(_1)];

		factor
			= (primitive >> '^' >> factor)[_val = new_<binop_t>('^', _1, _2)]
			| primitive[_val = _1];
		
		term
			= factor[_val = _1] >> 
				*( (char_("/*") >> factor)[_val = new_<binop_t>(_1, _val, _2)] 
				| factor[_val = new_<binop_t>('*', _val, _1)] );
		
		expr
			= term[_val = _1] >> *(char_("+-") >> term)[_val = new_<binop_t>(_1, _val, _2)]
			| ('-' >> term)[_val = new_<unop_t>('-', _1)];
	}
	
	qi::rule<Iterator, expr_t*(), ascii::space_type> primitive, factor, term, expr;
	qi::real_parser<real_t> real;
};

Instrs Instrs::get(const QString& expr)
{
	expr_t* tree = NULL;
	std::string s = expr.toStdString();
	ExprGrammar<std::string::const_iterator> g;
	std::string::const_iterator begin = s.begin(), end = s.end();
	bool r = phrase_parse(begin, end, g, space, tree);
	if (!r || begin != end || !tree)
	{
		if (tree) delete tree;
		throw std::invalid_argument("Syntax error");
	}

	Instrs ret;
	tree->addInstr(&ret);
	ret.requiredStackSize = tree->getDepth();
	delete tree;
	return ret;
}
