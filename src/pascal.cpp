#include "gdrawer.hpp"
#include <QTemporaryFile>
#include <QTextStream>
#include <QString>
#include <QRegularExpression>
#include <QProcess>
#include <dlfcn.h>
#include <QDir>

#if defined(Q_OS_OSX)
#define PREFIX "_"
#define FPC "./fpc.sh"
#define SO ".dylib"
#elif defined(Q_OS_LINUX)
#define PREFIX ""
#define FPC "./fpc.sh"
#define SO ".so"
#elif defined(Q_OS_WIN)
#define PREFIX "_"
#define FPC "fpc.bat"
#define SO ".dll"
#else
#error "This OS is not yet supported"
#endif

Vm* getPascalVm(const QString& prog)
{
	QTemporaryFile tmp(QDir::tempPath() + "/solution.XXXXXX.pas");
	if (!tmp.open())
		throw Exception("Cannot create temp file");

	QString vars, body;
	{
		int var = prog.indexOf(QRegularExpression("(\\s|^)var(\\s|$)", QRegularExpression::CaseInsensitiveOption), 0);
		int begin = prog.indexOf(QRegularExpression("(\\s|^)begin(\\s|$)", QRegularExpression::CaseInsensitiveOption), var);
		int end = prog.indexOf(QRegularExpression("(\\s|^)end.(\\s|$)", QRegularExpression::CaseInsensitiveOption), begin);
		if (var == -1 || begin == -1 || end == -1)
		{
			throw Exception("Var, Begin or End were not found");
		}
		vars = prog.mid(var + 4, begin - (var + 4));
		body = prog.mid(begin + 6, end - (begin + 6));
	}

	QTextStream s(&tmp);
	s << "library solution;\n"
	  << "function __r(__x, __y: real):boolean;\n"
	  << "cdecl;\n"
	  << "var " << vars << "\n"
	  << "begin\n"
	  << "x := __x; y := __y;\n"
	  << body << "\n"
	  << "__r := r;\n"
	  << "end;\n"
	  << "exports\n"
	  << "__r name '" << PREFIX << "pascal_run';\n"
	  << "end.\n";
	tmp.close();
	
	QProcess fpc;
	QTemporaryFile tmp1(QDir::tempPath() + "/solution.XXXXXX" SO);
	tmp1.open(); tmp1.close();

	fpc.start(FPC, QStringList() << tmp.fileName() << tmp1.fileName());
	fpc.waitForFinished();
	if (fpc.exitCode())
	{
		throw Exception(QString("fpc: %1").arg(QString::fromUtf8(fpc.readAllStandardError())));
	}

	PascalVm *ret = new PascalVm;
	ret->lib = dlopen(tmp1.fileName().toUtf8().data(), RTLD_LAZY | RTLD_LOCAL);
	if (!ret->lib)
	{
		delete ret;
		throw Exception(QString("dlopen(): %1").arg(dlerror()));
	}
	ret->fn = reinterpret_cast<char (*)(double, double)>(dlsym(ret->lib, "pascal_run"));
	if (!ret->fn)
	{
		delete ret;
		throw Exception(QString("dlsym(): %1").arg(dlerror()));
	}
	return ret;
}

Real PascalVm::execute(Ctx* _ctx) const
{
	PascalCtx *ctx = static_cast<PascalCtx*>(_ctx);
	return fn(ctx->x, ctx->y) ? Real(0, 0) : Real(1, 1);
}

PascalVm::~PascalVm()
{
	if (lib)
	{
		dlclose(lib);
	}
}