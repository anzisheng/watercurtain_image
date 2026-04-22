#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class WaterCurtainWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLoadImage();
    void onToggleSimulation();
    void onParticleCountChanged(int value);

private:
    void setupUI();
    
    WaterCurtainWidget* m_waterCurtainWidget;
    bool m_simulationRunning;
};

#endif // MAINWINDOW_H