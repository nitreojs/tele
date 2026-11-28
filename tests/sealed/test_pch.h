#pragma once

#include <QtCore/QtCore>
#include <QtGui/QtGui>

inline QString operator""_q(const char16_t *text, size_t size) {
	return QString::fromUtf16(text, qsizetype(size));
}
