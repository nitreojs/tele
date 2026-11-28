#pragma once
#include <QtCore/QtCore>
#include <gsl/util>
#include <algorithm>
#include <functional>
#include <memory>
#include <optional>
#include <utility>
using uint32 = quint32;
template <typename T> using not_null = T;
template <typename Signature> using Fn = std::function<Signature>;
inline QString operator""_q(const char16_t *text, size_t size) {
	return QString::fromUtf16(text, qsizetype(size));
}
