#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow), pLatexWorker(new LatexWorker(nullptr,true))
{
	ui->setupUi(this);

//    qDebug() << thread()  << pLatexWorker->thread();
//    QMetaObject::invokeMethod(pLatexWorker, "isLatexInstalled");

	QByteArray testJsonDocumentData = readTextFromFile(":/testJsonDocumentData.json");

	QSettings settings(ORGANIZATION_STRING, APPLICATION_STRING);
	settings.beginGroup("AppSettings");
	QString filename = settings.value("lastFileName","").toString();
	QString filenameOut = settings.value("lastOutFileName","").toString();
	if(!filename.isEmpty()) {
		if(QFile::exists(filename)) {
			if((QFile::permissions(filename) & QFileDevice::ReadUser) == QFileDevice::ReadUser) {
				ui->lineEdit->setText(filename);
			}
		}
	}
	ui->lineEdit_2->setText(filenameOut);
//	QString jsonData = settings.value("jsonData","").toString();
//	if(!jsonData.isEmpty()) {
//		ui->plainTextEditJsonData->setPlainText(jsonData);
//	} else {
		ui->plainTextEditJsonData->setPlainText(QString::fromUtf8(testJsonDocumentData));
//	}

	connect(pLatexWorker, &LatexWorker::standardOutputMsg, [&](QString text) {
		ui->plainTextEditLog->appendPlainText("------------------\n"+text);
	});
	connect(ui->lineEdit, &QLineEdit::textChanged, [&](QString text){
		QSettings settings(ORGANIZATION_STRING, APPLICATION_STRING);
		settings.beginGroup("AppSettings");
		settings.setValue("lastFileName", text);
		settings.endGroup();
	});
	connect(ui->lineEdit_2, &QLineEdit::textChanged, [&](QString text){
		QSettings settings(ORGANIZATION_STRING, APPLICATION_STRING);
		settings.beginGroup("AppSettings");
		settings.setValue("lastOutFileName", text);
		settings.endGroup();
	});
	connect(ui->plainTextEditJsonData, &QPlainTextEdit::textChanged, [&](){
		QSettings settings(ORGANIZATION_STRING, APPLICATION_STRING);
		settings.beginGroup("AppSettings");
		settings.setValue("jsonData", ui->plainTextEditJsonData->toPlainText());
		settings.endGroup();
	});
	connect(pLatexWorker, &LatexWorker::progress, [&](int line, int lines){
		ui->progressBar->setValue(static_cast<int>(static_cast<double>(line)/static_cast<double>(lines)*100.0));
	});

	qDebug() << ui->lineEdit->text().section(QDir::separator(),0,-2);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_pushButton_2_clicked()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Select TeX file"), QDir::homePath(), tr("LaTeX source (*.tex);;Text file without extension(*)"));	

	if(filename.isEmpty()) return;

	if((QFile::permissions(filename) & QFileDevice::ReadUser) != QFileDevice::ReadUser) {
		QMessageBox::warning(this, tr("Ошибка чтения"), tr("Нет доступа на чтение файла."));
		return;
	}

	ui->lineEdit->setText(filename);
}

void MainWindow::on_pushButton_clicked()
{
	pLatexWorker->setTempleteTexFile(ui->lineEdit->text());
	pLatexWorker->setStructure(ui->plainTextEditJsonData->toPlainText().toLocal8Bit());
	pLatexWorker->setStructureValue("NameCheckedDevice", "ВЕДРО-2");
	pLatexWorker->setStructureValue("TextCheckAim", "адских работ");
	pLatexWorker->writeStructure(ui->checkSecure->isChecked()); //TextCheckAim

	//qDebug() << "size: " << pLatexWorker->testProgressSize("test", true); //--> size: 1138
	pLatexWorker->initProgressSize("test", 1138);
	ui->progressBar->setValue(1);
	if(!ui->checkBox_Tex->isChecked()) {
		auto res = pLatexWorker->createPdf(ui->lineEdit_2->text(), "test", true);
		if(!res) ui->plainTextEditLog->appendPlainText("can't create Pdf-file");
		else ui->progressBar->setValue(100);
	} else {
		auto res = pLatexWorker->createTex(ui->lineEdit_2->text());
		if(!res) ui->plainTextEditLog->appendPlainText("can't create Tex-file");
		else ui->progressBar->setValue(100);
	}
}

QByteArray MainWindow::readTextFromFile(QString fileName) const
{
	QByteArray result = "";

	QFile file(fileName);
	if(!file.open(QIODevice::ReadOnly | QFile::Text))
		return result;

	QTextStream in(&file);
	auto cutf8 = QTextCodec::codecForName("UTF-8");
	in.setCodec(cutf8);

	result.append(in.readAll());

	return result;
}

void MainWindow::on_pushButton_3_clicked()
{
	QString filename = QFileDialog::getSaveFileName(this, tr("Write output file name"), QDir::homePath(), tr("Documents (*.pdf);;All files(*)"));
	ui->lineEdit_2->setText(filename);
}
