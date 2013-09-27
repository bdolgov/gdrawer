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
	open(":/demo.txt");
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
	open(QFileDialog::getOpenFileName(this, tr("Open file"), "", "Text file (*.txt)"));
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
	pathLabel->setText(QFileInfo(name).fileName());
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
			QStringList parts = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
			qDebug() << parts;
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
					break;
				}
				order[i]->setText(parts[i]);
			}
			continue;
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
	try
	{
		QString f = getFormula(path); /* to set x1, x2, y1, y2 */

		QRectF rect(
			QPointF(x1->text().toDouble(), y1->text().toDouble()),
			QPointF(x2->text().toDouble(), y2->text().toDouble()));

		picture->setPixmap(QPixmap::fromImage(drawFormula(f, rect, picture->size())));
	}
	catch (Exception e)
	{
		QMessageBox::critical(this, tr("Error"), e.what());
	}
}

void MainWindow::view()
{
	auto form = new FileEditor(path);
	form->setAttribute(Qt::WA_DeleteOnClose);
	connect(form, SIGNAL(saved()), this, SLOT(draw()));
	form->show();
}

FileEditor::FileEditor(const QString& _path): path(_path), modified(false)
{
	QVBoxLayout *layout = new QVBoxLayout;
	
	QHBoxLayout *tools = new QHBoxLayout;
	QPushButton *saveButton = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton), tr("Save"));
	connect(saveButton, SIGNAL(clicked()), this, SLOT(save()));
	tools->addWidget(saveButton);

	QPushButton *exitButton = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_DialogCloseButton), tr("Exit"));
	connect(exitButton, SIGNAL(clicked()), this, SLOT(close()));
	tools->addWidget(exitButton);

	layout->addLayout(tools);

	edit = new QTextEdit;
	QFile f(path);
	f.open(QIODevice::ReadOnly | QIODevice::Text);
	edit->setPlainText(QString::fromUtf8(f.readAll()));
	connect(edit, &QTextEdit::textChanged, [this]() { this->modified = true; });
	layout->addWidget(edit);

	setLayout(layout);
	setMinimumWidth(640);
	setMinimumHeight(480);
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
	modified = false;
	f.close();
	emit saved();
}

void FileEditor::closeEvent(QCloseEvent* event)
{
	if (modified)
	{
		QMessageBox::StandardButton ans = QMessageBox::question(this,
			tr("Save"), tr("Do you want to save file?"),
			QMessageBox::Save | QMessageBox::Cancel | QMessageBox::Discard);
		if (ans == QMessageBox::Save)
		{
			save();
		}
		else if (ans == QMessageBox::Cancel)
		{
			event->ignore();
		}
	}
}
