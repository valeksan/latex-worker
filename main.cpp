#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QGuiApplication::setOrganizationName(ORGANIZATION_STRING);
	QGuiApplication::setOrganizationDomain(DOMAIN_STRING);
	QGuiApplication::setApplicationName(APPLICATION_STRING);
	QGuiApplication::setApplicationVersion(VERSION_STRING);

	MainWindow w;
	w.show();

	return a.exec();
}
