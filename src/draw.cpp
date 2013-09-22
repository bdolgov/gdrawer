#include "gdrawer.hpp"
#include <QThreadPool>
#include <QRunnable>
#include <QDebug>

namespace
{
	class Task : public QRunnable
	{
		private:
			QImage *img;
			QRectF rect;
			QRect viewport;
			Instrs *vm;
		public:
			Task(QImage* _img, const QRectF& _rect, const QRect& _viewport, Instrs *_vm);
			void run();
	};
}

QImage drawFormula(const QString& formula, const QRectF& rect, const QSize& viewport)
{
	Instrs vm = Instrs::get(formula);
	vm.dump();
	int threads = QThread::idealThreadCount();
	if (threads == -1) threads = 2;
	QImage ret(viewport, QImage::Format_Indexed8);
	qDebug() << "viewport: " << viewport;
	ret.setColor(0, qRgb(255, 255, 255));
	ret.setColor(1, qRgb(0, 0, 0));
	ret.setColor(2, qRgb(255, 255, 0));
	ret.fill(2);
	QThreadPool pool;
	pool.setMaxThreadCount(threads);
	QRectF rect1(rect);
	rect1.setHeight(rect1.height() / threads);
	QRect viewport1(QPoint(0, 0), viewport);
	viewport1.setHeight(viewport1.height() / threads);
	for (int i = 0; i < threads; ++i)
	{
		pool.start(new Task(&ret, rect1, viewport1, &vm));
		qDebug() << "Started pool" << rect1 << viewport1;
		rect1.moveTop(rect1.top() + rect1.height());
		viewport1.moveTop(viewport1.top() + viewport1.height());
	}
	pool.waitForDone();
	return ret;
}

Task::Task(QImage* _img, const QRectF& _rect, const QRect& _viewport, Instrs* _vm):
	img(_img), rect(_rect), viewport(_viewport), vm(_vm)
{
}

void Task::run()
{
	float_t dx = static_cast<float_t>(rect.width()) / viewport.width(),
	        dy = static_cast<float_t>(rect.height()) / viewport.height(),
			y = rect.top(),
			x = 0;
	int pxEnd = viewport.right() + 1, pyEnd = viewport.bottom() + 1;
	Ctx ctx(vm->requiredStackSize);
	qDebug() << viewport.top() << pyEnd;
	for (int py = viewport.top(); py != pyEnd; ++py, y += dy)
	{
		x = rect.left();
		uchar *line = img->scanLine(py);
		ctx.vars['y' - 'a'] = Real(y, y + dy);
		for (int px = 0; px != pxEnd; ++px, x += dx)
		{
			ctx.vars['x' - 'a'] = Real(x, x + dx);
			ctx.reset();
			Real res = vm->execute(&ctx);
			line[px] = res.isZero();
		}
	}
}


