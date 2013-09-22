#ifndef GDRAWER_HPP
#define GDRAWER_HPP

#include <memory>
#include <vector>
#include <array>
#include <ctype.h>
#include <QString>
#include <QWidget>
#include <QImage>

typedef double real_t;

#define EPS (1e-2)

extern real_t epsCoeff;

struct Ctx
{
	std::unique_ptr<real_t[]> origStack;
	std::array<float_t, 26> vars;
	real_t *stack;
	real_t eps;

	Ctx(int stackSize): origStack(new real_t[stackSize]), stack(origStack.get()), eps(EPS) {}
	void reset() { stack = origStack.get(); }
	inline void push(real_t val)
	{
		*(stack++) = val;
	}
	inline real_t pop()
	{
		return *(--stack);
	}
	inline real_t top()
	{
		return *(stack - 1);
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
	void dump();
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

class QLabel;
class QLineEdit;

class MainWindow : public QWidget
{
	Q_OBJECT
	private:
		QLabel *picture;
		QLineEdit *x1, *y1, *x2, *y2;
		QLabel *pathLabel;
		QString formula;
		QString path;
		void resetRect();

	public slots:
		void open();
		void open(QString path);
		void draw();
		void view();

	public:
		MainWindow();
};

QImage drawFormula(const QString& formula, const QRectF& rect, const QSize& viewport);

#endif
