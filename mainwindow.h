#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <QDir>
#include <QStandardPaths>
#include <QTextCodec>

#include "latexworker.h"

#define ORGANIZATION_STRING "NIIEP"
#define DOMAIN_STRING "corp.nried.latex_worker_simple_app"
#define APPLICATION_STRING "LatexWorkerSApp"
#define VERSION_STRING "0.5"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private slots:
	void on_pushButton_2_clicked();
	void on_pushButton_clicked();
	
	void on_pushButton_3_clicked();

private:
	Ui::MainWindow *ui;

	LatexWorker *pLatexWorker;

	QByteArray readTextFromFile(QString fileName) const;
};

#endif // MAINWINDOW_H
