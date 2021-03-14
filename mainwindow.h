#pragma once

#include "lib/plot/plot.h"
#include <QWidget>
#include <QTimer>
#include <QPointer>

namespace Ui {
    class MainWindow;
}

class TableWindow;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

	bool eventFilter(QObject *object, QEvent *event) override;

public slots:
	void start(bool);
	void pause(bool);
	void interrupt();
    void calculate();
    void calculateReady();
	void setProgress(int);
	void store();
	void load();

private:
    void setupUi();
    void setupPlot();
	void setupTimer();
    void setupConnections();

    void populateFunctionComboBox();

	void enableGUI(bool);

	void createValueTable();
	void populateValueTable();

private:
    Ui::MainWindow *ui;
    Plot m_plot;
	QTimer m_refreshTimer; // Обновляет индикатор прогресса и окно графика
	QPointer<TableWindow> m_tableWindow;
};

