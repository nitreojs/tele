#include "tele/tele_sealed_files.h"
#include "tele/tele_sealed_media_cache.h"
#include <QCoreApplication>
#include <QBuffer>
#include <QColorSpace>
#include <QImage>
#include <QImageReader>
#include <QTextStream>
#include <QtEndian>
#include <cstdlib>

int main(int argc, char **argv) {
	QCoreApplication app(argc, argv);
	auto checks = 0;
	const auto require = [&](bool pass, const char *name) {
		++checks;
		if (!pass) { QTextStream(stderr) << "FAILED: " << name << '\n'; std::exit(1); }
	};
	const auto round = Tele::PrepareSealedFilePayload("C:\\private\\name.txt", QByteArray("abc\0def", 7));
	require(round.has_value(), "generic prepare");
	const auto parsed = Tele::OpenSealedFilePayload(*round);
	require(parsed && parsed->name == "name.txt" && parsed->bytes == QByteArray("abc\0def", 7), "generic binary roundtrip");
	require((*round)[8] == 1 && parsed->presentation == Tele::SealedFilePresentation::Document, "legacy generic remains document v1");
	const auto empty = Tele::PrepareSealedFilePayload("empty.bin", {});
	require(empty && Tele::OpenSealedFilePayload(*empty)->bytes.isEmpty(), "empty file roundtrip");
	for (const auto &name : {"..", "CON", "aux.txt", "LPT1.bin", "a:b?.txt", "../x/../../last.bin", "a\n\t.txt", "a.", "a ", "NUL.txt"}) {
		const auto prepared = Tele::PrepareSealedFilePayload(QString::fromUtf8(name), "data");
		require(prepared && Tele::OpenSealedFilePayload(*prepared).has_value(), "unsafe outbound name sanitizes consistently");
	}
	for (auto i = 0; i < 15; ++i) {
		require(!Tele::OpenSealedFilePayload(round->left(i)), "truncated header");
	}
	auto modified = *round;
	modified[0] ^= 1;
	require(!Tele::OpenSealedFilePayload(modified), "wrong magic");
	modified = *round;
	modified[8] = 3;
	require(!Tele::OpenSealedFilePayload(modified), "wrong version");
	modified = *round;
	modified.append('x');
	require(!Tele::OpenSealedFilePayload(modified), "trailing bytes");
	modified = *round;
	qToBigEndian(quint32(0xFFFFFFFF), modified.data() + 11);
	require(!Tele::OpenSealedFilePayload(modified), "oversized declared content");
	modified = *round;
	qToBigEndian(quint16(0xFFFF), modified.data() + 9);
	require(!Tele::OpenSealedFilePayload(modified), "oversized name");
	require(!Tele::PrepareSealedFilePayload("big.bin", QByteArray(Tele::kSealedFilePayloadLimit + 1, 'a')), "oversized input");
	const auto maximum = Tele::PrepareSealedFilePayload("x", QByteArray(Tele::kSealedFilePayloadLimit - 16, 'a'));
	require(maximum && maximum->size() == Tele::kSealedFilePayloadLimit, "exact maximum payload");
	require(!Tele::PrepareSealedFilePayload("x", QByteArray(Tele::kSealedFilePayloadLimit - 15, 'a')), "one over maximum payload");
	QImage source(QSize(2, 2), QImage::Format_ARGB32);
	source.setPixel(0, 0, qRgba(11, 22, 33, 0));
	source.setPixel(1, 0, qRgba(44, 55, 66, 63));
	source.setPixel(0, 1, qRgba(77, 88, 99, 128));
	source.setPixel(1, 1, qRgba(111, 122, 133, 255));
	source.setText("Comment", "PRIVATE GPS 12.3456 78.9012");
	source.setText("Author", "PRIVATE PHOTOGRAPHER");
	source.setColorSpace(QColorSpace::SRgb);
	source.setDotsPerMeterX(12345);
	source.setDotsPerMeterY(12345);
	QByteArray png;
	QBuffer pngBuffer(&png);
	pngBuffer.open(QIODevice::WriteOnly);
	require(source.save(&pngBuffer, "PNG"), "PNG input fixture");
	require(png.contains("PRIVATE GPS"), "PNG fixture has text metadata");
	require(png.contains("iCCP"), "PNG fixture has ICC metadata");
	const auto sanitized = Tele::PrepareSealedFilePayload("private-camera-location.png", png, true);
	require(sanitized.has_value(), "sanitize PNG");
	const auto photo = Tele::OpenSealedFilePayload(*sanitized);
	require(photo && photo->name == "photo.png", "photo uses generic name");
	const auto decoded = QImage::fromData(photo->bytes, "PNG");
	require(decoded.size() == source.size(), "PNG dimensions retained");
	for (auto y = 0; y < source.height(); ++y) for (auto x = 0; x < source.width(); ++x) {
		require(source.pixel(x, y) == decoded.pixel(x, y), "RGBA pixels retained exactly");
	}
	require(decoded.textKeys().isEmpty(), "PNG text removed");
	require(!decoded.colorSpace().isValid(), "PNG ICC removed");
	QStringList chunks;
	for (qsizetype offset = 8; offset + 12 <= photo->bytes.size();) {
		const auto size = qFromBigEndian<quint32>(reinterpret_cast<const uchar*>(photo->bytes.constData() + offset));
		chunks += QString::fromLatin1(photo->bytes.mid(offset + 4, 4));
		offset += 12 + size;
	}
	QTextStream(stdout) << "PNG chunks: " << chunks.join(",") << "\n";
	require(chunks == QStringList{"IHDR", "IDAT", "IEND"}, "PNG contains only pixel chunks");
	require((*sanitized)[8] == 1 && photo->presentation == Tele::SealedFilePresentation::Document, "legacy sanitized photo remains document v1");
	const auto asImage = [](const QByteArray &bytes) {
		auto result = Tele::PrepareSealedFilePayload("photo.png", bytes);
		if (!result) return QByteArray();
		(*result)[8] = 2;
		result->insert(9, char(1));
		return *result;
	};
	const auto repairCrc = [](QByteArray &bytes, qsizetype offset) {
		const auto length = qFromBigEndian<quint32>(reinterpret_cast<const uchar*>(bytes.constData() + offset));
		auto crc = quint32(0xFFFFFFFFU);
		for (auto i = qsizetype(4); i < qsizetype(length) + 8; ++i) {
			crc ^= uchar(bytes[offset + i]);
			for (auto bit = 0; bit != 8; ++bit) {
				crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320U : 0U);
			}
		}
		qToBigEndian(crc ^ 0xFFFFFFFFU, bytes.data() + offset + qsizetype(length) + 8);
	};
	const auto imagePayload = Tele::PrepareSealedFilePayload("private-camera-location.png", png, true, Tele::SealedFilePresentation::Image);
	require(imagePayload.has_value(), "explicit image presentation prepares");
	const auto image = Tele::OpenSealedFilePayload(*imagePayload);
	require(image && image->presentation == Tele::SealedFilePresentation::Image && image->name == "photo.png", "authenticated inner image presentation");
	require(image->bytes == photo->bytes, "image sanitization preserves legacy pixel bytes");
	require((*imagePayload)[8] == 2 && (*imagePayload)[9] == 1 && imagePayload->size() == sanitized->size() + 1, "image presentation v2 framing");
	require(*imagePayload == asImage(photo->bytes), "independent image framing fixture");
	require(!Tele::PrepareSealedFilePayload("photo.png", png, false, Tele::SealedFilePresentation::Image), "image presentation requires sanitization");
	require(!Tele::PrepareSealedFilePayload("photo.png", photo->bytes, false, Tele::SealedFilePresentation::Image), "even clean image requires explicit sanitization");
	require(!Tele::PrepareSealedFilePayload("photo.png", png, true, static_cast<Tele::SealedFilePresentation>(2)), "unknown outbound presentation rejected");
	const auto documentImage = Tele::PrepareSealedFilePayload("photo.png", png, false, Tele::SealedFilePresentation::Document);
	require(documentImage && Tele::OpenSealedFilePayload(*documentImage)->presentation == Tele::SealedFilePresentation::Document, "image sent as document stays document");
	for (auto i = 0; i < 16; ++i) {
		require(!Tele::OpenSealedFilePayload(imagePayload->left(i)), "truncated image presentation header");
	}
	modified = *imagePayload;
	modified[9] = 0;
	const auto v2Document = Tele::OpenSealedFilePayload(modified);
	require(v2Document && v2Document->presentation == Tele::SealedFilePresentation::Document, "v2 explicit document accepted");
	for (const auto hint : {2, 127, 255}) {
		modified = *imagePayload;
		modified[9] = char(hint);
		require(!Tele::OpenSealedFilePayload(modified), "unknown inbound presentation rejected");
	}
	modified = *imagePayload;
	modified[16] = 'x';
	require(!Tele::OpenSealedFilePayload(modified), "image presentation requires generic sanitized name");
	modified = *imagePayload;
	modified.append('x');
	require(!Tele::OpenSealedFilePayload(modified), "image presentation trailing payload rejected");
	modified = *imagePayload;
	qToBigEndian(quint32(0xFFFFFFFF), modified.data() + 12);
	require(!Tele::OpenSealedFilePayload(modified), "image presentation oversized content rejected");
	modified = *imagePayload;
	qToBigEndian(quint16(0xFFFF), modified.data() + 10);
	require(!Tele::OpenSealedFilePayload(modified), "image presentation oversized name rejected");
	require(!Tele::OpenSealedFilePayload(asImage("not an image")), "image hint cannot label binary data");
	require(!Tele::OpenSealedFilePayload(asImage({})), "image hint cannot label empty data");
	require(!Tele::OpenSealedFilePayload(asImage(png)), "image hint cannot retain text ICC or resolution chunks");
	modified = photo->bytes;
	modified[0] ^= 1;
	require(!Tele::OpenSealedFilePayload(asImage(modified)), "image PNG signature validated");
	modified = photo->bytes;
	modified[29] ^= 1;
	require(!Tele::OpenSealedFilePayload(asImage(modified)), "image PNG header CRC validated");
	modified = photo->bytes;
	modified[41] ^= 1;
	require(!Tele::OpenSealedFilePayload(asImage(modified)), "image PNG pixels CRC validated");
	modified = photo->bytes;
	modified[modified.size() - 1] ^= 1;
	require(!Tele::OpenSealedFilePayload(asImage(modified)), "image PNG end CRC validated");
	modified = photo->bytes;
	modified.append('x');
	require(!Tele::OpenSealedFilePayload(asImage(modified)), "image PNG trailing bytes rejected");
	modified = photo->bytes;
	modified.chop(12);
	require(!Tele::OpenSealedFilePayload(asImage(modified)), "image PNG missing end rejected");
	modified = photo->bytes;
	qToBigEndian(quint32(0xFFFFFFFF), modified.data() + 8);
	require(!Tele::OpenSealedFilePayload(asImage(modified)), "image PNG chunk bounds checked before CRC");
	for (const auto size : {QSize(0, 2), QSize(16385, 1), QSize(4097, 4096)}) {
		modified = photo->bytes;
		qToBigEndian(quint32(size.width()), modified.data() + 16);
		qToBigEndian(quint32(size.height()), modified.data() + 20);
		repairCrc(modified, 8);
		require(!Tele::OpenSealedFilePayload(asImage(modified)), "image PNG decoded dimension bounds enforced");
	}
	for (const auto offset : {24, 25, 26, 27, 28}) {
		modified = photo->bytes;
		modified[offset] = char(1);
		repairCrc(modified, 8);
		require(!Tele::OpenSealedFilePayload(asImage(modified)), "image PNG canonical RGBA8 header enforced");
	}
	modified = photo->bytes;
	const auto idatSize = qFromBigEndian<quint32>(reinterpret_cast<const uchar*>(modified.constData() + 33));
	for (auto i = quint32(0); i < idatSize; ++i) modified[41 + i] = char(0);
	repairCrc(modified, 33);
	require(!Tele::OpenSealedFilePayload(asImage(modified)), "image PNG full decode required after structural validation");
	const auto jpegBytes = QByteArray::fromBase64(
		"/9j/4AAQSkZJRgABAQAAAQABAAD/4QDcRXhpZgAATU0AKgAAAAgABAEOAAIAAAAaAAAAPgESAAMA"
		"AAABAAYAAAE7AAIAAAAVAAAAWIglAAQAAAABAAAAbgAAAABQUklWQVRFIEdQUyBtZXRhZGF0YSB0"
		"ZXN0AFByaXZhdGUgcGhvdG9ncmFwaGVyAAAABAABAAIAAAACTgAAAAACAAUAAAADAAAApAADAAIA"
		"AAACRQAAAAAEAAUAAAADAAAAvAAAAAAAAAAMAAAAAQAAACIAAAABAAAAOAAAAAEAAABOAAAAAQAA"
		"AAwAAAABAAAAIgAAAAH/2wBDAAEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEB"
		"AQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQH/2wBDAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEB"
		"AQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQH/wAARCAACAAMDAREAAhEBAxEB"
		"/8QAHwAAAQUBAQEBAQEAAAAAAAAAAAECAwQFBgcICQoL/8QAtRAAAgEDAwIEAwUFBAQAAAF9AQID"
		"AAQRBRIhMUEGE1FhByJxFDKBkaEII0KxwRVS0fAkM2JyggkKFhcYGRolJicoKSo0NTY3ODk6Q0RF"
		"RkdISUpTVFVWV1hZWmNkZWZnaGlqc3R1dnd4eXqDhIWGh4iJipKTlJWWl5iZmqKjpKWmp6ipqrKz"
		"tLW2t7i5usLDxMXGx8jJytLT1NXW19jZ2uHi4+Tl5ufo6erx8vP09fb3+Pn6/8QAHwEAAwEBAQEB"
		"AQEBAQAAAAAAAAECAwQFBgcICQoL/8QAtREAAgECBAQDBAcFBAQAAQJ3AAECAxEEBSExBhJBUQdh"
		"cRMiMoEIFEKRobHBCSMzUvAVYnLRChYkNOEl8RcYGRomJygpKjU2Nzg5OkNERUZHSElKU1RVVldY"
		"WVpjZGVmZ2hpanN0dXZ3eHl6goOEhYaHiImKkpOUlZaXmJmaoqOkpaanqKmqsrO0tba3uLm6wsPE"
		"xcbHyMnK0tPU1dbX2Nna4uPk5ebn6Onq8vP09fb3+Pn6/9oADAMBAAIRAxEAPwD7/n/ZR/Zb8UeI"
		"/iLqXib9mv4A+ItRtvjD8avDttqGu/Bz4d6ve2/h/wAIfFrxp4T8J6FBdah4cuJ4tH8L+FdE0bwz"
		"4d0yORbLRPD+kaZo2mwW2nWFrbRf6F/Qd8SfEXgz6Ev0M8o4P4+414UynGfRL+jXxZjMr4a4pz3I"
		"suxXFPHvgtwRxzxzxLicFlePwuGr8QcacbcRcQcY8WZzVpTzHiLinPc54gzjE4zNs0xuLr/84X7b"
		"Txd8V/D39pZ485DwD4n+IfA+RY/hH6M3HGOyXhDjXiThrKcbxp4m/RR8D/EnxJ4vxeXZLmWCweJ4"
		"o8QfETizinj7jjP61GebcWcacS8QcU59i8fnmc5jj8T/AP/Z");
	require(jpegBytes.contains("Exif") && jpegBytes.contains("PRIVATE GPS"), "JPEG fixture has EXIF metadata");
	const auto jpegPayload = Tele::PrepareSealedFilePayload("location.jpg", jpegBytes, true);
	require(jpegPayload.has_value(), "sanitize JPEG");
	require(!Tele::OpenSealedFilePayload(asImage(jpegBytes)), "image hint rejects unsanitized JPEG bytes");
	const auto jpegImagePayload = Tele::PrepareSealedFilePayload("location.jpg", jpegBytes, true, Tele::SealedFilePresentation::Image);
	const auto jpegImage = jpegImagePayload ? Tele::OpenSealedFilePayload(*jpegImagePayload) : std::nullopt;
	require(jpegImage && jpegImage->presentation == Tele::SealedFilePresentation::Image && QImage::fromData(jpegImage->bytes, "PNG").size() == QSize(2, 3), "explicit JPEG image sanitizes orientation into displayable PNG");
	const auto jpegPhoto = Tele::OpenSealedFilePayload(*jpegPayload);
	const auto jpegDecoded = QImage::fromData(jpegPhoto->bytes, "PNG");
	require(jpegDecoded.size() == QSize(2, 3), "EXIF orientation applied");
	require(!jpegPhoto->bytes.contains("Exif") && !jpegPhoto->bytes.contains("PRIVATE"), "JPEG EXIF GPS and text removed");
	QBuffer jpegInput;
	jpegInput.setData(jpegBytes);
	jpegInput.open(QIODevice::ReadOnly);
	QImageReader jpegReader(&jpegInput);
	jpegReader.setAutoTransform(true);
	const auto expected = jpegReader.read().convertToFormat(QImage::Format_ARGB32);
	for (auto y = 0; y < expected.height(); ++y) for (auto x = 0; x < expected.width(); ++x) {
		require(expected.pixel(x, y) == jpegDecoded.pixel(x, y), "JPEG oriented pixels retained");
	}
	require(!Tele::PrepareSealedFilePayload("bad.jpg", "not a photo", true), "non image photo rejected");
	for (const auto size : {QSize(1, 1), QSize(2000, 1500), QSize(4096, 4096), QSize(16384, 1024)}) {
		require(Tele::SealedMediaImageSizeAllowed(size), "native media pure bounds accept valid sizes");
	}
	for (const auto size : {QSize(), QSize(0, 1), QSize(1, 0), QSize(-1, 1), QSize(1, -1), QSize(16385, 1), QSize(1, 16385), QSize(4097, 4096), QSize(2147483647, 2147483647)}) {
		require(!Tele::SealedMediaImageSizeAllowed(size), "native media pure bounds reject invalid or overflowing sizes");
	}
	{
		auto camera = QImage(QSize(4000, 3000), QImage::Format_ARGB32);
		auto noise = quint32(0x5A17B23D);
		for (auto y = 0; y < camera.height(); ++y) {
			const auto row = reinterpret_cast<QRgb*>(camera.scanLine(y));
			for (auto x = 0; x < camera.width(); ++x) {
				noise ^= noise << 13;
				noise ^= noise >> 17;
				noise ^= noise << 5;
				row[x] = qRgb(int(noise & 0xFF), int((noise >> 8) & 0xFF), int((noise >> 16) & 0xFF));
			}
		}
		auto cameraBytes = QByteArray();
		auto cameraBuffer = QBuffer(&cameraBytes);
		cameraBuffer.open(QIODevice::WriteOnly);
		require(camera.save(&cameraBuffer, "JPEG", 85), "12MP camera JPEG fixture");
		require(cameraBytes.size() < Tele::kSealedFilePayloadLimit && camera.sizeInBytes() > 32 * 1024 * 1024, "12MP compressed source fits despite large decoded pixels");
		const auto resizedPayload = Tele::PrepareSealedFilePayload("camera.jpg", cameraBytes, true, Tele::SealedFilePresentation::Image);
		require(resizedPayload && resizedPayload->size() <= Tele::kSealedFilePayloadLimit, "12MP normal photo fits encrypted payload after resize");
		const auto resized = Tele::OpenSealedFilePayload(*resizedPayload);
		require(resized && resized->presentation == Tele::SealedFilePresentation::Image, "12MP resized photo authentic image presentation");
		const auto resizedPixels = QImage::fromData(resized->bytes, "PNG");
		require(resizedPixels.size() == QSize(2000, 1500), "12MP landscape photo becomes 2000x1500");
		require(resizedPixels.textKeys().isEmpty() && !resizedPixels.colorSpace().isValid(), "12MP resized photo has no metadata");
		require(!Tele::PrepareSealedFilePayload("camera.jpg", cameraBytes, true), "legacy document sanitization fails oversized pixels without resizing fallback");
		camera.fill(qRgba(12, 34, 56, 255));
		camera.setText("Comment", "PRIVATE CAMERA LOCATION");
		auto uniformBytes = QByteArray();
		auto uniformBuffer = QBuffer(&uniformBytes);
		uniformBuffer.open(QIODevice::WriteOnly);
		require(camera.save(&uniformBuffer, "PNG"), "12MP compressible document fixture");
		const auto fullPayload = Tele::PrepareSealedFilePayload("camera.png", uniformBytes, true);
		require(fullPayload && (*fullPayload)[8] == 1, "legacy 12MP sanitized document keeps v1 encoding");
		const auto full = Tele::OpenSealedFilePayload(*fullPayload);
		require(full && full->presentation == Tele::SealedFilePresentation::Document && QImage::fromData(full->bytes, "PNG").size() == QSize(4000, 3000), "legacy 12MP sanitized document preserves full size when it fits");
	}
	QImage large(QSize(4097, 4096), QImage::Format_ARGB32);
	large.fill(Qt::red);
	QByteArray largeBytes;
	QBuffer largeBuffer(&largeBytes);
	largeBuffer.open(QIODevice::WriteOnly);
	require(large.save(&largeBuffer, "PNG"), "large compressed fixture");
	require(!Tele::PrepareSealedFilePayload("large.png", largeBytes, true), "decompressed pixel bound");
	require(!Tele::PrepareSealedFilePayload("large.png", largeBytes, true, Tele::SealedFilePresentation::Image), "normal photo resize does not bypass input decoded pixel bound");
	QTextStream(stdout) << checks << " file/photo checks passed.\n";
	return 0;
}
