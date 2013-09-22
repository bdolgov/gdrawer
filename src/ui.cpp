#include "gdrawer.hpp"
#include <QtWidgets>

MainWindow::MainWindow()
{
	picture = new QLabel;

	QHBoxLayout *layout = new QHBoxLayout;
	layout->addWidget(picture, 1);

	QFormLayout *form = new QFormLayout;
	pathLabel = new QLabel;
	form->addRow(tr("GDrawer"), pathLabel);

	QPushButton *openButton = new QPushButton(tr("Open"));
	connect(openButton, SIGNAL(clicked()), this, SLOT(open()));
	form->addRow(openButton);

	x1 = new QLineEdit();
	y1 = new QLineEdit();
	x2 = new QLineEdit();
	y2 = new QLineEdit();
	resetRect();
	form->addRow("x1", x1);
	form->addRow("y1", y1);
	form->addRow("x2", x2);
	form->addRow("y2", y2);

	QPushButton *drawButton = new QPushButton(tr("Draw"));
	connect(drawButton, SIGNAL(clicked()), this, SLOT(draw()));
	form->addRow(drawButton);

	QPushButton *viewButton = new QPushButton(tr("View solution"));
	connect(viewButton, SIGNAL(clicked()), this, SLOT(view()));
	form->addRow(viewButton);

	layout->addLayout(form);
	setLayout(layout);

	setMinimumWidth(800);
	setMinimumHeight(600);
}

void MainWindow::resetRect()
{
	x1->setText("-100");
	y1->setText("-100");
	x2->setText("100");
	y2->setText("100");
}

void MainWindow::open()
{
	open(QFileDialog::getOpenFileName(this, tr("Open file"), "Text file (*.txt)"));
}

void MainWindow::open(QString name)
{
	QFile f(name);
	if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox::warning(this, tr("GDrawer"), tr("File not found"));
		return;
	}
	
	path = name;
	pathLabel->setText(QFileInfo(name).baseName());
}

QString MainWindow::getFormula(const QString& filename)
{
	QString formula;
	QFile f(filename);
	f.open(QIODevice::ReadOnly | QIODevice::Text);
	int lineNumber = 0;
	while (!f.atEnd())
	{
		QString line = QString::fromUtf8(f.readLine());
		if (line.startsWith("#!"))
		{
			line.remove(0, 2);
			QStringList parts = line.split(QRegExp("\\s+"));
			if (parts.size() != 4) continue;
			QLineEdit *order[] = { x1, y1, x2, y2 };
			for (int i = 0; i < 4; ++i)
			{
				bool ok = true;
				parts[i].toDouble(&ok);
				if (!ok)
				{
					QMessageBox::warning(this, tr("GDrawer"), tr("Rect sizes are invalid"));
					resetRect();
					continue;
				}
				order[i]->setText(parts[i]);
			}
		}
		if (line.startsWith('#'))
		{
			continue;
		}
		if (lineNumber) formula.append('\n');
		formula.append(line);
		++lineNumber;
	}
	return formula;
}

void MainWindow::draw()
{
	QRectF rect(
		QPointF(x1->text().toDouble(), y1->text().toDouble()),
		QPointF(x2->text().toDouble(), y2->text().toDouble()));

	picture->setPixmap(QPixmap::fromImage(drawFormula(getFormula(path), rect, picture->size())));
}

void MainWindow::view()
{
	auto form = new FileEditor(path);
	form->setAttribute(Qt::WA_DeleteOnClose);
	connect(form, SIGNAL(destroyed(QObject*)), this, SLOT(draw()));
	form->show();
}

FileEditor::FileEditor(const QString& _path): path(_path)
{
	QGridLayout *layout = new QGridLayout;

	QPushButton *saveButton = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton), "Save");
	connect(saveButton, SIGNAL(clicked()), this, SLOT(save()));
	layout->addWidget(saveButton, 0, 0);

	edit = new QTextEdit;
	QFile f(path);
	f.open(QIODevice::ReadOnly | QIODevice::Text);
	edit->setPlainText(QString::fromUtf8(f.readAll()));
	layout->addWidget(edit, 1, 0);

	setLayout(layout);
}

void FileEditor::save()
{
	QFile f(path);
	if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QMessageBox::warning(this, tr("GDrawer"), tr("Cannot open file for writing"));
		return;
	}
	f.write(edit->toPlainText().toUtf8());
}

void FileEditor::closeEvent(QCloseEvent* event)
{
}
