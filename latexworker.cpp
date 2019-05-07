#include "latexworker.h"
#include <QDebug>

LatexWorker::LatexWorker(QObject *parent, bool newThread) : QObject(parent),
	m_templateTex(""),
	m_maxNumBytesInQrCode(DEF_MAX_DATABYTES_IN_QRCODE)
{
	if(newThread) {
		moveToThread(new QThread());
		connect(thread(), &QThread::started, this, [=]
		{
			//...
		});
		thread()->start();
	}
//	QByteArray text = "text текст text text text text text text text text text text text";
//	qDebug() << "Text: " << text;
//	QByteArray encText;
//	QCrypter::encrypt(text, encText);
//	qDebug() << "Encrypted text:" << encText;
//	QByteArray decText;
//	QCrypter::decrypt(encText, decText);
//	qDebug() << "Decrypted text:" << decText;
//	QByteArray zipText;
//	QCompressor::gzipCompress(text, zipText, 1);
//	qDebug() << "Zipped text:" << zipText << "size: " << zipText.size();
//	QByteArray unzippedText;
//	QCompressor::gzipDecompress(zipText, unzippedText);
//	qDebug() << "Unzipped text:" << unzippedText << "size: " << unzippedText.size();;

//	QByteArray key;
//	QCrypter::rsa_read_private_key(QString("/home/vi/.ssh/id_rsa"), key);
//	qDebug() << "key: " << key;

//	QByteArray data1 = "hello world!";
//	QByteArray data2 = "apple";
//	QByteArray hash1;
//	QByteArray hash2;
//	QCrypter::sha1(data1, hash1);
//	QCrypter::sha1(data2, hash2);

//	qDebug() << data1 << " : " << hash1 << endl;
//	qDebug() << data2 << " : " << hash2 << endl;

//	FileSource fs1("/home/vi/.ssh/id_rsa", true);


//	QList<QString> listQrCode;
//	if(QQrCodeReader::scan("/home/vi/test2.png", listQrCode, 1.5, true)) {
//		qDebug() << "YEAH!";
//		qDebug() << "list_data: " << listQrCode;
//	} else {
//		qDebug() << "OH NO!";
//		qDebug() << "list_data: " << listQrCode;
//	}
}

LatexWorker::~LatexWorker()
{

}

using namespace CryptoPP;

bool LatexWorker::isLatexInstalled()
{
	bool res = false;
	QProcess proc;
#ifdef Q_OS_UNIX
	proc.start("which pdflatex");
	proc.waitForFinished(INF);
	if(proc.readAllStandardOutput().contains("pdflatex")) res = true;
#elif defined(Q_OS_WIN)
	proc.start("where pdflatex");
	proc.waitForFinished(INF);
	if(proc.readAllStandardOutput().contains("pdflatex")) res = true;
#elif
	//not supported
#endif
	return res;
}

bool LatexWorker::createPdf(QString pdfFileName, QString progressInitKey, bool cleanup, bool make_tex_if_texlive_not_installed)
{
	bool isTexFormatOutput = false;

	QString pdfFileNameArg = pdfFileName.section(QDir::separator(),-1,-1);
	QString outputDirArg = pdfFileName.section(QDir::separator(),0,-2);

	QProcess proc;
	bool result = false;
//	QString consoleErr;

	if(!isLatexInstalled()) {
		m_lastErrorType = ERR_LATEX_NOT_INSTALLED;
		isTexFormatOutput = true;
		qDebug() << "error: pdflatex not found!";
		if(!make_tex_if_texlive_not_installed) {
			emit complete("", ERR_LATEX_NOT_INSTALLED);
			return false;
		}
	}
	// создание временного файла из считанного шаблона
	if(!makeTempleteTexFile()) {
		qDebug() << "error: no make templete tex-file!";
		m_lastErrorType = ERR_FAIL_WRITE_TEX_FILE;
		emit complete("", ERR_FAIL_WRITE_TEX_FILE);
		return false;
	}

	int num_lines = m_templateTex.count('\n');
	int progress_counter = 0;
	emit progress(0, num_lines);

	// fix input arg(outputFileName):
	QString suffix = pdfFileNameArg.section('.', -1, -1);
	//qDebug() << "suffix: " << suffix;
	if(!isTexFormatOutput) {
		if(suffix.isEmpty() || suffix != "pdf") {
			pdfFileNameArg.append(".pdf");
		}
	} else {
		if(suffix.isEmpty() || suffix != "tex") {
			QString pdfSuffix = QString(".pdf");
			if(pdfFileNameArg.endsWith(pdfSuffix)) {
				pdfFileNameArg.remove(pdfFileNameArg.size()-pdfSuffix.size(), pdfSuffix.size());
			}
			pdfFileNameArg.append(".tex");
		}
	}
	// -

	// fix input arg(outputDir):
	if(outputDirArg.isEmpty()) {
		QFileInfo info(pdfFileNameArg);
		outputDirArg = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
		//qDebug() << "outputDir: " <<outputDir;
	}
	// -

	// make output signals
	connect(&proc, &QProcess::readyRead, [&]() {
		QString consoleOut = proc.readAll();
		ErrorType currentErrorType = ERR_OK;
		if(consoleOut.endsWith("*")) {
			//qDebug() << "KILL!";
			proc.kill();
			currentErrorType = ERR_SYNTAX_EXT_1;
		}
		QString errorSegment = consoleOut.section("Error:", 1, -1).simplified();
		if(!errorSegment.isEmpty()) {
			//qDebug() << errorSegment;
			proc.kill();
			currentErrorType = ERR_SYNTAX;
		}
		if((m_progressCorrector.value(progressInitKey, -1) == -1) || progressInitKey.isEmpty()) {
			QString foundProgress = consoleOut.section(QRegExp("lines \\d{1,}--"),1,1).section("\n",0,0);
			//qDebug() << foundProgress;
			int tmp_progress_counter = foundProgress.toInt();
			//qDebug() << tmp_progress_counter;
			if(tmp_progress_counter > 0) {
				int new_progress = ((tmp_progress_counter > progress_counter) ? tmp_progress_counter : progress_counter);
				if(new_progress > progress_counter) {
					progress_counter = new_progress;
					//qDebug() << "progress: " << progress_counter/num_lines*1.0;
					emit progress(progress_counter, num_lines);
				}
			}
		} else {
			progress_counter += consoleOut.count('\n');
			emit progress(progress_counter, m_progressCorrector.value(progressInitKey, progress_counter));
		}
		switch (currentErrorType) {
		case ERR_OK:
//			consoleErr = "";
			result = true;
			break;
		case ERR_SYNTAX_EXT_1:
//			consoleErr = "Incomplete block segment found!";
			break;
		case ERR_SYNTAX:
//			consoleErr = consoleOut.section("Error:", 1, -1).simplified();
			break;
		default:
			qDebug() << "this block will never be in use!";
			break;
//		case ERR_LATEX_NOT_INSTALLED:
//			consoleErr = "Latex is not installed.";
//			break;
//		case ERR_FAIL_WRITE_TEX_FILE:
//			break;
		}
		emit standardOutputMsg(consoleOut);
	});
	// -

	// make command:
	QString cmd = QString("latexmk -pdf -silent -verbose \"%1\"").arg(m_tempfilePath);
	//qDebug() << "cmd: " << cmd;
	// -

	// set paths
	QString tmpPath = m_tempfilePath.section(QDir::separator(), 0, -2);
	//qDebug() << "tmpPath: " << tmpPath;
	proc.setWorkingDirectory(tmpPath);

	// make full output path:
	QString baseTmpFileNamesPath = m_tempfilePath.section(QDir::separator(),0,-2)+QDir::separator()+m_tempfilePath.section('.',-2,-2).section(QDir::separator(),-1,-1);
	QString genFilePath = baseTmpFileNamesPath + (!isTexFormatOutput ? ".pdf" : ".tex"); //QString("%1").arg(tmpFileName.section(QDir::separator(),-1,-1));
	//qDebug() << "outputMakeFile: " << genFilePath;
	//QString finalOutputMakeFile = QDir(outputDir).path() + QDir::separator() + genFile.section(QDir::separator(),-1,-1);
	//qDebug() << "finalOutputMakeFile: " << finalOutputMakeFile;
	QString outputFileNamePath = QDir(outputDirArg).path() + QDir::separator() + pdfFileNameArg;
	//qDebug() << "outputFileNameFullPath: " << genFilePath;

	if(!isTexFormatOutput) {
		// start working commands:
		QFile::remove(baseTmpFileNamesPath+".log");
		QFile::remove(baseTmpFileNamesPath+".aux");
		QFile::remove(baseTmpFileNamesPath+".fls");
		QFile::remove(baseTmpFileNamesPath+".fdb_latexmk");
		QFile::remove(baseTmpFileNamesPath+".wajqrQ");
		QFile::remove(baseTmpFileNamesPath+".cIDnXX");

		proc.start(QString("latexmk -c"));
		proc.waitForFinished(INF);
		proc.start(cmd);
		proc.waitForFinished(INF);
		proc.disconnect();
		emit progress(num_lines, num_lines);
	}

	// copy file from template directory to output directory:
	QFile tmp(m_tempfilePath);
	if(tmp.exists()) {
		if(QFile::exists(outputFileNamePath)) QFile::remove(outputFileNamePath);
		if(!isTexFormatOutput) {
			QFile::rename(genFilePath, outputFileNamePath);
		} else {
			QFile::rename(m_tempfilePath, outputFileNamePath);
		}
		//qDebug() << "file renamed!";
	}

	// clean:
	if(cleanup && !isTexFormatOutput) {
		proc.start(QString("latexmk -c"));
		proc.waitForFinished(INF);
		QFile::remove(m_tempfilePath);
		//qDebug() << "remove " << m_tempfilePath;
		QFile::remove(baseTmpFileNamesPath+".pdf");
		QFile::remove(baseTmpFileNamesPath+".log");
		QFile::remove(baseTmpFileNamesPath+".aux");
		QFile::remove(baseTmpFileNamesPath+".fls");
		QFile::remove(baseTmpFileNamesPath+".fdb_latexmk");
	}
	// -

	// emit error signals:
	m_lastErrorType = currentErrorType;
	// -
	emit complete(outputFileNamePath, static_cast<int>(currentErrorType));
	return result;
	//return makePdf(pdfFileNameArg, m_tempfilePath, outputDirArg, progressInitKey, cleanup, errorType);
}

void LatexWorker::setTempleteTexFile(QString texFileName)
{
	m_templateTexFile = texFileName;
	// чтение содержимого шаблона
	m_templateTex = readTextFromFile(texFileName);
}

bool LatexWorker::setStructure(QByteArray jsonDocumentData, bool update, bool simplified)
{
	bool state = false;

	if(!update) {
		// очистка
		m_keyConvTable.clear();
		m_listDataPages.clear();
		m_listJsonItems.clear();
		m_keyByPageNumberTable.clear();
	}

	// извлечение json-данных
	state = readJsonData(jsonDocumentData, simplified, update);

	return state;
}

bool LatexWorker::setStructure(QList<QImage> pages, bool simplified)
{
	// <тут будет код>
	return false;
}

bool LatexWorker::setStructureValue(QString key, QString value)
{
	int nCounter = getNCounterByKey(key);
	if(nCounter == -1) {
		return false;
	}
	int nPage = getNPageByNCounter(nCounter);
	if(nPage == -1) {
		return false;
	}

	if(nPage >= m_listDataPages.size()) {
		return false;
	}

	auto page = m_listDataPages.value(nPage);
	if(m_listDataPages.value(nPage).contains(key)) {
		page.remove(key);
	}
	page.insert(key, value);
	m_listDataPages.replace(nPage, page);
	//qDebug() << "modified m_listDataPages: " <<  m_listDataPages;

	return true;
}

bool LatexWorker::writeStructure(bool protect, QString variblesBlockMark, QString qrBlockMark)
{
	bool state;
	// вставка во временный файл считанных из json данных
	state = insertJsonDataInTemplete(variblesBlockMark);
	if(state) {
		state = insertQrCodesInTemplete(protect, qrBlockMark);
	}
	return state;
}

void LatexWorker::insertTexCodeByMark(QString mark, QString latexCode)
{
	QString markerFmt = QString("%!%1").arg(mark);
	int indexToStartInsert = m_templateTex.indexOf(markerFmt);
	int indexToEnd = m_templateTex.indexOf(markerFmt, indexToStartInsert+1);
	m_templateTex.remove(indexToStartInsert, (indexToEnd != -1 ? (indexToEnd-indexToStartInsert+markerFmt.length()+1) : m_templateTex.length() - indexToStartInsert));
	m_templateTex.insert(indexToStartInsert, latexCode);
}

void LatexWorker::clearStructure()
{
	m_templateTex.clear();
	setTempleteTexFile(m_templateTexFile);
	m_keyConvTable.clear();
	m_tempfilePath.clear();
	m_listDataPages.clear();
	m_listJsonItems.clear();
	m_keyByPageNumberTable.clear();
}

void LatexWorker::fillStructure(QString sep)
{
	QList<QString> keys = m_keyConvTable.keys();
	for(int i = 0; i < keys.size(); i++) {
		setStructureValue(keys.at(i), sep);
	}
}

bool LatexWorker::createTex(QString texFileName)
{
	// fix input arg(outputFileName):
	QString suffix = texFileName.section('.', -1, -1);
	//qDebug() << "suffix: " << suffix;
	if(suffix.isEmpty() || suffix != "tex") {
		QString pdfSuffix = QString(".pdf");
		if(texFileName.endsWith(pdfSuffix)) {
			texFileName.remove(texFileName.size()-pdfSuffix.size(), pdfSuffix.size());
		}
		texFileName.append(".tex");
	}

	if(texFileName.isEmpty()) {
		qDebug() << "error: file name is empty!";
		return false;
	}
	if(m_templateTex.isEmpty()) {
		qDebug() << "error: templete file not init!";
		return false;
	}
	return writeTextToFile(m_templateTex, texFileName);
}

QByteArray LatexWorker::exportConvTableForSimplifyPages()
{
	QByteArray jsonResult;

	QList<QString> keys = m_keyConvTable.keys();
	int size = keys.size();
	QJsonArray jarray;
	for (int i = 0; i<size; i++) {
		QString key = keys.at(i);
		int ncounter = getNCounterByKey(key);
		int npage = getNPageByNCounter(ncounter);
		QJsonObject jobj = {
			{"k",	key							},
			{"n",	ncounter					},
			{"p",	npage						}
		};
		jarray.append(jobj);
	}

	QJsonDocument exportJDoc(jarray);
	jsonResult = exportJDoc.toJson(QJsonDocument::Compact);

	return jsonResult;
}

bool LatexWorker::importConvTableForSimplifyPages(QByteArray jsonConvTable)
{
	QJsonParseError jerr;
	QJsonDocument importJDoc = QJsonDocument::fromJson(jsonConvTable, &jerr);

	// проверка на корректновсть входных данных
	if(!importJDoc.isArray() || jerr.error != QJsonParseError::NoError) {
		return false;
	}

	// очистить старую таблицу конвертации ключей в числа
	m_keyConvTable.clear();
	m_keyByPageNumberTable.clear();

	// импорт новой таблицы
	QJsonArray jarr = importJDoc.array();
	int size = jarr.size();
	for (int i = 0; i<size; i++) {
		if(jarr.at(i).isObject()) {
			QJsonObject jobj = jarr.at(i).toObject();
			if(jobj.contains("k") && jobj.contains("n") && jobj.contains("p")) {
				QString key = jobj.value("k").toString();
				int ncounter = jobj.value("n").toInt();
				int page = jobj.value("p").toInt();
				m_keyConvTable.insert(key, ncounter);
				m_keyByPageNumberTable.insert(key, page);
			}
		}
	}
	return true;
}

QByteArray LatexWorker::getJsonPage(int pageNumber, bool simplify) const
{
	return (simplify ? getSimplifyJsonPage(pageNumber) : m_listJsonItems.value(pageNumber, QByteArray("null")));
}

QByteArray LatexWorker::readTextFromFile(QString fileName) const
{
	QByteArray result = "";

	QFile file(fileName);
	if(!file.open(QIODevice::ReadOnly | QFile::Text)) {
		qDebug() << "error: text file not found!";
		return result;
	}

	QTextStream in(&file);
	auto cutf8 = QTextCodec::codecForName("UTF-8");
	in.setCodec(cutf8);
	result.append(in.readAll());
	file.close();

	return result;
}

bool LatexWorker::writeTextToFile(QString text, QString fileName)
{
	QFile file(fileName);
	if(!file.open(QIODevice::WriteOnly | QFile::Text)) {
		qDebug() << "error: text file not found!";
		return false;
	}

	QTextStream out(&file);
	auto cutf8 = QTextCodec::codecForName("UTF-8");
	out.setCodec(cutf8);
	out << text;
	file.close();

	return true;
}

QString LatexWorker::getKeyByNCounter(int n) const
{
	return m_keyConvTable.key(n, "");
}

int LatexWorker::getNPageByNCounter(int n) const
{
	return m_keyByPageNumberTable.value(getKeyByNCounter(n), -1);
}

int LatexWorker::getNCounterByKey(QString key) const
{
	return m_keyConvTable.value(key, -1);
}

int LatexWorker::testProgressSize(QString progressInitKey, bool init)
{
	QProcess proc;
	int resultInitProgress = -1;
	QString consoleErr;
	bool fail = false;

	if(!isLatexInstalled()) {
		qDebug() << "error: pdflatex not found!";
		return -1;
	}

	// создание временного файла из считанного шаблона
	if(!makeTempleteTexFile()) {
		qDebug() << "error: no make templete tex-file!";
		return -1;
	}

	// make output signals
	connect(&proc, &QProcess::readyRead, [&]() {
		QString consoleOut = proc.readAll();
		enum ErrorType {
			ERR_OK = 0,
			ERR_SYNTAX,
			ERR_SYNTAX_EXT_1
		} currentErrorType = ERR_OK;

		if(consoleOut.endsWith("*")) {
			qDebug() << "KILL!";
			proc.kill();
			currentErrorType = ERR_SYNTAX_EXT_1;
			fail = true;
		}

		QString errorSegment = consoleOut.section("Error:", 1, -1).simplified();
		if(!errorSegment.isEmpty()) {
			proc.kill();
			currentErrorType = ERR_SYNTAX;
			fail = true;
		}

		switch (currentErrorType) {
		case ERR_OK:
			consoleErr = "";
			if(!fail && !consoleOut.isEmpty()) {
				resultInitProgress += consoleOut.count('\n');
			}
			break;
		case ERR_SYNTAX_EXT_1:
			consoleErr = "Incomplete block segment found!";
			resultInitProgress = -1;
			break;
		case ERR_SYNTAX:
			consoleErr = consoleOut.section("Error:", 1, -1).simplified();
			resultInitProgress = -1;
			break;
		}
	});
	// -

	// test command:
	QString cmd = QString("latexmk -pdf -silent -verbose \"%1\"").arg(m_tempfilePath);
	//qDebug() << "cmd: " << cmd;
	// -

	// make full output path:
	QString baseTmpFileNamesPath = m_tempfilePath.section(QDir::separator(),0,-2)+QDir::separator()+m_tempfilePath.section('.',-2,-2).section(QDir::separator(),-1,-1);
	//qDebug() << "baseTmpFileNamesPath: " << baseTmpFileNamesPath;

	// start working commands:
	proc.setWorkingDirectory(QStandardPaths::writableLocation(QStandardPaths::TempLocation));
	QString prevPath = QDir::currentPath();
	QDir::setCurrent(QStandardPaths::writableLocation(QStandardPaths::TempLocation));
	QFile::remove(baseTmpFileNamesPath+".log");
	QFile::remove(baseTmpFileNamesPath+".aux");
	QFile::remove(baseTmpFileNamesPath+".fls");
	QFile::remove(baseTmpFileNamesPath+".fdb_latexmk");
	//QFile::remove(baseTmpFileNamesPath+".wajqrQ");
	//QFile::remove(baseTmpFileNamesPath+".cIDnXX");
	proc.start(QString("latexmk -c"));
	proc.waitForFinished(10000);
	proc.start(cmd);
	proc.waitForFinished(INF);
	proc.disconnect();
	proc.start(QString("latexmk -c"));
	proc.waitForFinished(10000);
	QFile::remove(m_tempfilePath);
	QFile::remove(baseTmpFileNamesPath+".pdf");
	QFile::remove(baseTmpFileNamesPath+".log");
	QFile::remove(baseTmpFileNamesPath+".aux");
	QFile::remove(baseTmpFileNamesPath+".fls");
	QFile::remove(baseTmpFileNamesPath+".fdb_latexmk");
	QDir::setCurrent(prevPath);
	if(resultInitProgress != -1 && init) {
		initProgressSize(progressInitKey, resultInitProgress);
	}

	return resultInitProgress;
}

void LatexWorker::initProgressSize(QString progressInitKey, int size)
{
	m_progressCorrector.insert(progressInitKey, size);
}

bool LatexWorker::insertJsonDataInTemplete(QString variblesBlockMark)
{
	//qDebug() << "tex: " << m_templateTex;
	QString markerFmt = QString("%!%1").arg(variblesBlockMark);
	QString old_tex_data_fragment = m_templateTex.section(markerFmt/*"%!upd"*/,1,1);
	QString new_tex_data_fragment = old_tex_data_fragment;
	//QString tmp;
	//qDebug() << "old_data_fragment:\n" << old_tex_data_fragment;

	for(int keyCounter=0,sizeElements=m_keyConvTable.size(); keyCounter<sizeElements; keyCounter++) {
		QString key = getKeyByNCounter(keyCounter);
		QString value = m_listDataPages.value(getNPageByNCounter(keyCounter)).value(key);
		QString fmt = QString("(\\\\newcommand\\{\\\\%1\\}\\[\\d{0,2}\\]\\[(?:[0-9\\-]*|\\{\\\\line[A-Z]{1}x[A-Z]{1}\\}|\\{\\\\def[A-Z]{1}x[A-Z]{1}\\})\\]\\{)#1(\\})").arg(key);
		if(value != "#1") new_tex_data_fragment.replace(QRegExp(fmt), QString("\\1%1\\2").arg(value));
	}
	m_templateTex.replace(old_tex_data_fragment, new_tex_data_fragment);
	//qDebug() << "new_tex_data_fragment:\n" << new_tex_data_fragment;

	return true;
}

bool LatexWorker::insertQrCodesInTemplete(bool protect, QString qrBlockMark)
{
	QString insertData = "";
	QString strCmdEnableQrHashSign = QString("\\def\\EnableQrHashSign{%1}\n");
	if(protect) {
		insertData.append(strCmdEnableQrHashSign.arg(1));
		int size = m_listJsonItems.size();
		for (int npage = 0; npage < size; npage++) {
			QByteArray qrData = crypt(npage, true, true).toBase64();

#ifdef TEST_DECRYPT_QR_FROM_FILE_BASE64
			writeTextToFile(qrData, QString("qrc_%1.txt").arg(npage));
			QByteArray rawQrEncodedData = readTextFromFile(QString("qrc_%1.txt").arg(npage));
			QByteArray rawQrDecodedData = decrypt(QByteArray::fromBase64(rawQrEncodedData));
			writeTextToFile(rawQrDecodedData, QString("qrd_%1.txt").arg(npage));
#endif
			//qDebug() << QString("size(page_%1): ").arg(npage) << qrData.size();
			QString strDataInBase64 = QString::fromLocal8Bit(qrData);
			QString data = strDataInBase64;
			QByteArray hashData;
			QCrypter::sha1(data.toLocal8Bit(), hashData);
			QString datetime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm");

			QJsonObject jobjPageSign = {
				{"date",	datetime									},
				{"hash",	QString::fromLocal8Bit(hashData)			},
				{"page",	QString::number(npage+1)					}
			};
			QJsonDocument jtextPageSign(jobjPageSign);
			insertData.append(QString("\\addQrCodeSignToArray{%1}\n").arg(QString::fromLocal8Bit(jtextPageSign.toJson(QJsonDocument::Compact).toBase64())));

			// !!! нужно переделать действующий код согласно алгоритму:
			// #) вычисляем подходящий делитель строк так чтобы каждая часть не была больше чем m_maxNumBytesInQrCode!
			// *) если есть остаток от деления, остаток распределяется так чтобы ячейки максимально распределились по частям, а не в какойто одной части (преимущество у частей в направлении от начала до конца)
			// **) если сумма части с частью остатка от деления вдруг больше чем m_maxNumBytesInQrCode то вернуться на стадию (1) с увеличением делителя на 1, или предусмотреть это заранее в (1) пункте.

			if(strDataInBase64.size() > m_maxNumBytesInQrCode) {
				int numParts = !strDataInBase64.isEmpty() ? strDataInBase64.size()/m_maxNumBytesInQrCode + 1 : 0; // !!!
				for(int part = 1, startIndex = 0, nread = 0; part <= numParts; part++) {
					nread = (strDataInBase64.mid(startIndex,-1).size() < m_maxNumBytesInQrCode ? -1 : startIndex + m_maxNumBytesInQrCode); // !!!
					QString partData = strDataInBase64.mid(startIndex, nread); // !!!
					QJsonObject jobj = {
						{"data",	partData									},
						{"date",	datetime									},
						{"page",	QString::number(npage+1)					},
						{"part",	QString::number(part)						},
						{"parts",	QString::number(numParts)					}
					};
					QJsonDocument jtext(jobj);
					insertData.append(QString("\\addQrCodeDataToArray{%1}\n").arg(QString::fromLocal8Bit(jtext.toJson(QJsonDocument::Compact).toBase64())));
					startIndex = nread != -1 ? startIndex + nread : strDataInBase64.length();
				}
#ifdef TEST_VIEW_QR_PARTS_BASE64
				//
#endif
			} else {
				QJsonObject jobj = {
					{"data",	data										},
					{"date",	datetime									},
					{"page",	QString::number(npage+1)					}
				};
				QJsonDocument jtext(jobj);
				insertData.append(QString("\\addQrCodeDataToArray{%1}\n").arg(QString::fromLocal8Bit(jtext.toJson(QJsonDocument::Compact).toBase64())));
			}
		}
	} else {
		insertData.append(strCmdEnableQrHashSign.arg(0));
	}
	insertTexCodeByMark(qrBlockMark, insertData);
	//qDebug() << "m_listJsonItems.size():" << m_listJsonItems.size();
	//writeTextToFile(m_templateTex, "tmpTex.tex");
	return true;
}

bool LatexWorker::makeTempleteTexFile()
{
	QTemporaryFile* tmpFile = new QTemporaryFile();
	if(tmpFile->open()) {
		QTextStream outputStream(tmpFile);
		outputStream << m_templateTex;
		m_tempfilePath = tmpFile->fileName();
		//qDebug() << "m_tempfilePath: " << m_tempfilePath;
		tmpFile->close();
		tmpFile->deleteLater();
		return true;
	}
	return false;
}

QByteArray LatexWorker::getSimplifyJsonPage(int pageNumber) const
{
	QJsonDocument pageJsonDoc = QJsonDocument::fromJson(m_listJsonItems.value(pageNumber,"null"));
	QJsonObject obj = pageJsonDoc.object();
	QJsonObject objSimplify;
	QStringList keys = obj.keys();
	int size = obj.size();
	for(int i=0; i<size; i++) {
		QString key = keys.value(i);
		QJsonValue value = obj.value(key);
		int ncounter = getNCounterByKey(key);
		objSimplify.insert(QString::number(ncounter), value);
	}
	QJsonDocument simplifyPageJsonDoc = QJsonDocument(objSimplify);
	return simplifyPageJsonDoc.toJson(QJsonDocument::Compact);
}

QByteArray LatexWorker::convertSimplifyJsonToReadFmt(QByteArray importedJson)
{
	QByteArray jsonResult;
	QJsonDocument jdata = QJsonDocument::fromJson(importedJson);

	if(!jdata.isArray()) {
		return jsonResult;
	}
	QJsonArray jdataPages = jdata.array();
	int size = jdataPages.size();
	for (int i = 0; i<size; i++) {
		QJsonObject edited_obj = jdataPages.at(i).toObject();
		QJsonDocument pageJDoc(edited_obj);
		QByteArray jsonPage(pageJDoc.toJson(QJsonDocument::Compact));
		QStringList keys = edited_obj.keys();
		// редактирование объекта-страницы (всех полей страницы)
		for(int i=0; i<keys.size(); i++) {
			QString key_1 = keys.at(i);
			QString key_2 = getKeyByNCounter(key_1.toInt());
			QString value = edited_obj.value(key_1).toString();
			// записываем новое значение в объект с удалением старого поля
			edited_obj.remove(key_1);
			edited_obj.insert(key_2, value);
		}
		// замена старого объекта на измененный
		jdataPages.replace(i, edited_obj);
	}
	QJsonDocument jresultDoc(jdataPages);
	jsonResult = jresultDoc.toJson(QJsonDocument::Compact);
	return jsonResult;
}

QByteArray LatexWorker::zip(QByteArray input, int level)
{
	QByteArray zippedJsonPage;
	QCompressor::gzipCompress(input, zippedJsonPage, level);
	return zippedJsonPage;
}

QByteArray LatexWorker::zip(int pageNumber, bool simplify, int level)
{
	QByteArray input = getJsonPage(pageNumber, simplify);
	return zip(input, level);
}

QByteArray LatexWorker::unzip(QByteArray input)
{
	QByteArray unZippedJsonPage;
	QCompressor::gzipDecompress(input, unZippedJsonPage);
	return unZippedJsonPage;
}

QByteArray LatexWorker::crypt(QByteArray input)
{
	QByteArray cryptedJsonPage;
	QCrypter::aes_enc(input, cryptedJsonPage);
	return cryptedJsonPage;
}

QByteArray LatexWorker::crypt(int pageNumber, bool simplify, bool compress, int compress_level)
{
	QByteArray input = (compress ? zip(getJsonPage(pageNumber, simplify), compress_level) : getJsonPage(pageNumber, simplify));
	return crypt(input);
}

QByteArray LatexWorker::decrypt(QByteArray input, bool decompress)
{
	QByteArray decryptedJsonPage;
	QCrypter::aes_dec(input, decryptedJsonPage);
	return (decompress ? unzip(decryptedJsonPage) : decryptedJsonPage);
}

bool LatexWorker::readJsonData(QByteArray jsonData, bool simplified, bool update)
{		
	QJsonDocument jdata = QJsonDocument::fromJson((simplified ? convertSimplifyJsonToReadFmt(jsonData) : jsonData));

	if(!jdata.isArray()) {
		return false;
	}
	QJsonArray jdataPages = jdata.array();

	int pageCounter = 0;
	int keyCounter = 0;
	if(!update) {
		foreach (QJsonValue dataPage, jdataPages) {
			QJsonObject obj = dataPage.toObject();
			QJsonDocument doc(obj);
			QByteArray jsonPage(doc.toJson(QJsonDocument::Compact));
			m_listJsonItems.insert(static_cast<int>(pageCounter), jsonPage);
			QStringList keys = obj.keys();
			QHash<QString,QString> data;
			for(int i=0; i<keys.size(); i++) {
				QString key = keys.at(i);
				QString value = obj.value(key).toString();
				data[key] = value;
				m_keyConvTable.insert(key, keyCounter);
				++keyCounter;
				m_keyByPageNumberTable.insert(key,pageCounter);
			}
			m_listDataPages.insert(pageCounter, data);
			++pageCounter;
		}
	} else {
		foreach (QJsonValue dataPage, jdataPages) {
			QJsonObject obj = dataPage.toObject();
			QJsonDocument doc(obj);
			QByteArray jsonPage(doc.toJson(QJsonDocument::Compact));
			QStringList keys = obj.keys();
			for(int i=0; i<keys.size(); i++) {
				QString key = keys.at(i);
				QString value = obj.value(key).toString();
				setStructureValue(key, value);
			}
			++pageCounter;
		}
	}
	//qDebug() << "m_listJsonItems" << m_listJsonItems;
	if(pageCounter > 0)
		return true;

	return false;
}

bool LatexWorker::readSimplifyJsonData(QByteArray jsonData, bool update)
{
	QJsonDocument jdata = QJsonDocument::fromJson(jsonData);

	if(!jdata.isArray()) {
		return false;
	}
	QJsonArray jdataPages = jdata.array();
	int pageCounter = 0;
	if(!update) {
		foreach (QJsonValue dataPage, jdataPages) {
			QJsonObject obj = dataPage.toObject();
			QJsonDocument doc(obj);
			QByteArray jsonPage(doc.toJson(QJsonDocument::Compact));
			m_listJsonItems.insert(static_cast<int>(pageCounter), jsonPage);
			QStringList keys = obj.keys();
			QHash<QString,QString> data;
		}
	} else {
		//
	}

	if(pageCounter > 0)
		return true;

	return false;
}
