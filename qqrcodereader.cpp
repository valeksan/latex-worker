#include "qqrcodereader.h"

QQrCodeReader::QQrCodeReader(QObject *parent) : QObject(parent)
{

}

bool QQrCodeReader::scan(const QString fileName, QList<QString> &outlist, double kx, bool adaptiveThreshold)
{
	bool g_qrCodeFound = false;
	zbar::ImageScanner g_pZBarScanner;
	g_pZBarScanner.set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 1);

	outlist.clear();

	cv::Mat image = cv::imread(fileName.toStdString());
	if (image.empty()) {
		qDebug() << "Error read image!" << "\n";
		return false;
	}
	cv::Mat imgbig;

	cv::resize(image, imgbig, cv::Size(static_cast<int>(image.cols*kx), static_cast<int>(image.rows*kx)), cv::INTER_NEAREST);

	cv::Mat grey;
	cv::Mat grey_thr;

	cv::cvtColor(imgbig, grey, CV_BGR2GRAY);

	if(adaptiveThreshold) {
		cv::adaptiveThreshold(grey, grey_thr, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 15, -5);
	}

	uint width = static_cast<uint>(grey.cols);
	uint height = static_cast<uint>(grey.rows);
	uchar *raw = reinterpret_cast<uchar *>(adaptiveThreshold ? grey_thr.data : grey.data);

	//Scan the grayscale converted image for QR codes
	zbar::Image g_pZBarImage(width, height, "Y800", raw, width * height);

#ifdef DEF_DEBUG_VIEW_MODIFIED_IMAGE
	cv::namedWindow("MyQR", CV_WINDOW_AUTOSIZE);
	cv::imshow("MyQR", adaptiveThreshold ? grey_thr : grey);
#endif

	if(zbar_scan_image(g_pZBarScanner, g_pZBarImage)) {
		///Iterate through all symbols within the ZBar Image object
		for(zbar::Image::SymbolIterator symbol = g_pZBarImage.symbol_begin(); symbol != g_pZBarImage.symbol_end(); ++symbol) {
			if (zbar_symbol_get_type(*symbol) == zbar::ZBAR_QRCODE) { //Extract and print the QR code text
				g_qrCodeFound = true;
				const char* pQrData = zbar_symbol_get_data(*symbol);
				const unsigned int qrDataLength = zbar_symbol_get_data_length(*symbol);
				std::vector<char> v(qrDataLength, 0);
				char *pVectorData = &*v.begin();
				strcpy(pVectorData, pQrData);
				QString qrCodeListItem(pVectorData);
				//qDebug() << "qr: " << qrCodeListItem;
				outlist.append(qrCodeListItem);
			}
		}
	}

	return !outlist.isEmpty();
}
