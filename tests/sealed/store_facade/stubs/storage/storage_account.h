#pragma once
namespace Storage { class Account {
public:
	QString path;
	std::shared_ptr<QByteArray> key = std::make_shared<QByteArray>(32, char(7));
	QString supportModePath() const { return path + "/support"; }
	std::shared_ptr<QByteArray> peekLegacyLocalKey() const { return key; }
}; }
