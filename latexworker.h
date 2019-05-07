#ifndef LATEXWORKER_H
#define LATEXWORKER_H

#include <QImage>
#include <QObject>
#include <QThread>
#include <QProcess>
#include <QDir>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QRegularExpression>
#include <QHash>
#include <QPair>
#include <QTemporaryFile>
#include <QStandardPaths>
#include <QTextCodec>
#include <QDateTime>
#include <QFloat16>
#include <QDebug>

#include "qcompressor.h"
#include "qcrypter.h"
#include "qqrcodereader.h"

#define INF -1

#ifndef DEF_MAX_DATABYTES_IN_QRCODE
#define DEF_MAX_DATABYTES_IN_QRCODE 1000
#endif

class LatexWorker : public QObject
{
	Q_OBJECT

	Q_PROPERTY(int lastErrorType READ lastErrorType WRITE setLastErrorType NOTIFY lastErrorTypeChanged)
	Q_PROPERTY(int maxNumBytesInQrCode READ maxNumBytesInQrCode WRITE setMaxNumBytesInQrCode NOTIFY maxNumBytesInQrCodeChanged)


public:
	explicit LatexWorker(QObject *parent = nullptr, bool newThread = false);
	~LatexWorker();

	enum ErrorType {
		ERR_OK = 0,
		ERR_SYNTAX,
		ERR_SYNTAX_EXT_1,
		ERR_LATEX_NOT_INSTALLED,
		ERR_FAIL_WRITE_TEX_FILE,
	} currentErrorType = ERR_OK;

	Q_INVOKABLE bool isLatexInstalled();												// проверка на то, что среда LaTeX установленна

	Q_INVOKABLE int testProgressSize(QString progressInitKey = "", bool init = false);	// применяется для исследования шаблона документа с установленной структурой, оценка числа выводимых строк в потоке вывода, для оценки прогресса работы с файлом для формирования pdf
	Q_INVOKABLE void initProgressSize(QString progressInitKey, int size);				// для корректного отображения прогресса работы с файлом при формировании pdf

	Q_INVOKABLE void setTempleteTexFile(QString texFileName);							// <step 1> - установка шаблона документа (путь к файлу)

	Q_INVOKABLE bool setStructure(QByteArray jsonDocumentData,
								  bool update = false,
								  bool simplified = false);								// <step 2> - установка структуры документа через json (набор ключевых переменных со значениями по умолчанию, в соответствии со страницами)
	Q_INVOKABLE bool setStructure(QList<QImage> pages, bool simplified = false);		// <step 2> - установка структуры документа через сканы страниц, а именно через данные хранящиеся в QR-кодах, выдает истину в случае если данные считались и корректны

	Q_INVOKABLE bool setStructureValue(QString key, QString value);						// <step K : [2<K<(N-1)] > - замена значения в структуре по умолчанию на новое значение, редактирование записанной структуры
	Q_INVOKABLE bool writeStructure(bool protect = false,
									QString variblesBlockMark = "upd",
									QString qrBlockMark = "qrcodes");					// <step N-1> - запись структуры в шаблон, формирование временного файла, нужного для формирования в дальнейшем pdf
	Q_INVOKABLE void insertTexCodeByMark(QString mark, QString latexCode);				// <step N-1> - вставка LaTex кода в будующий tex-файл (модификация считанного исходного шаблона).
	Q_INVOKABLE void clearStructure();
	Q_INVOKABLE void fillStructure(QString sep = "");
	Q_INVOKABLE bool createPdf(QString pdfFileName,
							   QString progressInitKey="",
							   bool cleanup = true,
							   bool make_tex_if_texlive_not_installed = true);			// <step N> (last) - метод формирования Pdf с нужным именем в требуемом каталоге
	Q_INVOKABLE bool createTex(QString texFileName);									// <step N> - для экспорта можифицированного tex-файла (можно использовать при отсутствии пакета texlive)

	Q_INVOKABLE QByteArray exportConvTableForSimplifyPages();							// экспорт таблицы конвертации ключей из строковых в номерные (чтобы сохранить гденибудь и использовать в дальнейшем)
	Q_INVOKABLE bool importConvTableForSimplifyPages(QByteArray jsonConvTable);			// применяется для правильной отработки метода setStructure в случае если там стоит параметр simplified (т.е. входная структура содержит номерные ключи которые нужно сконвертить)

	int lastErrorType() const
	{
		return m_lastErrorType;
	}

	int maxNumBytesInQrCode() const
	{
		return m_maxNumBytesInQrCode;
	}

public slots:
	void slotCreatePdf(QString pdfFileName, QString progressInitKey) {
		createPdf(pdfFileName, progressInitKey, true, true);
	}

private:
	// for generate tex-file
	bool insertJsonDataInTemplete(QString variblesBlockMark);
	bool insertQrCodesInTemplete(bool protect, QString variblesBlockMark);
	bool makeTempleteTexFile();

	// to simplify get data
	QString getKeyByNCounter(int n) const;
	int getNPageByNCounter(int n) const;
	int getNCounterByKey(QString key) const;
	QByteArray getJsonPage(int pageNumber, bool simplify = false) const;

	// util methods
	QByteArray getSimplifyJsonPage(int pageNumber) const;
	QByteArray convertSimplifyJsonToReadFmt(QByteArray importedJson);
	QByteArray zip(QByteArray input, int level = -1);
	QByteArray zip(int pageNumber, bool simplify = true, int level=-1);
	QByteArray unzip(QByteArray input);
	QByteArray crypt(QByteArray input);
	QByteArray crypt(int pageNumber, bool simplify = true, bool compress = true, int compress_level = -1);
	QByteArray decrypt(QByteArray input, bool decompress = true);
	QByteArray readTextFromFile(QString fileName) const;
	bool writeTextToFile(QString text, QString fileName);
	bool readJsonData(QByteArray jsonData, bool simplified = false, bool update = false);
	bool readSimplifyJsonData(QByteArray jsonData, bool update = false);

signals:
	void standardOutputMsg(QString text);
	void errorOutputMsg(QString text);
	void progress(int,int); // <num_completed, size>
	void complete(QString, int); // <fileName, errorType>

	void lastErrorTypeChanged(int lastErrorType);

	void maxNumBytesInQrCodeChanged(int maxNumBytesInQrCode);

public slots:

void setLastErrorType(int lastErrorType)
{
	if (m_lastErrorType == lastErrorType)
		return;

	m_lastErrorType = lastErrorType;
	emit lastErrorTypeChanged(m_lastErrorType);
}

void setMaxNumBytesInQrCode(int maxNumBytesInQrCode)
{
	if (m_maxNumBytesInQrCode == maxNumBytesInQrCode)
		return;

	m_maxNumBytesInQrCode = maxNumBytesInQrCode;
	emit maxNumBytesInQrCodeChanged(m_maxNumBytesInQrCode);
}

private:
	QString m_templateTexFile;
	QList<QHash<QString,QString> > m_listDataPages;	// list:index=page_number; Hash {key, value}

	QHash<QString,int> m_keyConvTable;
	QHash<QString,int> m_keyByPageNumberTable;

	QString m_templateTex; // readed large tex-file

	QString m_tempfilePath;
	QList<QByteArray> m_listJsonItems;

	QHash<QString,int> m_progressCorrector;
	int m_lastErrorType;
	int m_maxNumBytesInQrCode;
};

#endif // LATEXWORKER_H
