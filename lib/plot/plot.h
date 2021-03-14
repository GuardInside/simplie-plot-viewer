#pragma once

#include <QWidget>
#include <QThread>
#include <QVector>
#include <QPointF>

class QPaintEvent;
class QPainter;
class PlotImpl;
class QPaintEvent;

/* График по обеим осям нормирован на единицу */
class Plot: public QWidget
{
    Q_OBJECT
public:
	explicit Plot(QWidget *parent = 0);

	QVector<QPointF> series() const;
	void setSeries(const QVector<QPointF> &);

	void setFunction(const std::function<double(double)> &, const QString &);
	QString functionName() const;

	void setParams(double A, double B, double C);
	void getParams(double &A, double &B, double &C) const;

	void setInterval(double from, double to, double step);
	void getInterval(double &from, double &to, double &step) const;

	// Запускает поток вычислений
	void start();
	bool isRunning();
	// Приостанавливает поток вычислений
	void pause(bool state);
	bool isPaused() const;
	// Прекращает вычисления
	void interrupt();
	//isRunning()

	void paintEvent(QPaintEvent *event) override;

	int progress() const;

	// Удаляет график
	void clear();

signals:
	void resultReady();
	void seriesChanged();

private:
	void setupCoordinateTransformatin(QPainter *);
	void renderBackground(QPainter *);
	void renderCoordinateSystem(QPainter *);
	void renderCurve(QPainter *);
	void setupConnections();
	void run();

private:
	PlotImpl *m_pimpl;
};
