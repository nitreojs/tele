// Native test-agent overlay for test/test_scenario.cpp; no account or audio device needed.
#if defined _DEBUG || defined TDESKTOP_RELEASE_TEST_HARNESS
#include "media/audio/media_audio_ffmpeg_loader.h"
#include "test/test_log.h"
#include "test/test_runner.h"

#include <QtCore/QDataStream>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <cmath>

namespace Test {
namespace {

QByteArray Wave(int rate, int channels) {
	constexpr auto seconds = 18;
	const auto samples = rate * seconds;
	const auto size = samples * channels * int(sizeof(qint16));
	auto bytes = QByteArray();
	bytes.reserve(44 + size);
	auto stream = QDataStream(&bytes, QIODevice::WriteOnly);
	stream.setByteOrder(QDataStream::LittleEndian);
	stream.writeRawData("RIFF", 4);
	stream << quint32(36 + size);
	stream.writeRawData("WAVEfmt ", 8);
	stream << quint32(16) << quint16(1) << quint16(channels)
		<< quint32(rate) << quint32(rate * channels * 2)
		<< quint16(channels * 2) << quint16(16);
	stream.writeRawData("data", 4);
	stream << quint32(size);
	for (auto i = 0; i != samples; ++i) {
		const auto time = double(i) / rate;
		stream << qint16(1500 + 300 * time);
		if (channels == 2) {
			stream << qint16(8000 * std::sin(2 * M_PI * 440 * time));
		}
	}
	return bytes;
}

using Loader = Media::FFMpegLoader;
using Error = Media::AudioPlayerLoader::ReadError;

bool Read(Loader &loader, QByteArray &output, int target, bool replay) {
	for (auto reads = 0; reads != 100000; ++reads) {
		if (target > 0 && output.size() >= target) {
			return true;
		}
		const auto result = loader.readMore();
		if (const auto pcm = std::get_if<bytes::const_span>(&result)) {
			output.append(reinterpret_cast<const char*>(pcm->data()), pcm->size());
		} else {
			const auto error = std::get<Error>(result);
			if (error == Error::EndOfFile) {
				return target <= 0;
			} else if (error == Error::RetryNotQueued && replay) {
				return true;
			} else if (error != Error::Retry && error != Error::RetryNotQueued) {
				return false;
			}
		}
	}
	return false;
}

QJsonObject Case(const QByteArray &wave, int rate, int channels, double from, double to) {
	auto reference = Loader(Core::FileLocation(), wave, bytes::vector());
	auto buffered = Loader(Core::FileLocation(), wave, bytes::vector());
	auto result = QJsonObject{
		{"rate", rate}, {"channels", channels}, {"from", from}, {"to", to},
	};
	auto ok = reference.open(0, from) && buffered.open(0, from);
	auto initialReference = QByteArray();
	auto initialBuffered = QByteArray();
	if (ok) {
		ok = Read(reference, initialReference, 4 * reference.bytesPerBuffer(), false)
			&& Read(buffered, initialBuffered, 4 * buffered.bytesPerBuffer(), false)
			&& initialReference == initialBuffered;
	}
	if (!ok) {
		result["error"] = "could not prime identical real decoders";
		result["passed"] = false;
		return result;
	}
	// The mixer saves a decoded chunk when its three OpenAL buffers are occupied.
	// Both loaders retain the same original frames; only one has saved output PCM.
	auto pending = initialBuffered.right(buffered.bytesPerBuffer());
	result["savedPcmBytes"] = pending.size();
	buffered.saveDecodedSamples(&pending);
	const auto position = int64(reference.samplesFrequency() * from / 2);
	reference.dropFramesTill(position);
	buffered.dropFramesTill(position);
	const auto referenceStart = reference.startReadingQueuedFrames(to);
	const auto bufferedStart = buffered.startReadingQueuedFrames(to);
	auto expected = QByteArray();
	auto actual = QByteArray();
	ok = referenceStart >= 0 && referenceStart == bufferedStart
		&& Read(reference, expected, 0, true)
		&& Read(buffered, actual, 0, true);
	if (buffered.holdsSavedDecodedSamples()) {
		auto saved = QByteArray();
		buffered.takeSavedDecodedSamples(&saved);
		actual.append(saved);
	}
	ok = ok && Read(reference, expected, 0, false)
		&& Read(buffered, actual, 0, false);
	result["expectedSamples"] = expected.size() / reference.sampleSize();
	result["actualSamples"] = actual.size() / buffered.sampleSize();
	result["passed"] = ok && actual == expected;
	return result;
}

} // namespace

QJsonObject RunAudioSpeedRegression() {
	auto cases = QJsonArray();
	for (const auto rate : { 44100, 48000 }) {
		for (const auto channels : { 1, 2 }) {
			const auto wave = Wave(rate, channels);
			for (const auto speeds : {
				std::pair(1., 2.), std::pair(2., 1.),
				std::pair(0.75, 1.5), std::pair(1.25, 2.),
				std::pair(2., 4.), std::pair(1.5, 0.75),
			}) {
				cases.append(Case(wave, rate, channels, speeds.first, speeds.second));
			}
		}
	}
	return QJsonObject{{"cases", cases}};
}

void SetupScenario(not_null<Runner*> runner) {
	runner->add({
		.name = u"Changing playback speed preserves the original audio timeline"_q,
		.then = [] {
			const auto report = RunAudioSpeedRegression();
			auto file = QFile(EvidenceDir() + u"/audio-speed.json"_q);
			if (file.open(QIODevice::WriteOnly)) {
				file.write(QJsonDocument(report).toJson());
			}
			for (const auto value : report["cases"].toArray()) {
				const auto item = value.toObject();
				Check(item["passed"].toBool(),
					u"Saved PCM cannot duplicate or slow audio after a tempo change"_q,
					QString::fromUtf8(QJsonDocument(item).toJson(QJsonDocument::Compact)));
			}
		},
	});
}

} // namespace Test
#endif
