#include "chunked.h"
#include <QDataStream>
#include <QtDebug>
#include <cassert>

#include "chunked.moc"


ChunkedDevice::ChunkedDevice(QIODevice* device):
	QIODevice(device) {
	readBuffer.open(ReadOnly);
	writeBuffer.open(WriteOnly);

	connect(device, SIGNAL(readyRead()), SLOT(parentReadyRead()));
	connect(device, SIGNAL(disconnected()), SIGNAL(disconnected()));

	if (device->isOpen())
		open(device->openMode());
}

QIODevice* ChunkedDevice::parentDevice() {
	return static_cast<QIODevice*>(QObject::parent());
}

bool ChunkedDevice::open(OpenMode mode) {
	QIODevice::open(mode | Unbuffered);
	return parentDevice()->open(mode);
}

void ChunkedDevice::parentReadyRead(bool emitReadyRead) {
	QIODevice* device = parentDevice();
	if (readBuffer.isReadable() && readBuffer.atEnd()
	 && device->bytesAvailable() >= sizeof(qint64)) {
		QDataStream stream(device);
		qint64 size;
		stream >> size;
		assert(size > 0);
		readBuffer.buffer().resize(size);
		readBuffer.close();
		readBuffer.open(WriteOnly);
		if (device->bytesAvailable() > 0)
			parentReadyRead(emitReadyRead);
	}
	if (readBuffer.isWritable()) {
		qint64 bytesToRead = readBuffer.size() - readBuffer.pos();
		if (bytesToRead > 0) {
			readBuffer.write(device->read(bytesToRead));
			if (readBuffer.atEnd()) {
				readBuffer.close();
				readBuffer.open(ReadOnly);
				//qDebug() << "* received message of size " << readBuffer.size() << emitReadyRead;
				if (emitReadyRead)
					emit readyRead();
			}
		}
	}
}

qint64 ChunkedDevice::readData(char* data, qint64 maxSize) {
	//qDebug() << "ChunkedDevice::readData" << this;
	if (readBuffer.isReadable()) {
		//qDebug() << "* read stat" << readBuffer.pos() << readBuffer.size();
		qint64 read = readBuffer.read(data, maxSize);
		//qDebug() << "* read" << read << maxSize;
		if (parentDevice()->bytesAvailable() > 0)
			parentReadyRead(false);
		return read;
	} else {
		return -1;
	}
}

qint64 ChunkedDevice::writeData(const char* data, qint64 maxSize) {
	if (writeBuffer.isWritable()) {
		qint64 written = writeBuffer.write(data, maxSize);
		return written;
	} else {
		return -1;
	}
}

bool ChunkedDevice::flush() {
	if (writeBuffer.isWritable()) {
		QIODevice* device = parentDevice();
		QDataStream stream(device);
		qint64 bytes = writeBuffer.pos();
		stream << bytes;
		device->write(writeBuffer.buffer().data(), bytes);
		//qDebug() << "* sending message of size " << writeBuffer.size();
		emit bytesWritten(sizeof(qint64) + bytes);
		writeBuffer.reset();
		return true;
	} else {
		return false;
	}
}

bool ChunkedDevice::isMessage() const {
	//qDebug() << "ChunkedDevice::isMessage" << this;
	//qDebug() << "* ismsg" << readBuffer.isReadable() << readBuffer.pos() << readBuffer.size();
	return (readBuffer.isReadable() && !readBuffer.atEnd());
}
