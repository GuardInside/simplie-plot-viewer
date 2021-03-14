#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tablewindow.h"
#include <QDebug>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QFileDialog>
#include <QFile>
#include <QDataStream>
#include <QMessageBox>
#include <functional>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::MainWindow)
{
	setupUi();
	setupTimer();
	setupPlot();
	setupConnections();

	m_plot.installEventFilter(this);
	setWindowTitle("simple-plot-viewer");
}

MainWindow::~MainWindow()
{
	delete ui;
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
	if (object == &m_plot && event->type() == QEvent::MouseButtonPress) {
		QMouseEvent *keyEvent = static_cast<QMouseEvent *>(event);
		if( keyEvent->KeyPress == QKeyEvent::KeyPress && keyEvent->button() == Qt::LeftButton) {
			if( m_plot.isRunning() || !m_plot.isPaused() ) {
				createValueTable();
			}
		}
	}
	return false;
}

void MainWindow::start(bool saveOldData)
{
	if( !m_plot.isRunning() ) {
		if( !saveOldData )
			m_plot.clear();

		enableGUI(false);
		calculate();
		m_refreshTimer.start();
	}
}

void MainWindow::pause(bool btnState)
{
	if( m_plot.isRunning() ) {
		const QString txt[] = {"Pause", "Continue"};
		m_plot.pause(btnState);
		ui->btnPause->setText(txt[btnState ? 1 : 0]);
	}
	else
		ui->btnPause->setChecked(false);
}

void MainWindow::interrupt()
{
	if( ui->btnPause->isChecked() )
		ui->btnPause->click();

	if( m_plot.isRunning() )
		calculateReady();

	m_plot.interrupt();
}

void MainWindow::calculate()
{
	const auto A = ui->sbA->value();
	const auto B = ui->sbB->value();
	const auto C = ui->sbC->value();
	const auto from = ui->sbFrom->value();
	const auto to = ui->sbTo->value();
	const auto step = ui->sbStep->value();
	const auto fName = ui->cbFunctions->currentText().mid(QString("f(x) = ").length());
	const auto fIndex =  ui->cbFunctions->currentIndex();
	std::function<double(double)> f;

	if( fIndex == 0 ) {
		f = [A, B, C](double x){ return A*(x*x) + B*x + C; };
		m_plot.setFunction(f, fName);
	}

	else if( fIndex == 1 ) {
		auto f = [A, B, C](double x){ return A*sin(x) + B*cos(C*x); };
		m_plot.setFunction(f, fName);
	}

	else if( fIndex == 2 ) {
		auto f = [A, B](double x){ return A*log(B*x); };
		m_plot.setFunction(f, fName);
	}

	else if( fIndex == 3 ) {
		auto f = [A, B](double x){ return A / ( B*sin(x*x) ); };
		m_plot.setFunction(f, fName);
	}

	else
		Q_UNREACHABLE();

	m_plot.setParams(A, B, C);
	m_plot.setInterval(from, to, step);
	m_plot.start();
}

void MainWindow::calculateReady()
{
	//	qDebug() << "calculateReady";
	ui->btnStart->setText(QString("%1").arg("New"));
	enableGUI(true);
	m_refreshTimer.stop();
	m_plot.update();
}

void MainWindow::setProgress(int progress)
{
	//	qDebug() << "setProgress" << progress;
	ui->btnStart->setText(QString("Progress (%1 %)").arg(progress));
}

void MainWindow::store()
{
	const QString fileName = QFileDialog::getSaveFileName(m_tableWindow, "Save series");

	if( fileName.isEmpty() )
		return;

	QFile ofile(fileName);

	if( !ofile.open(QFile::WriteOnly | QFile::Truncate) ) {
		QMessageBox::warning(this, "Save error", ofile.errorString(), QMessageBox::Ok);
		return;
	}

	QDataStream stream(&ofile);
	double A, B, C;
	double from, to, step;
	auto fName = m_plot.functionName();

	m_plot.getParams(A, B, C);
	m_plot.getInterval(from, to, step);

	stream << fName
		   << A << B << C
		   << from << to << step;

	stream << m_plot.series();

	ofile.close();
}

void MainWindow::load()
{
	const QString fileName = QFileDialog::getOpenFileName(m_tableWindow, "Load series");

	if( fileName.isEmpty() )
		return;

	QFile ifile(fileName);

	if( !ifile.open(QFile::ReadOnly) ) {
		QMessageBox::warning(this, "Open error", ifile.errorString(), QMessageBox::Ok);
		return;
	}

	QDataStream stream(&ifile);
	double A, B, C;
	double from, to, step;
	QString fName;

	stream >> fName;
	stream >> A >> B >> C
			>> from >> to >> step;

	m_plot.setParams(A, B, C);
	m_plot.setInterval(from, to, step);

	ui->cbFunctions->setCurrentIndex(ui->cbFunctions->findText("f(x) = " + fName));
	ui->sbA->setValue(A); ui->sbB->setValue(B); ui->sbC->setValue(C);
	ui->sbFrom->setValue(from); ui->sbTo->setValue(to); ui->sbStep->setValue(step);

	QVector<QPointF> series;
	stream >> series;

	m_plot.clear();
	m_plot.setSeries(series);
	start(true);

	ifile.close();

	m_tableWindow->close();
}

/* Private */

void MainWindow::setupUi()
{
	ui->setupUi(this);
	ui->hlMain->addWidget(&m_plot);
	populateFunctionComboBox();
}

void MainWindow::setupPlot()
{

}

void MainWindow::setupTimer()
{
	m_refreshTimer.setParent(this);
	m_refreshTimer.setInterval(100);

	connect(&m_refreshTimer, &QTimer::timeout, this, [this]()
	{
		if( !m_plot.isRunning() || m_plot.isPaused() )
			return;

		setProgress(m_plot.progress());
		m_plot.update();
	});
}

void MainWindow::populateFunctionComboBox()
{
	const auto fNames = {"A*(x*x) + B*x + C", "A*sin(x) + B*cos(C*x)",
						 "A*log(B*x)", "A / ( B*sin(x*x) )"};

	for(const auto &fName: fNames)
		ui->cbFunctions->addItem(QString("f(x) = ") + fName);
}

void MainWindow::setupConnections()
{
	connect(ui->btnStart, &QPushButton::clicked, this, &MainWindow::start);
	connect(ui->btnPause, &QPushButton::toggled, this, &MainWindow::pause);
	connect(ui->btnBreak, &QPushButton::clicked, this, &MainWindow::interrupt);
	connect(&m_plot, &Plot::resultReady, this, &MainWindow::calculateReady);
}

void MainWindow::enableGUI(bool isEnable)
{
	ui->cbFunctions->setEnabled(isEnable);
	ui->sbA->setEnabled(isEnable);
	ui->sbB->setEnabled(isEnable);
	ui->sbC->setEnabled(isEnable);
	ui->sbFrom->setEnabled(isEnable);
	ui->sbTo->setEnabled(isEnable);
	ui->sbStep->setEnabled(isEnable);
}

void MainWindow::createValueTable() {
	if( m_tableWindow.isNull() ) {
		m_tableWindow = new TableWindow(this);

		connect(m_tableWindow, &TableWindow::store, this, &MainWindow::store);
		connect(m_tableWindow, &TableWindow::load, this, &MainWindow::load);
	}

	if( m_tableWindow->isHidden() )
		populateValueTable();

	m_tableWindow->show();
}

void MainWindow::populateValueTable()
{
	m_tableWindow->setSeries(m_plot.series());
	m_tableWindow->setFunctionName(m_plot.functionName());
}
