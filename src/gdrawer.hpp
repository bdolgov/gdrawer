#ifndef GDRAWER_HPP
#define GDRAWER_HPP

#include <memory>
#include <vector>
#include <array>
#include <ctype.h>
#include <QString>
#include <QWidget>
#include <QImage>
#include <cmath>
#include <QComboBox>

typedef double real_t;

#define EPS (1e-9)

class Exception
{
	public:
		Exception(const QString& _what): m_what(_what) {}
		QString what() const { return m_what; }
		void append(const QString& _what) { m_what.append(_what); }
	private:
		QString m_what;
};

struct RangeReal
{
	real_t min, max;
	RangeReal(): min(0), max(0) {}
	RangeReal(real_t val): min(val), max(val) {}
	RangeReal(real_t _min, real_t _max): min(_min), max(_max) {}
	RangeReal operator+(const RangeReal& other) const
	{
		return RangeReal(min + other.min, max + other.max);
	}
	RangeReal operator-(const RangeReal& other) const
	{
		return RangeReal(min - other.max, max - other.min);
	}
	RangeReal operator-() const
	{
		return RangeReal(-max, -min);
	}
	RangeReal operator*(const RangeReal& other) const
	{
		real_t a = min * other.min, b = min * other.max,
			   c = max * other.min, d = max * other.max;
		return RangeReal(std::min({a, b, c, d}), std::max({a, b, c, d}));
	}
	RangeReal operator/(const RangeReal& other) const
	{
		if (other.isZero())
		{
			throw Exception("Division by zero");
		}
		real_t a = min / other.min, b = min / other.max,
			   c = max / other.min, d = max / other.max;
		return RangeReal(std::min({a, b, c, d}), std::max({a, b, c, d}));
	}
	RangeReal pow(const RangeReal& other) const
	{
		if (min <= EPS && max >= -EPS)
		{
			return RangeReal(0, std::pow(std::max(-min, max), other.max));
		}
		else if (max >= 0)
		{
			return RangeReal(std::pow(min, other.min), std::pow(max, other.max));
		}
		else
		{
			int o = other.max;
			if (std::fabs(other.max - o) > EPS)
			{
				throw Exception("Attempted to calculate a^b, a<0 and b is not integer.");
			}
			if (o % 2 == 0)
			{
				return RangeReal(std::pow(max, o), std::pow(min, o));
			}
			else
			{
				return RangeReal(std::pow(min, o), std::pow(max, o));
			}
		}
	}
	RangeReal abs() const
	{
		if (min <= EPS && max >= -EPS)
		{
			return RangeReal(0, std::max(-min, max));
		}
		else if (max >= 0)
		{
			return RangeReal(min, max);
		}
		else
		{
			return RangeReal(-max, -min);
		}
	}
	bool isZero() const
	{
		return min <= EPS && max >= -EPS;
	}
};

typedef RangeReal Real;

struct Ctx
{
	virtual void reset() = 0;
	virtual void setVar(char name, Real value) = 0;
	virtual ~Ctx() {}
};

struct Vm
{
	virtual Ctx *createCtx() const = 0;

	virtual Real execute(Ctx* ctx) const = 0;
	virtual ~Vm() {}
};

struct MathCtx : Ctx
{
	std::unique_ptr<Real[]> origStack;
	std::array<Real, 26> vars;
	Real *stack;

	MathCtx(int stackSize): origStack(new Real[stackSize]), stack(origStack.get()) {}
	void reset() { stack = origStack.get(); }
	inline void push(const Real& val)
	{
		*(stack++) = val;
	}
	inline Real pop()
	{
		return *(--stack);
	}
	inline Real top()
	{
		return *(stack - 1);
	}
	inline void swap()
	{
		std::swap(*(stack - 1), *(stack - 2));
	}
	void setVar(char name, Real value)
	{
		vars[name - 'a'] = value;
	}
};

struct Instr
{
	char type;
	char arg;
	real_t val;

	Instr(char _type, char _arg = 0, real_t _val = 0): type(_type), arg(_arg), val(_val) {}
};

struct MathVm : Vm, std::vector<Instr>
{
	int requiredStackSize;
	Ctx* createCtx() const { return new MathCtx(requiredStackSize); }
	Real execute(Ctx* ctx) const;
	static Vm *get(const QString& expr);
	void dump();
};

struct expr_t
{
	virtual int getDepth() const = 0;
	virtual void addInstr(MathVm* instrs) const = 0;
	virtual ~expr_t() {}
	virtual bool equalsTo(const expr_t* other) const = 0;
	virtual char opcode() const = 0;
};

struct const_t : expr_t
{
	real_t val;
	const_t(float_t _val): val(_val) {}
	int getDepth() const { return 1; }
	void addInstr(MathVm* instrs) const;
	bool equalsTo(const expr_t* other) const;
	char opcode() const { return 'C'; }
};

struct var_t : expr_t
{
	char name;
	var_t(char _name): name(tolower(_name)) {}
	int getDepth() const { return 1; }
	void addInstr(MathVm* instrs) const;
	bool equalsTo(const expr_t* other) const;
	char opcode() const { return 'V'; }
};

struct binop_t : expr_t
{
	char op;
	std::unique_ptr<expr_t> l, r;
	binop_t(char _op, expr_t* _l, expr_t* _r = NULL): op(_op), l(_l), r(_r) {}
	int getDepth() const { return 1 + std::max(l->getDepth(), r->getDepth()); }
	void addInstr(MathVm* instrs) const;
	bool equalsTo(const expr_t* other) const;
	char opcode() const { return op; }
};

struct unop_t : expr_t
{
	char op;
	std::unique_ptr<expr_t> l;
	unop_t(char _op, expr_t* _l): op(_op), l(_l) {}
	int getDepth() const { return l->getDepth(); }
	void addInstr(MathVm* instrs) const;
	bool equalsTo(const expr_t* other) const;
	char opcode() const { return op == '-' ? 'm' : op; }
};

class QLabel;
class QLineEdit;
class QTextEdit;
class QCloseEvent;

class MainWindow : public QWidget
{
	Q_OBJECT
	private:
		QLabel *picture;
		QLineEdit *x1, *y1, *x2, *y2;
		QLabel *pathLabel;
		QString path;
		void resetRect();
		QString getFormula(const QString& filename);
		QComboBox *type;

	public slots:
		void open();
		void open(QString path);
		void draw();
		void view();

	public:
		MainWindow();
};

class FileEditor : public QWidget
{
	Q_OBJECT
	private:
		QTextEdit *edit;
		QString path;
		bool modified;
	
	protected:
		void closeEvent(QCloseEvent *event);

	public slots:
		void save();

	signals:
		void saved();

	public:
		FileEditor(const QString& _path);
};

QImage drawFormula(Vm* vm, const QRectF& rect, const QSize& viewport);

struct PascalCtx : Ctx
{
	double x, y;
	void reset() {} 
	void setVar(char name, Real value) { (name == 'x' ? x : y) = value.min; }
};

struct PascalVm : Vm
{
	void *lib;
	char (*fn)(double, double);
	Ctx *createCtx() const { return new PascalCtx; }
	Real execute(Ctx*) const;
	~PascalVm();
};

Vm* getPascalVm(const QString& program);
Vm* getCppVm(const QString& program);

#endif
