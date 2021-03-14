#include "plot.h"
#include "plotimpl.h"
#include <QEvent>
#include <QDebug>
#include <QMetaEnum>
#include <QPainter>
#include <QTransform>
#include <QPaintEvent>
#include <QRect>

Plot::Plot(QWidget *parent)
	: QWidget(parent)
{
	m_pimpl = new PlotImpl(this); // todo
	setupConnections();
}

QVector<QPointF> Plot::series() const
{
	return m_pimpl->series();
}

void Plot::setSeries(const QVector<QPointF> &series)
{
	m_pimpl->setSeries(series);
}

QString Plot::functionName() const
{
	return m_pimpl->functionName();
}

void Plot::setParams(double A, double B, double C)
{
	m_pimpl->setParams(A, B, C);
}

void Plot::getParams(double &A, double &B, double &C) const
{
	m_pimpl->getParams(A, B, C);
}

void Plot::setFunction(const std::function<double(double)> &f, const QString &name) {
	m_pimpl->setFunction(f, name);
}

void Plot::setInterval(double from, double to, double step) {
	m_pimpl->setInterval(from, to, step);
}

void Plot::getInterval(double &from, double &to, double &step) const
{
	m_pimpl->getInterval(from, to, step);
}

void Plot::pause(bool state) {
	m_pimpl->pause(state);
}

bool Plot::isPaused() const
{
	return m_pimpl->isPaused();
}

void Plot::start()
{
	m_pimpl->start();
}

bool Plot::isRunning()
{
	return m_pimpl->isRunning();
}

void Plot::interrupt()
{
	m_pimpl->requestInterruption();
}

void Plot::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	renderBackground(&p);
	setupCoordinateTransformatin(&p);
	renderCoordinateSystem(&p);
	renderCurve(&p);
}

int Plot::progress() const
{
	return m_pimpl->progress();
}

void Plot::clear()
{
	m_pimpl->clear();
	update();
}

void Plot::setupCoordinateTransformatin(QPainter * p)
{
	// Смещаем все точки так, чтобы (0,0)
	// соответстовала центру полотна
	p->translate(width() / 2, height() / 2);
	// Мультиплицируем координаты точек так,
	// чтобы полотно лежало между точками
	// (-1, 1) и (1, -1)
	int side = qMin(width(), height());
	p->scale(side/2, -side/2);
}

void Plot::renderBackground(QPainter *p)
{
	p->fillRect(rect(), QGradient::ColdEvening);
}

void Plot::renderCoordinateSystem(QPainter *p)
{
	static const qreal arrowScale = 0.1;
	static const QPointF arrow[3] = {
		arrowScale * QPointF(-0.25, -1.0),
		arrowScale * QPointF(0.0, 0.0),
		arrowScale * QPointF(0.25, -1.0)
	};

	// Координатные прямые
	p->setPen(QPen(Qt::black, 0.005, Qt::SolidLine));
	QLineF xAxis(-1.0, 0.0, 1.0, 0.0);
	QLineF yAxis(0.0, -1.0, 0.0, 1.0);
	p->drawLine(xAxis);
	p->drawLine(yAxis);

	// Направляющие координатные стрелки
	p->save();
	p->translate(0.0, 1.0);
	p->drawPolyline(arrow, 3);
	p->restore();

	p->save();
	p->translate(1.0, 0.0);
	p->rotate(-90);
	p->drawPolyline(arrow, 3);
	p->restore();
}

void Plot::renderCurve(QPainter *p)
{
	const auto curve = m_pimpl->curve();
	p->drawImage(QRectF(-1, -1, 2, 2), curve, curve.rect());
}

void Plot::setupConnections()
{
	connect(m_pimpl, &PlotImpl::resultReady, this, &Plot::resultReady);
}

