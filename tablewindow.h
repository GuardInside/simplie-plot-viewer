#pragma once

#include <QVector>
#include <QPointF>
#include <QWidget>

namespace Ui {
	class TableWindow;
}

class ValueTableModel;

class TableWindow: public QWidget
{
	Q_OBJECT

public:
	explicit TableWindow(QWidget * = nullptr);

	void setSeries(const QVector<QPointF> &);

	void setFunctionName(const QString &);

	~TableWindow();

signals:
	void store();
	void load();

private:
	void setupUi();
	void setupConnections();
private:
	Ui::TableWindow *ui;
};
