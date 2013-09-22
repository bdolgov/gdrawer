#include "gdrawer.hpp"
#include <QApplication>

int main(int ac, char** av)
{
	QApplication app(ac, av);
	MainWindow w;
	w.show();
	return app.exec();
}
