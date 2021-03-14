#include "tablewindow.h"
#include "mainwindow.h"
#include "ui_tablewindow.h"
#include <QDebug>
#include <QAbstractTableModel>
#include <QVariant>

class ValueTableModel: public QAbstractTableModel
{
public:
	explicit ValueTableModel(QObject * = nullptr);

	int rowCount(const QModelIndex &parent) const override;
	int columnCount(const QModelIndex &parent) const override;
	QVariant data(const QModelIndex &index, int role) const override;

	void setSeries(const QVector<QPointF> &);

private:
	QVector<QPointF> m_series;
};


ValueTableModel::ValueTableModel(QObject *parent)
	: QAbstractTableModel(parent)
{

}

int ValueTableModel::rowCount(const QModelIndex &) const
{
	return m_series.size();
}
int ValueTableModel::columnCount(const QModelIndex &) const
{
	return 2;
}

QVariant ValueTableModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::DisplayRole) {
		const int col = index.column();

		if( col == 0 )
			return QVariant::fromValue(QString("%1").arg(m_series[index.row()].x(), 2, 'f'));
		else
			return QVariant::fromValue(QString("%1").arg(m_series[index.row()].y(), 2, 'f'));
	}
	return QVariant();
}

void ValueTableModel::setSeries(const QVector<QPointF> &series)
{
	m_series = series;
}

/* TableWindow */

TableWindow::TableWindow(QWidget *parent)
	: QWidget(parent)
	, ui (new Ui::TableWindow)
{
	setupUi();
	setupConnections();

	setWindowFlags(Qt::Dialog);
}

void TableWindow::setSeries(const QVector<QPointF> &series)
{
	auto view = ui->tableView;
	auto model = new ValueTableModel(view);

	model->setSeries(series);
	view->setModel(model);
}

void TableWindow::setFunctionName(const QString &name)
{
	ui->lblFunctionName->setText(QString("f(x) = ") + name);
}

TableWindow::~TableWindow()
{
	delete ui;
}

void TableWindow::setupUi()
{
	ui->setupUi(this);
}

void TableWindow::setupConnections()
{
	connect(ui->btnSave, &QPushButton::clicked, this, &TableWindow::store);
	connect(ui->btnLoad, &QPushButton::clicked, this, &TableWindow::load);
}
