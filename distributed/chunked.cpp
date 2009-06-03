#include "chunked.h"
#include <QDataStream>
#include <QtDebug>

#include <chunked.moc>

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
	QIODevice::open(mode);
	return parentDevice()->open(mode);
}

void ChunkedDevice::parentReadyRead() {
	QIODevice* device = parentDevice();
	if (readBuffer.isWritable()) {
		qint64 bytesToRead = readBuffer.size() - readBuffer.pos();
		if (bytesToRead > 0) {
			readBuffer.write(device->read(bytesToRead));
			if (readBuffer.atEnd()) {
				readBuffer.close();
				readBuffer.open(ReadOnly);
				emit readyRead();
			}
		}
	} else if (readBuffer.atEnd() && device->bytesAvailable() >= sizeof(qint64)) {
		QDataStream stream(device);
		qint64 size;
		stream >> size;
		if (size > 0) {
			readBuffer.buffer().resize(size);
			readBuffer.close();
			readBuffer.open(WriteOnly);
		}
		if (device->bytesAvailable() > 0) {
			parentReadyRead();
		}
	}
}

qint64 ChunkedDevice::readData(char* data, qint64 maxSize) {
	if (readBuffer.isReadable()) {
		qint64 read = readBuffer.read(data, maxSize);
		parentReadyRead();
		return read;
	} else {
		return -1;
	}
}

qint64 ChunkedDevice::writeData(const char* data, qint64 maxSize) {
	if (writeBuffer.isWritable()) {
		return writeBuffer.write(data, maxSize);
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
		emit bytesWritten(sizeof(qint64) + bytes);
		writeBuffer.reset();
		return true;
	} else {
		return false;
	}
}

