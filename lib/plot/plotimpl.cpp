#include "plotimpl.h"
#include <QDebug>
#include <QMutexLocker>
#include <QPainter>
#include <QImage>
#include <QWidget>
#include <cmath>

PlotImpl::PlotImpl(QObject *parent)
	: QThread (parent)
{ }

QVector<QPointF> PlotImpl::series() const
{
	QMutexLocker locker(&m_mutex);
	return m_series;
}

QString PlotImpl::functionName() const
{
	return m_fName;
}

void PlotImpl::setSeries(const QVector<QPointF> &series) {
	m_series = series;
}

void PlotImpl::getParams(double &A, double &B, double &C) const
{
	A = m_A;
	B = m_B;
	C = m_C;
}

void PlotImpl::setParams(double A, double B, double C)
{
	m_A = A;
	m_B = B;
	m_C = C;
}

void PlotImpl::setFunction(const std::function<double (double)> &f, const QString &name) {
	m_f = f;
	m_fName = name;
}

void PlotImpl::getInterval(double &from, double &to, double &step) const
{
	from = m_from;
	to = m_to;
	step = m_step;
}

void PlotImpl::setInterval(double from, double to, double step) {
	m_from = from;
	m_to = to;
	m_step = step;
}

void PlotImpl::pause(bool state) {
	m_pause = state;
}

bool PlotImpl::isPaused() const
{
	return m_pause;
}

QImage PlotImpl::curve() const
{
	QMutexLocker locker(&m_mutex);
	return m_curve;
}

int PlotImpl::progress() const
{
	QMutexLocker locker(&m_mutex);
	int percents = 0;

	if( m_series.size() > 0 ) {
		percents += 0.5 * static_cast<int>( std::ceil(100.0 * (m_series.last().x() - m_from) / (m_to - m_from)) );
		percents += 0.25 * static_cast<int>( std::ceil(100.0 * m_observedPoints / m_series.size()) );
		percents += 0.25 * static_cast<int>( std::ceil(100.0 * m_printedPoints / m_series.size()) );
	}

	return percents;
}

void PlotImpl::clear()
{
	QMutexLocker locker (&m_mutex);
	m_series.clear();
	m_curve = QImage();
	m_printedPoints = 0;
	m_observedPoints = 0;
}

/* Private */

void PlotImpl::run() {
	calculate();
	findMaxAbs();
	render();

	emit resultReady();
}

void PlotImpl::calculate()
{
	const int size = static_cast<int>( std::ceil((m_to - m_from) / m_step) ) + 1;
	const int currentSize = m_series.size();
	QVector<QPointF> pointSegment; pointSegment.reserve(segmentSize);
	double x = 0;

	for(int i = currentSize; i < size; ++i) {
		x = m_from + m_step * i;

		if( isInterruptionRequested() )
			return;

		while(m_pause);

		pointSegment << QPointF(x, m_f(x));

		if( i % segmentSize == 0 || i == size - 1 ) { // Сегмент заполнен
			QMutexLocker locker(&m_mutex);
			m_series << pointSegment;

			pointSegment.clear();
		}
	}
}

void PlotImpl::findMaxAbs()
{
	const int size = m_series.size();

	if( size > 0 ) {
		// Поиск экстремумов
		yMaxAbs = 0;
		xMaxAbs = 0;

		for(int i = 0; i < size; ++i) {
			if( isInterruptionRequested() )
				return;

			while(m_pause);

			yMaxAbs = qMax(qAbs(yMaxAbs), qAbs(m_series[i].y()));

			if( i % segmentSize == 0 || i == size  - 1) {
				QMutexLocker locker(&m_mutex);
				m_observedPoints = i;
			}
		}

		xMaxAbs = qMax(qAbs(m_series.first().x()), qAbs(m_series.last().x()));
	}
}

void PlotImpl::render()
{
	const int size = m_series.size();

	if( size > 0) {
		const QPointF beginPoint = {m_series.first().x() / xMaxAbs, m_series.first().y() / yMaxAbs };
		QPainterPath curve;
		QPointF np; // Следующая точка

		curve.moveTo(beginPoint);
		for(int i = 1; i < size; ++i) {
			if( isInterruptionRequested() )
				return;

			while(m_pause);

			// Нормировка
			np = m_series[i];

			np.setX( np.x() / xMaxAbs );
			np.setY( np.y() / yMaxAbs );

			curve.lineTo(np);

			if( i % segmentSize == 0 || i == size - 1) {
				QMutexLocker locker(&m_mutex);

				if( m_curve.isNull() )
					m_curve = emptyImage();

				QPainter p(&m_curve);
				// Трансформация
				const int side = m_curve.width();
				p.translate(side / 2, side / 2);
				p.scale(side/2, side/2);

				// Отрисовка
				p.setPen(QPen(Qt::white, 0.005, Qt::SolidLine));
				p.drawPath(curve);

				m_printedPoints = i;
				locker.unlock();

				curve.clear();
				curve.moveTo(np); //should be?
			}
		}
	}
}

QImage PlotImpl::emptyImage()
{
	const auto side = 512; // Качество изображения
	const auto fmt  = QImage::Format_RGBA8888; // Цветопередача
	QImage img (side, side, fmt);

	img.fill(Qt::transparent);
	return img;
}
