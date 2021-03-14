#pragma once

#include <QThread>
#include <QMutex>
#include <QVector>
#include <QPointF>
#include <QImage>
#include <functional>

class PlotImpl: public QThread
{
	Q_OBJECT
public:
	PlotImpl(QObject *parent);

	void setSeries(const QVector<QPointF> &);
	QVector<QPointF> series() const;

	void setParams(double A, double B, double C);
	void getParams(double &A, double &B, double &C) const;

	void setFunction(const std::function<double(double)> &f, const QString &);
	QString functionName() const;

	void setInterval(double from, double to, double step);
	void getInterval(double &from, double &to, double &step) const;

	void pause(bool state);
	bool isPaused() const;

	QImage curve() const;

	int progress() const;

	void clear();

signals:
	void resultReady();

private:
	void run();
	void calculate();
	void findMaxAbs();
	void render();
	QImage emptyImage();

private:
	std::function<double(double)> m_f;
	QString m_fName;
	double m_from = 0, m_to = 0, m_step = 0;
	double m_A = 0, m_B = 0, m_C = 0;
	bool m_pause = false;
	double yMaxAbs = 0, xMaxAbs = 0;

	/* Обработанные точки добавляются пакетом,
	 * рамзер которого определяет segmentSize */
	const int segmentSize = 500;

	mutable QMutex m_mutex;		// Защищает доступ к определенным ниже полям
	QVector<QPointF> m_series;
	QImage m_curve;
	int m_printedPoints = 0;	// Добавлено на m_curves
	int m_observedPoints = 0;	// Нормировано
};
