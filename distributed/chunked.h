#ifndef CHUNKED_H_
#define CHUNKED_H_


#include <QBuffer>


struct ChunkedDevice: QIODevice {

	Q_OBJECT

public:
	ChunkedDevice(QIODevice* device);
	QIODevice* parentDevice();
	bool open(OpenMode mode);
	qint64 readData(char* data, qint64 maxSize);
	qint64 writeData(const char* data, qint64 maxSize);
	bool flush();
	bool isMessage() const;
	
signals:
	void disconnected();

protected slots:
	void parentReadyRead(bool emitReadyRead = true);

private:
	QBuffer readBuffer;
	QBuffer writeBuffer;
};


#endif // CHUNKED_H_
